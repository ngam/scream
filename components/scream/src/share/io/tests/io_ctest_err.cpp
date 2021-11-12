#include <catch2/catch.hpp>
#include <memory>
#include "ekat/util/ekat_string_utils.hpp"
#include "share/scream_types.hpp"
#include "share/io/scream_scorpio_interface.hpp"
#include "share/io/scorpio_input.hpp"
#include "share/io/scream_scorpio_interface.hpp"
#include "share/util/scream_time_stamp.hpp"
#include "scream_config.h"
#include "ekat/util/ekat_units.hpp"
#include "ekat/ekat_parameter_list.hpp"
#include "ekat/ekat_scalar_traits.hpp"
#include "ekat/ekat_assert.hpp"

namespace {

using namespace scream;

using KT = KokkosTypes<DefaultDevice>;
template <typename S>
using view_1d = typename KT::template view_1d<S>;

TEST_CASE("input_output_nomgr","io")
{

  ekat::Comm io_comm(MPI_COMM_WORLD);  // MPI communicator group used for I/O set as ekat object.
  Int num_gcols = 2*io_comm.size();
  MPI_Fint fcomm = MPI_Comm_c2f(io_comm.mpi_comm());  // MPI communicator group used for I/O.  In our simple test we use MPI_COMM_WORLD, however a subset could be used.
  scorpio::eam_init_pio_subsystem(fcomm);   // Gather the initial PIO subsystem data creater by component coupler

  // Construct a timestamp
  util::TimeStamp t0 ({2000,1,1},{0,0,0});
  util::TimeStamp time = t0;
  Int max_steps = 10;
  Int dt = 1;
  for (Int ii=0;ii<max_steps;++ii) {
    time += dt;
  }

  std::string filename = "io_output_test_np1.INSTANT.Steps_x10.2000-01-01.000010.nc";
  std::string fieldname = "field_1";

  // Read data for check
  // Try absolutely manually:
  std::vector<std::string> vec_of_dims = {"ncol"};
  std::vector<int> var_dof = {0,1};
  view_1d<Real> field_1(fieldname,num_gcols);
  auto field_1_h = Kokkos::create_mirror_view(field_1);
  scorpio::register_file       (filename,scorpio::Read);
  scorpio::get_variable        (filename,fieldname,fieldname, 1, vec_of_dims, 6, "Real-ncol");
  scorpio::set_dof             (filename,fieldname, 2, var_dof.data());
  scorpio::set_decomp          (filename);
  scorpio::grid_read_data_array(filename,fieldname, -1, field_1_h.data());
  scorpio::eam_pio_closefile   (filename);
  Kokkos::deep_copy(field_1,field_1_h);

  REQUIRE(field_1_h(0) == 10);
  REQUIRE(field_1_h(1) == 11);
  
} // TEST_CASE
} // namespace
