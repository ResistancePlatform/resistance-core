#!/usr/bin/env bash
# A wrapper around ./resutil/build.sh for instrumenting the build with AFL:
#   ./resutil/afl/afl-build.sh <directory where AFL is installed> <fuzz case>
# You may obtain a copy of AFL using ./resutil/afl/afl-get.sh.

set -eu -o pipefail

export AFL_INSTALL_DIR=$(realpath "$1")
FUZZ_CASE="$2"
shift 2
export AFL_LOG_DIR="$(pwd)"
export ZCUTIL=$(realpath "./resutil")

cp "./src/fuzzing/$FUZZ_CASE/fuzz.cpp" src/fuzz.cpp

CONFIGURE_FLAGS="--enable-tests=no --enable-fuzz-main" "$ZCUTIL/build.sh" "CC=$ZCUTIL/afl/resistance-wrapper-gcc" "CXX=$ZCUTIL/afl/resistance-wrapper-g++" AFL_HARDEN=1 "$@"

echo "You can now run AFL as follows:"
echo "$ ./resutil/afl/afl-run.sh '$AFL_INSTALL_DIR' '$FUZZ_CASE'"
