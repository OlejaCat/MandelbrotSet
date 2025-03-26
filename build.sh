#!/bin/bash

set -e

mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --target mandel -j 16
