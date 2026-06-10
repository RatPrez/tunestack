#!/usr/bin/env bash

set -e  # stop on first error

BUILD_DIR="build"

cmake -S . -B $BUILD_DIR $CMAKE_EXTRA
cmake --build $BUILD_DIR

ln -sf $BUILD_DIR/compile_commands.json compile_commands.json

./$BUILD_DIR/tunestack
