#!/usr/bin/env python3
"""
RealmEngine Build Script - Main Entry Point
Cross-platform build system for Windows, Linux, and macOS
"""

import sys
from pathlib import Path

# Add scripts directory to path
scripts_dir = Path(__file__).parent / "scripts"
sys.path.insert(0, str(scripts_dir))

# Import and run main build script
from build import main

if __name__ == "__main__":
    sys.exit(main())
