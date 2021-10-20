#include "ekat/kokkos/ekat_kokkos_types.hpp"
#include "ekat/util/ekat_test_utils.hpp"
#include "ekat/mpi/ekat_comm.hpp"

#include <catch2/catch.hpp>

#include <iostream>
#include <chrono>
#include <thread>

namespace {

TEST_CASE("gpu_spread", "[fake_infra_test]")
{
  // Nothing needs to be done here except sleeping to give a chance
  // for tests to run concurrently.
  const auto seconds_to_sleep = 5;
  std::this_thread::sleep_for(std::chrono::seconds(seconds_to_sleep));

  using KT = ekat::KokkosTypes<ekat::DefaultDevice>;

  // Simple kernel for printing device id
  Kokkos::parallel_for(1, KOKKOS_LAMBDA(const int& i) {
    const auto device_id = Kokkos::Tools::Experimental::device_id(typename KT::ExeSpace());
    printf("JGF device_id: %d\n", device_id);
  });

}

} // empty namespace
