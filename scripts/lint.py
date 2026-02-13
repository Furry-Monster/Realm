#!/usr/bin/env python3
"""
RealmEngine Code Linting Script
Run clang-tidy on C++ source code
"""

import argparse
import json
import sys
from pathlib import Path

# Add scripts directory to path
sys.path.insert(0, str(Path(__file__).parent))

from build_config import get_build_config


def find_source_files(src_dir: Path, extensions: list = None) -> list:
    """Find all source files in directory"""
    if extensions is None:
        extensions = [".cpp", ".cc", ".cxx"]  # Only C++ implementation files
    
    files = []
    for ext in extensions:
        files.extend(src_dir.rglob(f"*{ext}"))
    
    return sorted(files)


def check_compile_commands(build_dir: Path) -> bool:
    """Check if compile_commands.json exists"""
    compile_commands = build_dir / "compile_commands.json"
    return compile_commands.exists()


def lint_files(
    files: list,
    clang_tidy: str,
    build_dir: Path,
    fix: bool = False,
    checks: str = None,
    verbose: bool = False
):
    """Lint files using clang-tidy"""
    config = get_build_config()
    logger = config.logger
    
    if fix:
        logger.info("Running clang-tidy with auto-fix...")
    else:
        logger.info("Running clang-tidy...")
    
    total = len(files)
    passed = 0
    failed = 0
    
    # Prepare base command
    cmd = [clang_tidy, "-p", str(build_dir)]
    
    if fix:
        cmd.extend(["--fix", "--fix-errors"])
    
    if checks:
        cmd.extend(["-checks", checks])
    
    for i, file in enumerate(files, 1):
        if verbose:
            logger.info(f"[{i}/{total}] Linting {file.relative_to(config.project_root)}")
        
        try:
            result = config.run_command(
                cmd + [str(file)],
                capture_output=True,
                check=False
            )
            
            output = result.stdout or ""
            
            if result.returncode != 0:
                failed += 1
                logger.error(f"  Issues found in: {file.relative_to(config.project_root)}")
                if output and not verbose:
                    # Print relevant lines
                    for line in output.splitlines():
                        if "warning:" in line or "error:" in line:
                            print(f"    {line}")
            else:
                passed += 1
                if verbose:
                    logger.success(f"  OK")
            
            if verbose and output:
                print(output)
        
        except Exception as e:
            failed += 1
            logger.error(f"  Error linting {file}: {e}")
    
    # Print summary
    logger.info("=" * 60)
    if failed == 0:
        logger.success(f"All {total} files passed linting")
        return 0
    else:
        logger.warning(f"Passed: {passed}, Failed: {failed}")
        return 1


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="RealmEngine Code Linting Script",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python lint.py                      # Lint all source files
  python lint.py --fix                # Lint and auto-fix issues
  python lint.py -d src/core          # Lint specific directory
  python lint.py -v                   # Verbose output
  python lint.py -c "modernize-*"     # Use specific checks
        """
    )
    
    parser.add_argument(
        "-d", "--directory",
        type=str,
        help="Directory to lint (default: src)"
    )
    
    parser.add_argument(
        "-b", "--build-dir",
        type=str,
        default="build",
        help="Build directory with compile_commands.json (default: build)"
    )
    
    parser.add_argument(
        "--fix",
        action="store_true",
        help="Automatically fix issues"
    )
    
    parser.add_argument(
        "-c", "--checks",
        type=str,
        help="Clang-tidy checks to run (e.g., 'modernize-*,readability-*')"
    )
    
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Verbose output"
    )
    
    parser.add_argument(
        "--clang-tidy",
        type=str,
        help="Path to clang-tidy executable"
    )
    
    args = parser.parse_args()
    
    # Get configuration
    config = get_build_config()
    logger = config.logger
    
    # Find clang-tidy
    if args.clang_tidy:
        clang_tidy = args.clang_tidy
    else:
        clang_tidy = config.find_program("clang-tidy")
    
    if not clang_tidy:
        logger.error("clang-tidy not found")
        logger.info("Please install clang-tidy or specify path with --clang-tidy")
        return 1
    
    # Get version
    try:
        result = config.run_command(
            [clang_tidy, "--version"],
            capture_output=True,
            check=False
        )
        if result.returncode == 0:
            version_line = result.stdout.strip().splitlines()[0]
            logger.info(f"Using {version_line}")
    except:
        pass
    
    # Check build directory and compile_commands.json
    build_dir = config.project_root / args.build_dir
    
    if not build_dir.exists():
        logger.error(f"Build directory not found: {build_dir}")
        logger.info("Please build the project first:")
        logger.info("  python scripts/build.py --configure")
        return 1
    
    if not check_compile_commands(build_dir):
        logger.error(f"compile_commands.json not found in {build_dir}")
        logger.info("Please configure the project with CMake:")
        logger.info("  python scripts/build.py --configure")
        return 1
    
    # Determine source directory
    if args.directory:
        src_dir = config.project_root / args.directory
    else:
        src_dir = config.src_dir
    
    if not src_dir.exists():
        logger.error(f"Directory not found: {src_dir}")
        return 1
    
    # Find source files
    logger.info(f"Scanning directory: {src_dir.relative_to(config.project_root)}")
    files = find_source_files(src_dir)
    
    if not files:
        logger.warning("No source files found")
        return 0
    
    logger.info(f"Found {len(files)} source files")
    
    # Lint files
    return lint_files(files, clang_tidy, build_dir, args.fix, args.checks, args.verbose)


if __name__ == "__main__":
    sys.exit(main())
