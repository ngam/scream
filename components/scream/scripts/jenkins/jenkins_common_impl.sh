#!/bin/bash -xe

# Check if the env var PR_LABELS is defined, and contains something meaningful
IFS=';' read -r -a labels <<< "$PR_LABELS";

# default values
skip_testing=0
test_scripts=0
test_cime=0
skip_mappy=0
skip_weaver=0
skip_blake=0
if [ ${#labels[@]} -gt 0 ]; then
  # We do have some labels. Look for some supported ones.
  for label in "${labels[@]}"
  do
    if [ "$label" == "AT: Integrate Without Testing" ]; then
      skip_testing=1
    elif [ "$label" == "scripts" ]; then
      test_scripts=1
    elif [ "$label" == "CIME" ]; then
      test_cime=1
    elif [ "$label" == "AT: Skip mappy" ]; then
      skip_mappy=1
    elif [ "$label" == "AT: Skip weaver" ]; then
      skip_weaver=1
    elif [ "$label" == "AT: Skip blake" ]; then
      skip_blake=1
    fi
  done
fi

if [ $skip_testing -eq 0 ]; then
  # User did not request to skip tests. Proceed with testing.

  cd $JENKINS_SCRIPT_DIR/../..

  source scripts/jenkins/${NODE_NAME}_setup

  # Check machine-specific skipping
  if [[ $skip_mappy == 1 && "$SCREAM_MACHINE" == "mappy" ]]; then
    echo "Tests were skipped, since the Github label 'AT: Skip mappy' was found.\n"
    exit 0
  elif [[ $skip_weaver == 1 && "$SCREAM_MACHINE" == "weaver" ]]; then
    echo "Tests were skipped, since the Github label 'AT: Skip weaver' was found.\n"
    exit 0
  elif [[ $skip_blake == 1 && "$SCREAM_MACHINE" == "blake" ]]; then
    echo "Tests were skipped, since the Github label 'AT: Skip blake' was found.\n"
    exit 0
  fi

  if [[ "$(whoami)" == "e3sm-jenkins" ]]; then
      git config --local user.email "jenkins@ignore.com"
      git config --local user.name "Jenkins Jenkins"
  fi

  SUBMIT="--submit"
  # We never want to submit a scripts-test run
  if [ -n "$SCREAM_FAKE_ONLY" ]; then
      SUBMIT=""
  fi

  AUTOTESTER_CMAKE=""
  # Now that we are starting to run things that we expect could fail, we
  # do not want the script to exit on any fail since this will prevent
  # later tests from running.
  set +e
  if [ -n "$PULLREQUESTNUM" ]; then
      SUBMIT="" # We don't submit AT runs
      AUTOTESTER_CMAKE="-c SCREAM_AUTOTESTER=ON"
  fi

  declare -i fails=0
  # The special string "AUTO" makes test-all-scream look for a baseline dir in the machine_specs.py file.
  # IF such dir is not found, then the default (ctest-build/baselines) is used
  BASELINES_DIR=AUTO

  ./scripts/gather-all-data "./scripts/test-all-scream --baseline-dir $BASELINES_DIR \$compiler -c EKAT_DISABLE_TPL_WARNINGS=ON $AUTOTESTER_CMAKE -p -i -m \$machine $SUBMIT" -l -m $SCREAM_MACHINE
  if [[ $? != 0 ]]; then fails=$fails+1; fi

  # Add a valgrind and coverage tests for mappy for nightlies
  if [[ -n "$SUBMIT" && "$SCREAM_MACHINE" == "mappy" ]]; then
    ./scripts/gather-all-data "./scripts/test-all-scream -t valg -t cov --baseline-dir $BASELINES_DIR \$compiler -c EKAT_DISABLE_TPL_WARNINGS=ON -p -i -m \$machine $SUBMIT" -l -m $SCREAM_MACHINE
    if [[ $? != 0 ]]; then fails=$fails+1; fi
  fi

  # Add a cuda-memcheck test for weaver for nightlies
  if [[ -n "$SUBMIT" && "$SCREAM_MACHINE" == "weaver" ]]; then
    ./scripts/gather-all-data "./scripts/test-all-scream -t cmc --baseline-dir $BASELINES_DIR \$compiler -c EKAT_DISABLE_TPL_WARNINGS=ON -p -i -m \$machine $SUBMIT" -l -m $SCREAM_MACHINE
    if [[ $? != 0 ]]; then fails=$fails+1; fi
  fi

  # scripts-tests is pretty expensive, so we limit this testing to mappy
  if [[ $test_scripts == 1 && "$SCREAM_MACHINE" == "mappy" ]]; then
    # JGF: I'm not sure there's much value in these dry-run comparisons
    # since we aren't changing HEADs
    ./scripts/scripts-tests -g -m $SCREAM_MACHINE
    if [[ $? != 0 ]]; then fails=$fails+1; fi
    ./scripts/scripts-tests -c -m $SCREAM_MACHINE
    if [[ $? != 0 ]]; then fails=$fails+1; fi

    ./scripts/scripts-tests -f -m $SCREAM_MACHINE
    if [[ $? != 0 ]]; then fails=$fails+1; fi
  fi

  # Run SCREAM CIME suite
  if [[ $test_cime == 1 && "$SCREAM_MACHINE" == "mappy" ]]; then
    ../../cime/scripts/create_test e3sm_scream -c -b master
    if [[ $? != 0 ]]; then fails=$fails+1; fi

    ../../cime/scripts/create_test e3sm_scream_v1 --compiler=gnu9 -c -b master
    if [[ $? != 0 ]]; then fails=$fails+1; fi
  fi

  if [[ $fails > 0 ]]; then
      exit 1
  fi

else
  echo "Tests were skipped, since the Github label 'AT: Integrate Without Testing' was found.\n"
fi
