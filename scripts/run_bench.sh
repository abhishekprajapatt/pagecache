#!/bin/bash

set -e

if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir -p build
fi

cd build

echo "Configuring CMake..."
cmake ..

echo "Building..."
make -j$(nproc)

echo "Building complete. Running benchmarks..."
./benchmark

echo ""
echo "Benchmark complete. Results saved above."
