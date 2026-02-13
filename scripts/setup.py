#!/usr/bin/env python3
"""
RealmEngine Setup Script
First-time setup and environment check
"""

import argparse
import platform
import shutil
import sys
from pathlib import Path

# Add scripts directory to path
sys.path.insert(0, str(Path(__file__).parent))

from build_config import get_build_config


def check_python_version():
    """Check Python version"""
    config = get_build_config()
    logger = config.logger
    
    version = sys.version_info
    if version < (3, 6):
        logger.error(f"Python 3.6+ required, found {version.major}.{version.minor}")
        return False
    
    logger.success(f"Python {version.major}.{version.minor}.{version.micro}")
    return True


def check_cmake():
    """Check CMake installation"""
    config = get_build_config()
    logger = config.logger
    
    cmake = config.find_program("cmake")
    if not cmake:
        logger.error("CMake not found")
        logger.info("Install from: https://cmake.org/download/")
        return False
    
    try:
        result = config.run_command(
            ["cmake", "--version"],
            capture_output=True,
            check=False
        )
        if result.returncode == 0:
            version = result.stdout.splitlines()[0]
            logger.success(version)
            return True
    except:
        pass
    
    return False


def check_compiler():
    """Check C++ compiler"""
    config = get_build_config()
    logger = config.logger
    
    if config.platform.value == "Windows":
        # Check for MSVC or MinGW
        if config.find_program("cl"):
            logger.success("Found MSVC (cl)")
            return True
        elif config.find_program("g++"):
            logger.success("Found MinGW (g++)")
            return True
        else:
            logger.error("No C++ compiler found")
            logger.info("Install Visual Studio or MinGW")
            return False
    else:
        # Check for GCC or Clang
        if config.find_program("g++"):
            try:
                result = config.run_command(
                    ["g++", "--version"],
                    capture_output=True,
                    check=False
                )
                if result.returncode == 0:
                    version = result.stdout.splitlines()[0]
                    logger.success(f"Found {version}")
                    return True
            except:
                pass
        
        if config.find_program("clang++"):
            try:
                result = config.run_command(
                    ["clang++", "--version"],
                    capture_output=True,
                    check=False
                )
                if result.returncode == 0:
                    version = result.stdout.splitlines()[0]
                    logger.success(f"Found {version}")
                    return True
            except:
                pass
        
        logger.error("No C++ compiler found")
        logger.info("Install GCC or Clang")
        return False


def check_optional_tools():
    """Check optional build tools"""
    config = get_build_config()
    logger = config.logger
    
    tools = {
        "ninja": "Ninja build system (faster builds)",
        "clang-format": "Code formatting tool",
        "clang-tidy": "Code linting tool",
        "git": "Version control system",
    }
    
    logger.info("\nOptional tools:")
    for tool, description in tools.items():
        if config.find_program(tool):
            logger.success(f"  ✓ {tool} - {description}")
        else:
            logger.warning(f"  ✗ {tool} - {description} (not found)")


def check_git_submodules():
    """Check if git submodules are initialized"""
    config = get_build_config()
    logger = config.logger
    
    libs_dir = config.project_root / "libs"
    
    # Check if libs directory has content
    if not libs_dir.exists():
        logger.error("libs/ directory not found")
        return False
    
    # Check for key libraries
    required_libs = ["glfw", "glad", "glm", "assimp", "spdlog", "imgui", "stb"]
    missing = []
    
    for lib in required_libs:
        lib_path = libs_dir / lib
        if not lib_path.exists() or not any(lib_path.iterdir()):
            missing.append(lib)
    
    if missing:
        logger.error(f"Missing or empty submodules: {', '.join(missing)}")
        logger.info("Initialize submodules with:")
        logger.info("  git submodule update --init --recursive")
        return False
    
    logger.success("Git submodules initialized")
    return True


def check_project_structure():
    """Check project directory structure"""
    config = get_build_config()
    logger = config.logger
    
    required_dirs = ["src", "libs", "assets", "shaders", "scripts"]
    required_files = ["CMakeLists.txt", "build.py"]
    
    missing_dirs = []
    missing_files = []
    
    for dir_name in required_dirs:
        if not (config.project_root / dir_name).exists():
            missing_dirs.append(dir_name)
    
    for file_name in required_files:
        if not (config.project_root / file_name).exists():
            missing_files.append(file_name)
    
    if missing_dirs or missing_files:
        if missing_dirs:
            logger.error(f"Missing directories: {', '.join(missing_dirs)}")
        if missing_files:
            logger.error(f"Missing files: {', '.join(missing_files)}")
        return False
    
    logger.success("Project structure verified")
    return True


def print_next_steps():
    """Print next steps for user"""
    config = get_build_config()
    logger = config.logger
    
    logger.info("\n" + "=" * 60)
    logger.info("Next Steps:")
    logger.info("=" * 60)
    logger.info("1. Build the project:")
    logger.info("   python build.py")
    logger.info("")
    logger.info("2. Build and run:")
    logger.info("   python build.py -r")
    logger.info("")
    logger.info("3. Release build:")
    logger.info("   python build.py -t Release")
    logger.info("")
    logger.info("For more options, run: python build.py --help")
    logger.info("Documentation: scripts/README.md")
    logger.info("=" * 60)


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="RealmEngine Setup Script",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
This script checks your environment and verifies that all required
tools are installed for building RealmEngine.
        """
    )
    
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Verbose output"
    )
    
    args = parser.parse_args()
    
    # Get configuration
    config = get_build_config()
    logger = config.logger
    
    logger.info("=" * 60)
    logger.info("RealmEngine Setup and Environment Check")
    logger.info("=" * 60)
    logger.info(f"Platform: {config.platform.value}")
    logger.info(f"Python: {sys.version.split()[0]}")
    logger.info("=" * 60)
    
    # Run checks
    checks = []
    
    logger.info("\nChecking required tools...")
    checks.append(("Python version", check_python_version()))
    checks.append(("CMake", check_cmake()))
    checks.append(("C++ compiler", check_compiler()))
    
    logger.info("\nChecking project...")
    checks.append(("Project structure", check_project_structure()))
    checks.append(("Git submodules", check_git_submodules()))
    
    check_optional_tools()
    
    # Summary
    logger.info("\n" + "=" * 60)
    logger.info("Summary:")
    logger.info("=" * 60)
    
    passed = sum(1 for _, result in checks if result)
    total = len(checks)
    
    for name, result in checks:
        status = "✓" if result else "✗"
        logger.info(f"{status} {name}")
    
    logger.info("=" * 60)
    
    if passed == total:
        logger.success(f"All checks passed ({passed}/{total})")
        print_next_steps()
        return 0
    else:
        logger.error(f"Some checks failed ({passed}/{total})")
        logger.info("Please install missing tools and try again")
        return 1


if __name__ == "__main__":
    sys.exit(main())
