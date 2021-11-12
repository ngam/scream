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
  Int num_levs = 3;
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

  // Read data for check
  view_1d<Real> field_1("field_1",num_gcols);
  scorpio::register_file(filename,scorpio::Read);
  std::vector<std::string> vec_of_dims{"ncol"};
  std::string r_decomp = "Real-ncol";
  scorpio::get_variable(filename, "field_1", "field_1", vec_of_dims.size(), vec_of_dims, PIO_REAL, r_decomp);
  std::vector<int> var_dof(num_gcols);
  std::iota(var_dof.begin(),var_dof.end(),0);
  scorpio::set_dof(filename,"field_1",var_dof.size(),var_dof.data());
  scorpio::set_decomp(filename);
  scorpio::grid_read_data_array(filename,"field_1",0,field_1.data());
  scorpio::eam_pio_closefile(filename);
  
} // TEST_CASE
} // namespace
