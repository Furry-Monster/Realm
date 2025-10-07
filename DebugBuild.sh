#!/bin/bash

set -e

echo "🔥Creating build directory..."
mkdir -p build

echo "🔥Running cmake in build directory..."
cd build
cmake ..

echo "🔥Running ninja build..."
ninja

echo "🔥Running Realm Engine program..."
./src/RealmEngine