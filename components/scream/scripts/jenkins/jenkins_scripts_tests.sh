#!/bin/bash -xe

export JENKINS_SCRIPT_DIR=$( cd "$( dirname "$0" )" && pwd )
DATE_STAMP=$(date "+%Y-%m-%d_%H%M%S")

set -o pipefail
$JENKINS_SCRIPT_DIR/jenkins_scripts_tests_impl.sh 2>&1 | tee JENKINS_$DATE_STAMP
