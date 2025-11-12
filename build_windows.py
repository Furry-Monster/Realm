#!/usr/bin/env python3
"""
RealmEngine Build Script for Windows
using MSVC
"""

import argparse
import locale
import os
import shutil
import subprocess
import sys
from pathlib import Path

# Colors for terminal output
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'

def print_info(msg):
    print(f"{Colors.BLUE}[INFO]{Colors.NC} {msg}")

def print_success(msg):
    print(f"{Colors.GREEN}[SUCCESS]{Colors.NC} {msg}")

def print_warning(msg):
    print(f"{Colors.YELLOW}[WARNING]{Colors.NC} {msg}")

def print_error(msg):
    print(f"{Colors.RED}[ERROR]{Colors.NC} {msg}")

def safe_print(text):
    """Safely print text, handling encoding errors"""
    try:
        print(text, flush=True)
    except (UnicodeEncodeError, UnicodeDecodeError):
        # Replace problematic characters if encoding fails
        safe_text = text.encode('utf-8', errors='replace').decode('utf-8', errors='replace')
        try:
            print(safe_text, flush=True)
        except:
            # Last resort: print as bytes
            sys.stdout.buffer.write(safe_text.encode('utf-8', errors='replace'))
            sys.stdout.buffer.write(b'\n')
            sys.stdout.buffer.flush()

def get_cpu_count():
    """Get the number of CPU cores"""
    try:
        return os.cpu_count() or 4
    except:
        return 4

def get_encoding():
    """Get Windows system encoding"""
    try:
        return locale.getpreferredencoding(False) or 'utf-8'
    except:
        return 'utf-8'

def run_command(cmd, check=True, capture_output=False, cwd=None):
    """Run a command with Windows encoding support"""
    encoding = get_encoding()
    try:
        result = subprocess.run(
            cmd,
            check=check,
            capture_output=capture_output,
            cwd=cwd,
            text=True,
            encoding=encoding,
            errors='replace'
        )
        return result
    except subprocess.CalledProcessError as e:
        print_error(f"Command failed: {' '.join(cmd)}")
        if e.stdout:
            print(e.stdout)
        if e.stderr:
            print(e.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print_error(f"Command not found: {cmd[0]}")
        sys.exit(1)

def configure_cmake(build_dir, build_type, generator=None, verbose=False):
    """Configure CMake for Windows"""
    print_info("Configuring CMake...")
    print_info(f"  Build type: {build_type}")
    print_info(f"  Generator: {generator or 'Default (Visual Studio)'}")
    print_info(f"  Build directory: {build_dir}")

    build_path = Path(build_dir)
    build_path.mkdir(exist_ok=True)

    cmd = ["cmake", ".."]
    if generator:
        cmd.extend(["-G", generator])
    # Visual Studio generators are multi-config, don't set CMAKE_BUILD_TYPE

    if verbose:
        cmd.append("-DCMAKE_VERBOSE_MAKEFILE=ON")

    run_command(cmd, cwd=build_path, capture_output=not verbose)
    print_success("CMake configuration completed")

def build_project(build_dir, generator, jobs, build_type, target=None, verbose=False):
    """Build the project on Windows"""
    print_info("Building project...")
    print_info(f"  Build type: {build_type}")
    if target:
        print_info(f"  Target: {target}")

    build_path = Path(build_dir)

    # Visual Studio generators use --config for build type
    cmd = ["cmake", "--build", ".", "--config", build_type]
    if target:
        cmd.extend(["--target", target])
    if jobs > 1:
        cmd.extend(["-j", str(jobs)])

    if not verbose:
        result = run_command(cmd, check=False, cwd=build_path, capture_output=True)
        stdout = result.stdout or ""
        stderr = result.stderr or ""
        output = stdout + stderr
        for line in output.splitlines():
            if any(keyword in line.lower() for keyword in ["error", "warning", "building", "linking"]):
                safe_print(line)
        if result.returncode != 0:
            print_error("Build failed")
            sys.exit(1)
    else:
        run_command(cmd, cwd=build_path)

    print_success("Build completed successfully")

def install_project(build_dir, generator, build_type):
    """Install the project"""
    print_info("Installing...")
    build_path = Path(build_dir)
    cmd = ["cmake", "--build", ".", "--target", "install", "--config", build_type]
    run_command(cmd, cwd=build_path)
    print_success("Installation completed")

def run_executable(executable_path):
    """Run the executable"""
    exe_path = Path(executable_path)
    if not exe_path.exists():
        print_error(f"Executable not found: {exe_path}")
        print_info("Build may have failed or executable is in a different location")
        return False

    print_info(f"Running executable: {exe_path}")
    print_info("---")
    run_command([str(exe_path)])
    return True

def main():
    parser = argparse.ArgumentParser(
        description="RealmEngine Build Script for Windows",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python build_windows.py                    # Default build (Debug)
  python build_windows.py --type Release     # Release build
  python build_windows.py --clean --run      # Clean build and run
        """
    )

    parser.add_argument("-t", "--type", default="Debug",
                        choices=["Debug", "Release", "RelWithDebInfo", "MinSizeRel"],
                        help="Build type (default: Debug)")
    parser.add_argument("-d", "--dir", default="build",
                        help="Build directory (default: build)")
    parser.add_argument("-g", "--generator",
                        help="CMake generator (default: Visual Studio)")
    parser.add_argument("-j", "--jobs", type=int, default=get_cpu_count(),
                        help=f"Number of parallel jobs (default: {get_cpu_count()})")
    parser.add_argument("-T", "--target",
                        help="Build specific target only")
    parser.add_argument("-c", "--clean", action="store_true",
                        help="Clean build directory before building")
    parser.add_argument("-r", "--run", action="store_true",
                        help="Run the executable after building")
    parser.add_argument("-i", "--install", action="store_true",
                        help="Install after building")
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="Verbose output")
    parser.add_argument("--configure", action="store_true",
                        help="Only run CMake configuration, don't build")
    parser.add_argument("--build", action="store_true",
                        help="Only build, skip CMake configuration")

    args = parser.parse_args()

    # Clean if requested
    if args.clean:
        build_path = Path(args.dir)
        if build_path.exists():
            print_info(f"Cleaning build directory: {args.dir}")
            shutil.rmtree(build_path)

    # Configure CMake
    if not args.build:
        configure_cmake(args.dir, args.type, args.generator, args.verbose)

    # Exit if only configuration was requested
    if args.configure:
        print_success("Configuration complete. Exiting.")
        return

    # Build
    build_project(args.dir, args.generator, args.jobs, args.type, args.target, args.verbose)

    # Install if requested
    if args.install:
        install_project(args.dir, args.generator, args.type)

    # Run if requested
    if args.run:
        executable = Path("bin/RealmEngine.exe")
        run_executable(executable)

    print_success("All done!")

if __name__ == "__main__":
    main()

