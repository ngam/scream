#include "catch2/catch.hpp"

#include "share/io/scream_scorpio_interface.hpp"
#include "physics/spa/spa_functions.hpp"

#include "ekat/ekat_pack.hpp"
#include "ekat/util/ekat_arch.hpp"
#include "ekat/kokkos/ekat_kokkos_utils.hpp"

#include <algorithm>
#include <array>
#include <random>
#include <thread>

namespace {

using namespace scream;
using namespace spa;

template <typename S>
using view_1d = typename KokkosTypes<DefaultDevice>::template view_1d<S>;

Real ps_func(const Int t, const Int icol, const Int ncols);
Real ccn3_func(const Int t, const Int icol, const Int klev, const Int ncols);
Real aer_func(const Int t, const Int icol, const Int bnd, const Int klev, const Int ncols, const Int mode);

TEST_CASE("spa_read_data","spa")
{
  // Set up the mpi communicator and init the pio subsystem
  ekat::Comm spa_comm(MPI_COMM_WORLD);  // MPI communicator group used for I/O set as ekat object.
  MPI_Fint fcomm = MPI_Comm_c2f(spa_comm.mpi_comm());  // MPI communicator group used for I/O.  In our simple test we use MPI_COMM_WORLD, however a subset could be used.
  scorpio::eam_init_pio_subsystem(fcomm);   // Gather the initial PIO subsystem data creater by component coupler

  // Establish the SPA function object
  using SPAFunc = spa::SPAFunctions<Real, DefaultDevice>;
  using Spack = SPAFunc::Spack;
  using gid_type = SPAFunc::gid_type;

  std::string spa_data_file = "spa_data_for_testing.nc";
  std::string spa_remap_file = "spa_data_for_testing.nc";
  Int max_time = 3;
  Int ncols    = 48;
  Int nlevs    = 4;
  Int nswbands = 2;
  Int nlwbands = 3;
  // Break the test set of columns into local degrees of freedom per mpi rank
  auto comm_size = spa_comm.size();
  auto comm_rank = spa_comm.rank();

  int my_ncols = ncols/comm_size + (comm_rank < ncols%comm_size ? 1 : 0);
  view_1d<gid_type> dofs_gids("",my_ncols);
  gid_type min_dof = 1; // Start global-ids from 1
  Kokkos::parallel_for("", my_ncols, KOKKOS_LAMBDA(const int& ii) {
    dofs_gids(ii) = min_dof + static_cast<gid_type>(comm_rank + ii*comm_size);
  });
  // Make sure that the total set of columns has been completely broken up.
  Int test_total_ncols = 0;
  spa_comm.all_reduce(&my_ncols,&test_total_ncols,1,MPI_SUM);
  REQUIRE(test_total_ncols == ncols);

  // Set up the set of SPA structures needed to run the test
  SPAFunc::SPAHorizInterp spa_horiz_interp;
  spa_horiz_interp.m_comm = spa_comm;
  SPAFunc::get_remap_weights_from_file(spa_remap_file,ncols,min_dof,dofs_gids,spa_horiz_interp);
  SPAFunc::SPAData spa_data(dofs_gids.size(), nlevs, nswbands, nlwbands);

  // Verify that the interpolated values match the algorithm for the data and the weights.
  //       weights(i) = 1 / (2**i), weights(-1) = 1 / (2**(ncols-1)) such that sum(weights) = 1., for i=0,1,2
  //       FOR t=1,2,3; i=0,1,2; b=1,2 or 1,2,3 and k=0,1,2,3
  //       p(t,i) = (t+1) * (i+1)*100
  //       ccn3(t,i,k) = (i+1)*100 + t*10 + k
  //       aer_g_sw(t,i,b,k) = t
  //       aer_ssa_sw(t,i,b,k) = i
  //       aer_tau_sw(t,i,b,k) = b
  //       aer_tau_lw(t,i,b,k) = k
  auto ps_h         = Kokkos::create_mirror_view(spa_data.PS);
  auto ccn3_h       = Kokkos::create_mirror_view(spa_data.CCN3);
  auto aer_g_sw_h   = Kokkos::create_mirror_view(spa_data.AER_G_SW);
  auto aer_ssa_sw_h = Kokkos::create_mirror_view(spa_data.AER_SSA_SW);
  auto aer_tau_sw_h = Kokkos::create_mirror_view(spa_data.AER_TAU_SW);
  auto aer_tau_lw_h = Kokkos::create_mirror_view(spa_data.AER_TAU_LW);
  auto dofs_gids_h = Kokkos::create_mirror_view(dofs_gids);
  Kokkos::deep_copy(dofs_gids_h,dofs_gids);
  for (int time_index = 0;time_index<max_time; time_index++) {
    SPAFunc::update_spa_data_from_file(spa_data_file, time_index+1, nswbands, nlwbands,
                                       spa_horiz_interp, spa_data);
    Kokkos::deep_copy(ps_h,spa_data.PS);
    Kokkos::deep_copy(ccn3_h,spa_data.CCN3);
    Kokkos::deep_copy(aer_g_sw_h,spa_data.AER_G_SW);
    Kokkos::deep_copy(aer_ssa_sw_h,spa_data.AER_SSA_SW);
    Kokkos::deep_copy(aer_tau_sw_h,spa_data.AER_TAU_SW);
    Kokkos::deep_copy(aer_tau_lw_h,spa_data.AER_TAU_LW);
    for (int dof_i=0;dof_i<dofs_gids_h.size();dof_i++) {
      gid_type glob_i = dofs_gids_h(dof_i);
      REQUIRE(ps_h(dof_i) == ps_func(time_index,glob_i,spa_horiz_interp.source_grid_ncols));
      for (int kk=0;kk<nlevs;kk++) {
        int kpack = kk / Spack::n;
        int kidx  = kk % Spack::n;
        REQUIRE(ccn3_h(dof_i,kpack)[kidx] == ccn3_func(time_index, glob_i, kk, spa_horiz_interp.source_grid_ncols));
        for (int n=0;n<nswbands;n++) {
          REQUIRE(aer_g_sw_h(dof_i,n,kpack)[kidx]   == aer_func(time_index,glob_i,n,kk,spa_horiz_interp.source_grid_ncols,0));
          REQUIRE(aer_ssa_sw_h(dof_i,n,kpack)[kidx] == aer_func(time_index,glob_i,n,kk,spa_horiz_interp.source_grid_ncols,1));
          REQUIRE(aer_tau_sw_h(dof_i,n,kpack)[kidx] == aer_func(time_index,glob_i,n,kk,spa_horiz_interp.source_grid_ncols,2));
        }
        for (int n=0;n<nlwbands;n++) {
          REQUIRE(aer_tau_lw_h(dof_i,n,kpack)[kidx] ==  aer_func(time_index,glob_i,n,kk,spa_horiz_interp.source_grid_ncols,3));
        }
      }
    }
  }
  
  // All Done 
  scorpio::eam_pio_finalize();
} // run_property

// Some helper functions for the require statements:
Real ps_func(const Int t, const Int icol, const Int ncols)
{
  Real ps = 0.0;
  for (int i=1;i<=ncols;i++) {
    Real wgt = 1.0 / std::pow(2.0,i);
    if (i == ncols) { wgt *= 2.0; }
    ps += (t+1) * i*100.0 * wgt;
  }
  return ps;
} // ps_func
//
Real ccn3_func(const Int t, const Int icol, const Int klev, const Int ncols)
{
  Real ccn3 = 0.0;
  for (int i=1;i<=ncols;i++) {
    Real wgt = 1.0 / std::pow(2.0,i);
    if (i == ncols) {wgt *= 2.0;}
    ccn3 += wgt * (klev*1.0 + t*10.0 + i*100.0);
  }
  return ccn3;
} // ccn3_func
//
Real aer_func(const Int t, const Int icol, const Int bnd, const Int klev, const Int ncols, const Int mode)
{
  Real aer_out = 0.0;
  for (int i=1;i<=ncols;i++) {
    Real wgt = 1.0 / std::pow(2.0,i);
    if (i == ncols) {wgt *= 2.0;}
    if (mode==0) {  // G
      aer_out += wgt * t;
    } else if (mode==1) { // SSA
      aer_out += wgt * (i-1);
    } else if (mode==2) { // TAU
      aer_out += wgt * bnd;
    } else if (mode==3) { // LW-TAU
      aer_out += wgt * klev;
    }
  }
  return aer_out;
} // aer_func

} // namespace

