#!/bin/bash
set -ex

if [ $# -ne 1 ]; then
  echo "Usage: $0 source_dir"
  exit 1
fi

SRCDIR="$1"

# Build libkpl
KPL_BUILDDIR="$SRCDIR/libkpl/build_ci"
rm -rf "$KPL_BUILDDIR"
mkdir "$KPL_BUILDDIR"
pushd "$KPL_BUILDDIR"
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON ..
make -j$(nproc)
popd

SPECULOS_BIN="/opt/speculos/speculos.py"

# Build the app for Nano S
pushd "$SRCDIR/app"
export BOLOS_SDK=$NANOS_SDK
make clean && make -j$(nproc)
popd

# Run the tests
python3 "$SRCDIR/tests/run.py" --kpl-build-dir "$KPL_BUILDDIR" --model nanos --speculos-bin "$SPECULOS_BIN"

# Build the app for Nano X
pushd "$SRCDIR/app"
export BOLOS_SDK=$NANOX_SDK
make clean && make -j$(nproc)
popd

# Run the tests
python3 "$SRCDIR/tests/run.py" --kpl-build-dir "$KPL_BUILDDIR" --model nanox --speculos-bin "$SPECULOS_BIN"
