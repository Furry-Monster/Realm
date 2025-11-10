#!/usr/bin/env python3
"""
RealmEngine Build Script
Provides build, format, and lint functionality with cross-platform support
"""

import argparse
import os
import platform
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
    NC = '\033[0m'  # No Color

def print_info(msg):
    print(f"{Colors.BLUE}[INFO]{Colors.NC} {msg}")

def print_success(msg):
    print(f"{Colors.GREEN}[SUCCESS]{Colors.NC} {msg}")

def print_warning(msg):
    print(f"{Colors.YELLOW}[WARNING]{Colors.NC} {msg}")

def print_error(msg):
    print(f"{Colors.RED}[ERROR]{Colors.NC} {msg}")

def detect_platform():
    """Detect the current platform"""
    system = platform.system()
    if system == "Linux":
        return "linux"
    elif system == "Darwin":
        return "macos"
    elif system == "Windows":
        return "windows"
    else:
        return "unknown"

def get_cpu_count():
    """Get the number of CPU cores"""
    try:
        return os.cpu_count() or 4
    except:
        return 4

def find_program(name, alternatives=None):
    """Find a program in PATH"""
    if alternatives is None:
        alternatives = [name]

    for prog in alternatives:
        if shutil.which(prog):
            return prog
    return None

def get_build_generator(platform_type, preferred="Ninja"):
    """Get appropriate CMake generator for the platform"""
    if preferred == "Ninja":
        if find_program("ninja"):
            return "Ninja"
        else:
            print_warning("Ninja not found, falling back to default generator")
            return None
    return preferred

def run_command(cmd, check=True, capture_output=False, cwd=None):
    """Run a command and handle errors"""
    try:
        result = subprocess.run(
            cmd,
            check=check,
            capture_output=capture_output,
            cwd=cwd,
            text=True
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

def format_code(verbose=False):
    """Format code using clang-format"""
    clang_format = find_program("clang-format")
    if not clang_format:
        print_error("clang-format not found. Please install clang-format.")
        return False

    print_info("Formatting source files...")

    src_dir = Path("src")
    if not src_dir.exists():
        print_error("src directory not found")
        return False

    # Find all source files
    source_files = []
    for ext in ["*.cpp", "*.h", "*.hpp"]:
        source_files.extend(src_dir.rglob(ext))

    if not source_files:
        print_warning("No source files found to format")
        return True

    # Format each file
    for file in source_files:
        if verbose:
            print(f"  Formatting {file}")
        run_command([clang_format, "-i", str(file)], check=False)

    print_success("Formatting completed")
    return True

def lint_code(fix=False, verbose=False):
    """Run clang-tidy on source files"""
    clang_tidy = find_program("clang-tidy")
    if not clang_tidy:
        print_error("clang-tidy not found. Please install clang-tidy.")
        return False

    compile_commands = Path("build/compile_commands.json")
    if not compile_commands.exists():
        print_error("compile_commands.json not found.")
        print_info("Please build the project first to generate compile_commands.json")
        print_info("Run: python build.py --configure")
        return False

    print_info("Running clang-tidy...")

    src_dir = Path("src")
    if not src_dir.exists():
        print_error("src directory not found")
        return False

    # Find all source files
    source_files = []
    for ext in ["*.cpp", "*.h", "*.hpp"]:
        source_files.extend(src_dir.rglob(ext))

    if not source_files:
        print_warning("No source files found to lint")
        return True

    # Run clang-tidy on each file
    cmd = [clang_tidy, "-p", "build"]
    if fix:
        cmd.extend(["--fix", "--fix-errors"])

    errors = []
    for file in source_files:
        if verbose:
            print(f"  Linting {file}")
        result = run_command(cmd + [str(file)], check=False, capture_output=True)
        if result.returncode != 0:
            errors.append(file)
            if not verbose:
                print(result.stdout)
                print(result.stderr)

    if errors:
        print_error(f"Linting found issues in {len(errors)} file(s)")
        return False
    else:
        action = "Fixed" if fix else "Checked"
        print_success(f"Linting {action.lower()} - no issues found")
        return True

def configure_cmake(build_dir, build_type, generator, verbose=False):
    """Configure CMake"""
    print_info("Configuring CMake...")
    print_info(f"  Build type: {build_type}")
    print_info(f"  Generator: {generator or 'Default'}")
    print_info(f"  Build directory: {build_dir}")

    build_path = Path(build_dir)
    build_path.mkdir(exist_ok=True)

    cmd = ["cmake", "..", f"-DCMAKE_BUILD_TYPE={build_type}"]
    if generator:
        cmd.extend(["-G", generator])

    if verbose:
        cmd.append("-DCMAKE_VERBOSE_MAKEFILE=ON")

    run_command(cmd, cwd=build_path, capture_output=not verbose)
    print_success("CMake configuration completed")

def build_project(build_dir, generator, jobs, target=None, verbose=False):
    """Build the project"""
    print_info("Building project...")
    print_info(f"  Jobs: {jobs}")
    if target:
        print_info(f"  Target: {target}")

    build_path = Path(build_dir)

    if generator == "Ninja":
        cmd = ["ninja"]
        if target:
            cmd.append(target)
    else:
        cmd = ["cmake", "--build", ".", "-j", str(jobs)]
        if target:
            cmd.extend(["--target", target])

    if not verbose:
        # Filter output to show only important messages
        result = run_command(cmd, check=False, cwd=build_path, capture_output=True)
        output = result.stdout + result.stderr
        for line in output.splitlines():
            if any(keyword in line.lower() for keyword in ["error", "warning", "building", "linking"]):
                print(line)
        if result.returncode != 0:
            print_error("Build failed")
            sys.exit(1)
    else:
        run_command(cmd, cwd=build_path)

    print_success("Build completed successfully")

def install_project(build_dir, generator):
    """Install the project"""
    print_info("Installing...")
    build_path = Path(build_dir)

    if generator == "Ninja":
        run_command(["ninja", "install"], cwd=build_path)
    else:
        run_command(["cmake", "--build", ".", "--target", "install"], cwd=build_path)

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
        description="<<<RealmEngine Building Script>>>",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python build.py                    # Default build (Debug, Ninja)
  python build.py --type Release     # Release build
  python build.py --clean --run      # Clean build and run
        """
    )

    # Build options
    parser.add_argument("-t", "--type", default="Debug",
                        choices=["Debug", "Release", "RelWithDebInfo", "MinSizeRel"],
                        help="Build type (default: Debug)")
    parser.add_argument("-d", "--dir", default="build",
                        help="Build directory (default: build)")
    parser.add_argument("-g", "--generator", default="Ninja",
                        choices=["Ninja", "Unix Makefiles", "Visual Studio 17 2022", "Xcode"],
                        help="CMake generator (default: Ninja)")
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

    # Action options
    parser.add_argument("--configure", action="store_true",
                        help="Only run CMake configuration, don't build")
    parser.add_argument("--build", action="store_true",
                        help="Only build, skip CMake configuration")
    parser.add_argument("--format", action="store_true",
                        help="Format code using clang-format")
    parser.add_argument("--lint", action="store_true",
                        help="Run clang-tidy linting")
    parser.add_argument("--lint-fix", action="store_true",
                        help="Run clang-tidy with auto-fix")

    args = parser.parse_args()

    # Detect platform
    platform_type = detect_platform()
    if platform_type == "unknown":
        print_warning(f"Unknown platform: {platform.system()}")

    # Adjust generator based on platform
    if args.generator == "Ninja":
        args.generator = get_build_generator(platform_type, "Ninja")

    # Clean if requested
    if args.clean:
        build_path = Path(args.dir)
        if build_path.exists():
            print_info(f"Cleaning build directory: {args.dir}")
            shutil.rmtree(build_path)

    # Format code
    if args.format:
        format_code(args.verbose)
        if not args.lint and not args.lint_fix and not args.configure and not args.build:
            return

    # Lint code
    if args.lint or args.lint_fix:
        lint_code(fix=args.lint_fix, verbose=args.verbose)
        if not args.configure and not args.build:
            return

    # Configure CMake
    if not args.build:
        configure_cmake(args.dir, args.type, args.generator, args.verbose)

    # Exit if only configuration was requested
    if args.configure:
        print_success("Configuration complete. Exiting.")
        return

    # Build
    build_project(args.dir, args.generator, args.jobs, args.target, args.verbose)

    # Install if requested
    if args.install:
        install_project(args.dir, args.generator)

    # Run if requested
    if args.run:
        executable = Path("bin/RealmEngine")
        if platform_type == "windows":
            executable = Path("bin/RealmEngine.exe")
        run_executable(executable)

    print_success("All done!")

if __name__ == "__main__":
    main()

