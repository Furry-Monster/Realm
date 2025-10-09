#!/bin/bash

set -e

echo "🔥Creating build directory..."
mkdir -p build

echo "🔥Running cmake in build directory..."
cd build
cmake ..