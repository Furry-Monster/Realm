#!/usr/bin/env python3
"""
RealmEngine Clean Script
Clean build artifacts and generated files
"""

import argparse
from pathlib import Path
import shutil
import sys


# Add scripts directory to path
sys.path.insert(0, str(Path(__file__).parent))

from build_config import get_build_config


def get_size_str(size_bytes: int) -> str:
    """Convert bytes to human-readable size"""
    for unit in ["B", "KB", "MB", "GB"]:
        if size_bytes < 1024.0:
            return f"{size_bytes:.2f} {unit}"
        size_bytes /= 1024.0
    return f"{size_bytes:.2f} TB"


def get_directory_size(path: Path) -> int:
    """Get total size of directory"""
    total = 0
    try:
        for item in path.rglob("*"):
            if item.is_file():
                total += item.stat().st_size
    except:
        pass
    return total


def clean_directory(path: Path, dry_run: bool = False, verbose: bool = False):
    """Clean a directory"""
    config = get_build_config()
    logger = config.logger

    if not path.exists():
        if verbose:
            logger.info(f"Directory does not exist: {path.relative_to(config.project_root)}")
        return 0

    size = get_directory_size(path)

    if dry_run:
        logger.info(f"Would remove: {path.relative_to(config.project_root)} ({get_size_str(size)})")
        return size

    try:
        logger.info(f"Removing: {path.relative_to(config.project_root)} ({get_size_str(size)})")
        shutil.rmtree(path)
        logger.success(f"Removed {path.relative_to(config.project_root)}")
        return size
    except Exception as e:
        logger.error(f"Failed to remove {path.relative_to(config.project_root)}: {e}")
        return 0


def clean_file(path: Path, dry_run: bool = False, verbose: bool = False):
    """Clean a file"""
    config = get_build_config()
    logger = config.logger

    if not path.exists():
        if verbose:
            logger.info(f"File does not exist: {path.relative_to(config.project_root)}")
        return 0

    size = path.stat().st_size

    if dry_run:
        logger.info(f"Would remove: {path.relative_to(config.project_root)} ({get_size_str(size)})")
        return size

    try:
        logger.info(f"Removing: {path.relative_to(config.project_root)}")
        path.unlink()
        return size
    except Exception as e:
        logger.error(f"Failed to remove {path.relative_to(config.project_root)}: {e}")
        return 0


CONFIG_EXTENSIONS = {".json", ".ini", ".toml", ".yaml", ".yml", ".cfg", ".conf"}


def is_config_file(path: Path) -> bool:
    return path.suffix.lower() in CONFIG_EXTENSIONS


def clean_bin_directory(path: Path, dry_run: bool = False, verbose: bool = False):
    """Clean bin directory while preserving config files"""
    config = get_build_config()
    logger = config.logger

    if not path.exists():
        if verbose:
            logger.info(f"Directory does not exist: {path.relative_to(config.project_root)}")
        return 0

    total_size = 0
    for item in path.rglob("*"):
        if item.is_file() and not is_config_file(item):
            total_size += clean_file(item, dry_run, verbose)
        elif item.is_file() and verbose:
            logger.info(f"Preserving config: {item.relative_to(path)}")

    all_dirs = [d for d in path.rglob("*") if d.is_dir()]
    for d in sorted(all_dirs, key=lambda p: len(p.parts), reverse=True):
        if d.exists() and not any(d.iterdir()) and not dry_run:
            d.rmdir()
            if verbose:
                logger.info(f"Removed empty dir: {d.relative_to(path)}")

    if total_size > 0:
        logger.success(
            f"Cleaned {path.relative_to(config.project_root)} ({get_size_str(total_size)})"
        )
    return total_size


def clean_pattern(directory: Path, pattern: str, dry_run: bool = False, verbose: bool = False):
    """Clean files matching pattern"""
    config = get_build_config()
    logger = config.logger

    if not directory.exists():
        return 0

    total_size = 0
    files = list(directory.rglob(pattern))

    if not files:
        if verbose:
            logger.info(
                f"No files matching '{pattern}' in {directory.relative_to(config.project_root)}"
            )
        return 0

    for file in files:
        if file.is_file():
            total_size += clean_file(file, dry_run, verbose)

    return total_size


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="RealmEngine Clean Script",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python clean.py                   # Clean build and bin (preserve configs)
  python clean.py --all             # Clean all generated files
  python clean.py --cache           # Clean cache files
  python clean.py --dry-run         # Show what would be removed
  python clean.py -v                # Verbose output
        """,
    )

    parser.add_argument(
        "--all",
        action="store_true",
        help="Clean all generated files (build, bin, cache)",
    )

    parser.add_argument("--build", action="store_true", help="Clean build directory")

    parser.add_argument("--bin", action="store_true", help="Clean binary output directory")

    parser.add_argument(
        "--cache", action="store_true", help="Clean cache files (.cache, CMake cache)"
    )

    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show what would be removed without actually removing",
    )

    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")

    args = parser.parse_args()

    # Get configuration
    config = get_build_config()
    logger = config.logger

    # Determine what to clean
    clean_build = args.all or args.build or not (args.bin or args.cache)
    clean_bin = args.all or args.bin or not (args.build or args.cache)
    clean_cache = args.all or args.cache

    if args.dry_run:
        logger.info("DRY RUN - No files will be removed")

    logger.info("=" * 60)

    total_size = 0

    # Clean build directory
    if clean_build:
        build_dir = config.project_root / "build"
        total_size += clean_directory(build_dir, args.dry_run, args.verbose)

    # Clean bin directory (preserve config files)
    if clean_bin:
        bin_dir = config.project_root / "bin"
        total_size += clean_bin_directory(bin_dir, args.dry_run, args.verbose)

    # Clean cache files
    if clean_cache:
        cache_dir = config.project_root / ".cache"
        total_size += clean_directory(cache_dir, args.dry_run, args.verbose)

        # Clean CMake cache files
        cmake_cache = config.project_root / "CMakeCache.txt"
        total_size += clean_file(cmake_cache, args.dry_run, args.verbose)

        cmake_files = config.project_root / "CMakeFiles"
        total_size += clean_directory(cmake_files, args.dry_run, args.verbose)

        # Clean .pyc files
        total_size += clean_pattern(
            config.project_root / "scripts", "*.pyc", args.dry_run, args.verbose
        )

        pycache = config.project_root / "scripts" / "__pycache__"
        total_size += clean_directory(pycache, args.dry_run, args.verbose)

    # Print summary
    logger.info("=" * 60)
    if args.dry_run:
        logger.info(f"Would free: {get_size_str(total_size)}")
    else:
        logger.success(f"Freed: {get_size_str(total_size)}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
