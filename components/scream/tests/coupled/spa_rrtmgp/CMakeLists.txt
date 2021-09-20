INCLUDE (ScreamUtils)

find_library(NETCDF_C netcdf HINTS ${NetCDF_C_PATHS}/lib)
SET (NEED_LIBS spa scream_rrtmgp rrtmgp scream_control physics_share scream_share ${NETCDF_C} yakl)

SET (SRC spa_rrtmgp.cpp ${SCREAM_BASE_DIR}/src/physics/rrtmgp/tests/rrtmgp_test_utils.cpp)
# Test atmosphere processes
CreateUnitTest(spa_rrtmgp "${SRC}" "${NEED_LIBS}" LABELS "spa;rrtmgp;physics")

# Copy yaml input file to run directory
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/input.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/input.yaml COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/spa_rrtmgp_init_ne2np4.nc
               ${CMAKE_CURRENT_BINARY_DIR}/spa_rrtmgp_init_ne2np4.nc COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/spa_rrtmgp_output.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/spa_rrtmgp_output.yaml COPYONLY)
## Copy yaml input file to run directory
FILE (MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)
CONFIGURE_FILE(${SCREAM_BASE_DIR}/../eam/src/physics/rrtmgp/external/rrtmgp/data/rrtmgp-data-sw-g224-2018-12-04.nc
               ${CMAKE_CURRENT_BINARY_DIR}/data COPYONLY)
CONFIGURE_FILE(${SCREAM_BASE_DIR}/../eam/src/physics/rrtmgp/external/rrtmgp/data/rrtmgp-data-lw-g256-2018-12-04.nc
               ${CMAKE_CURRENT_BINARY_DIR}/data COPYONLY)
CONFIGURE_FILE(${SCREAM_BASE_DIR}/../eam/src/physics/rrtmgp/external/extensions/cloud_optics/rrtmgp-cloud-optics-coeffs-sw.nc
               ${CMAKE_CURRENT_BINARY_DIR}/data COPYONLY)
CONFIGURE_FILE(${SCREAM_BASE_DIR}/../eam/src/physics/rrtmgp/external/extensions/cloud_optics/rrtmgp-cloud-optics-coeffs-lw.nc
               ${CMAKE_CURRENT_BINARY_DIR}/data COPYONLY)
