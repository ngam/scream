include (ScreamUtils)

# Create the test
set (NEED_LIBS zm scream_control scream_share)
CreateUnitTest(zm_standalone "zm_standalone.cpp" "${NEED_LIBS}" LABELS "zm;physics")

# Set AD configurable options
SetVarDependingOnTestProfile(NUM_STEPS 2 24 48)
set (ATM_TIME_STEP 1800)

# Configure yaml input file to run directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/input.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/input.yaml)
