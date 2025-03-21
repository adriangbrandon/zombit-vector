#!/bin/sh

echo "Clean project"
sh ./clean.sh

echo "Download external projects"
git submodule init
git submodule update --init --recursive

echo "Create folder build"
mkdir -p build
cd build

echo "Run CMake"
arch -arch x86_64 /usr/local/bin/cmake -DCMAKE_BUILD_TYPE=Release ..

echo "Run make"
make

echo "DONE!!!"
