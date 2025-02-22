set (dynLibName)
if ("${SCREAM_DYNAMICS_DYCORE}" STREQUAL "HOMME")
  # Need these two to be linked, due to homme deps

  # Recall that src/dynamics/homme/CMakeLists.txt does not build a dyn lib.
  # It only defines a macro to build it, so that different tests can build
  # a dyn lib if necessary.
  # Here, we ask to create a dyn lib depending on the config options that
  # were set during e3sm configuration

  # This is a list of cmake vars whose values are then used when calling
  # CreateDynamicsLib, to correctly build the dynamics library within Homme.
  # We set them to a default, but each compset should set its values anyways.
  set (SCREAM_DYN_TARGET none CACHE STRING "The name of the desired Homme target.")
  set (SCREAM_NP 4 CACHE STRING "The number of Gauss points per element.")
  set (SCREAM_NUM_TRACERS 4 CACHE STRING "The max number of tracers.")
  set (SCREAM_USE_PIO FALSE CACHE STRING "Whether Homme can use PIO.")

  CreateDynamicsLib (${SCREAM_DYN_TARGET} ${SCREAM_NP} ${SCREAM_NUM_VERTICAL_LEV} ${SCREAM_NUM_TRACERS})
endif()

set(ATM_SRC
  scream_cxx_f90_interface.cpp
  atm_comp_mct.F90
  dead_mct_mod.F90
  dead_data_mod.F90
  dead_mod.F90
  ekat_string_utils.F90
  scream_f2c_mod.F90
  scream_cpl_indices.F90
  ${SCREAM_BASE_DIR}/../eam/src/physics/cam/physics_utils.F90
)

set (SCREAM_LIBS
     scream_share
     scream_control
     ${dynLibName}
     p3
     shoc
     scream_rrtmgp
     cld_fraction
     spa
)

include (tpls/Mct)
CreateMctTarget()

# Create atm lib
add_library(atm ${ATM_SRC})

# Set f90 compiled modules dir
set_target_properties(atm PROPERTIES Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/cmake/atm)

# Link libraries
target_link_libraries(atm PRIVATE ${SCREAM_LIBS} mct)
