#!/bin/bash

# Lint-fix script for RealmEngine
# This script runs clang-tidy with auto-fix on all source files

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Check if clang-tidy is available
if ! command -v clang-tidy &> /dev/null; then
    echo "Error: clang-tidy not found. Please install clang-tidy."
    exit 1
fi

# Check if compile_commands.json exists
if [ ! -f "build/compile_commands.json" ]; then
    echo "Error: compile_commands.json not found."
    echo "Please build the project first to generate compile_commands.json"
    echo "Run: cd build && cmake .. && make"
    exit 1
fi

echo "Running clang-tidy with auto-fix on source files..."

# Run clang-tidy with auto-fix on all source files
find src -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec clang-tidy {} -p build --fix --fix-errors \;

echo "Lint-fix complete!"

