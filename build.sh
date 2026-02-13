#!/bin/bash
# RealmEngine Build Script Wrapper for Unix-like systems (Linux/macOS)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
python3 "${SCRIPT_DIR}/scripts/build.py" "$@"
