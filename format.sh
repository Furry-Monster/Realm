#!/bin/bash

# Format script for RealmEngine
# This script formats all source files using clang-format

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Check if clang-format is available
if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format not found. Please install clang-format."
    exit 1
fi

echo "Formatting source files..."

# Find and format all C++ source files
find src -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec clang-format -i {} +

echo "Formatting complete!"

