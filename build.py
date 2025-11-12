#!/usr/bin/env python3
"""
RealmEngine Build Script - Platform Router
根据平台自动调用对应的构建脚本
"""

import platform
import subprocess
import sys
from pathlib import Path

def main():
    system = platform.system()
    
    if system == "Windows":
        script = Path(__file__).parent / "build_windows.py"
        if not script.exists():
            print("Error: build_windows.py not found")
            sys.exit(1)
        subprocess.run([sys.executable, str(script)] + sys.argv[1:])
    elif system in ["Linux", "Darwin"]:
        script = Path(__file__).parent / "build_unix.py"
        if not script.exists():
            print("Error: build_unix.py not found")
            sys.exit(1)
        subprocess.run([sys.executable, str(script)] + sys.argv[1:])
    else:
        print(f"Error: Unsupported platform: {system}")
        print("Please use build_windows.py for Windows or build_unix.py for Linux/macOS")
        sys.exit(1)

if __name__ == "__main__":
    main()

