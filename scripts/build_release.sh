#!/bin/bash

# Build release

rm -rf build_release
mkdir build_release
cd build_release || exit
# export CC=/trinity/shared/apps/cv-standard/gcc/12.1.0/bin/gcc
# export CXX=/trinity/shared/apps/cv-standard/gcc/12.1.0/bin/g++
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
cd ..
