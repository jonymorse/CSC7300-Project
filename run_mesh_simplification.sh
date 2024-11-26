#!/bin/bash

# Navigate to the project directory
cd "$(dirname "$0")"

# Navigate to the build directory
echo "Entering the build directory..."
cd build

# Run cmake
echo "Running CMake..."
cmake ..

# Compile the project
echo "Compiling the project with make..."
make

# Run the program
echo "Executing MeshSimplification..."
./MeshSimplification
