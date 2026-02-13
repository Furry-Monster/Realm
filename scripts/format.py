#!/usr/bin/env python3
"""
RealmEngine Code Formatting Script
Format C++ source code using clang-format
"""

import argparse
import sys
from pathlib import Path

# Add scripts directory to path
sys.path.insert(0, str(Path(__file__).parent))

from build_config import get_build_config


def find_source_files(src_dir: Path, extensions: list = None) -> list:
    """Find all source files in directory"""
    if extensions is None:
        extensions = [".cpp", ".h", ".hpp", ".cc", ".cxx", ".hxx"]
    
    files = []
    for ext in extensions:
        files.extend(src_dir.rglob(f"*{ext}"))
    
    return sorted(files)


def format_files(files: list, clang_format: str, check_only: bool = False, verbose: bool = False):
    """Format files using clang-format"""
    config = get_build_config()
    logger = config.logger
    
    if check_only:
        logger.info("Checking code formatting...")
    else:
        logger.info("Formatting code...")
    
    total = len(files)
    formatted = 0
    errors = 0
    
    for i, file in enumerate(files, 1):
        if verbose:
            logger.info(f"[{i}/{total}] {file.relative_to(config.project_root)}")
        
        try:
            if check_only:
                # Check if file needs formatting
                result = config.run_command(
                    [clang_format, "--dry-run", "-Werror", str(file)],
                    capture_output=True,
                    check=False
                )
                if result.returncode != 0:
                    formatted += 1
                    logger.warning(f"  Needs formatting: {file.relative_to(config.project_root)}")
            else:
                # Format file in-place
                result = config.run_command(
                    [clang_format, "-i", str(file)],
                    capture_output=True,
                    check=False
                )
                if result.returncode == 0:
                    formatted += 1
                else:
                    errors += 1
                    logger.error(f"  Failed to format: {file.relative_to(config.project_root)}")
        except Exception as e:
            errors += 1
            logger.error(f"  Error processing {file}: {e}")
    
    # Print summary
    logger.info("=" * 60)
    if check_only:
        if formatted == 0:
            logger.success(f"All {total} files are properly formatted")
            return 0
        else:
            logger.warning(f"{formatted} out of {total} files need formatting")
            return 1
    else:
        if errors == 0:
            logger.success(f"Formatted {formatted} files successfully")
            return 0
        else:
            logger.error(f"Formatted {formatted} files, {errors} errors")
            return 1


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="RealmEngine Code Formatting Script",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python format.py                  # Format all source files
  python format.py --check          # Check formatting without modifying
  python format.py -d src/core      # Format specific directory
  python format.py -v               # Verbose output
        """
    )
    
    parser.add_argument(
        "-d", "--directory",
        type=str,
        help="Directory to format (default: src)"
    )
    
    parser.add_argument(
        "-c", "--check",
        action="store_true",
        help="Check formatting without modifying files"
    )
    
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Verbose output"
    )
    
    parser.add_argument(
        "--clang-format",
        type=str,
        help="Path to clang-format executable"
    )
    
    args = parser.parse_args()
    
    # Get configuration
    config = get_build_config()
    logger = config.logger
    
    # Find clang-format
    if args.clang_format:
        clang_format = args.clang_format
    else:
        clang_format = config.find_program("clang-format")
    
    if not clang_format:
        logger.error("clang-format not found")
        logger.info("Please install clang-format or specify path with --clang-format")
        return 1
    
    # Get version
    try:
        result = config.run_command(
            [clang_format, "--version"],
            capture_output=True,
            check=False
        )
        if result.returncode == 0:
            logger.info(f"Using {result.stdout.strip()}")
    except:
        pass
    
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
    
    # Format files
    return format_files(files, clang_format, args.check, args.verbose)


if __name__ == "__main__":
    sys.exit(main())
