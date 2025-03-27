#!/bin/bash

set -e

mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=RELEASE ..
cmake --build . --target mandel -j 16
