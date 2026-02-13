#!/usr/bin/env python3
"""
RealmEngine Test Runner
Run tests and generate reports
"""

import argparse
from pathlib import Path
import sys


# Add scripts directory to path
sys.path.insert(0, str(Path(__file__).parent))

from build_config import get_build_config


def find_test_executables(test_dir: Path) -> list:
    """Find test executables"""
    if not test_dir.exists():
        return []

    executables = []
    for file in test_dir.iterdir():
        if file.is_file() and file.stat().st_mode & 0o111:  # Check if executable
            if "test" in file.name.lower():
                executables.append(file)

    return sorted(executables)


def run_tests(executables: list, verbose: bool = False):
    """Run test executables"""
    config = get_build_config()
    logger = config.logger

    if not executables:
        logger.warning("No test executables found")
        return 0

    logger.info(f"Found {len(executables)} test executable(s)")
    logger.info("=" * 60)

    passed = 0
    failed = 0

    for i, exe in enumerate(executables, 1):
        logger.info(f"[{i}/{len(executables)}] Running: {exe.name}")

        try:
            result = config.run_command([str(exe)], capture_output=not verbose, check=False)

            if result.returncode == 0:
                passed += 1
                logger.success("  PASSED")
            else:
                failed += 1
                logger.error(f"  FAILED (exit code: {result.returncode})")

                if not verbose and result.stdout:
                    print(result.stdout)
                if not verbose and result.stderr:
                    print(result.stderr)

        except Exception as e:
            failed += 1
            logger.exception(f"  ERROR: {e}")

    # Print summary
    logger.info("=" * 60)
    logger.info(f"Tests run: {len(executables)}")
    logger.info(f"Passed: {passed}")
    logger.info(f"Failed: {failed}")

    if failed == 0:
        logger.success("All tests passed!")
        return 0
    else:
        logger.error(f"{failed} test(s) failed")
        return 1


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="RealmEngine Test Runner",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python test.py                    # Run all tests
  python test.py -v                 # Verbose output
  python test.py -t Release         # Run Release build tests
        """,
    )

    parser.add_argument(
        "-t",
        "--type",
        type=str,
        default="Debug",
        choices=["Debug", "Release", "RelWithDebInfo", "MinSizeRel"],
        help="Build type (default: Debug)",
    )

    parser.add_argument(
        "-d", "--test-dir", type=str, help="Test directory (default: bin/tests or bin/Debug/tests)"
    )

    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")

    args = parser.parse_args()

    # Get configuration
    config = get_build_config()
    logger = config.logger

    # Determine test directory
    if args.test_dir:
        test_dir = config.project_root / args.test_dir
    else:
        # Try common locations
        possible_dirs = [
            config.bin_dir / "tests",
            config.bin_dir / args.type / "tests",
            config.bin_dir,
        ]

        test_dir = None
        for dir_path in possible_dirs:
            if dir_path.exists():
                test_dir = dir_path
                break

        if not test_dir:
            test_dir = config.bin_dir

    logger.info(f"Test directory: {test_dir.relative_to(config.project_root)}")

    # Find test executables
    executables = find_test_executables(test_dir)

    # Run tests
    return run_tests(executables, args.verbose)


if __name__ == "__main__":
    sys.exit(main())
