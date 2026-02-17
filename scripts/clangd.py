#!/usr/bin/env python3
"""
Generate compile_commands.json for clangd (configure only, no build).
Uses Ninja by default so compile_commands.json is produced; with VS env, Ninja uses cl.exe.
"""

import argparse
import shutil
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))

from build_config import BuildConfig, BuildType, get_build_config
from build import Builder


def main():
    parser = argparse.ArgumentParser(
        description="Generate compile_commands.json for clangd (configure only, no build)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Use VS toolchain (MSVC): run this script from "x64 Native Tools Command Prompt for VS"
(or "Developer Command Prompt for VS"). Ninja will use cl.exe from the environment;
compile_commands.json will match your VS build. No extra flags needed.
        """.strip(),
    )
    parser.add_argument(
        "-d",
        "--dir",
        type=str,
        default="build_clangd",
        help="Build directory for clangd config (default: build_clangd, avoids clashing with existing build/)",
    )
    parser.add_argument(
        "-g",
        "--generator",
        type=str,
        help="CMake generator (auto-detected if not specified)",
    )
    parser.add_argument(
        "--no-copy",
        action="store_true",
        help="Do not copy compile_commands.json to project root",
    )
    parser.add_argument(
        "-D",
        action="append",
        dest="cmake_defines",
        metavar="VAR=VALUE",
        help="Pass -D options to CMake (can be used multiple times)",
    )
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")
    args = parser.parse_args()

    config = get_build_config()
    logger = config.logger

    if not config.check_cmake():
        return 1

    tools_ok, missing = config.check_build_tools()
    if not tools_ok:
        logger.error(f"Missing required build tools: {', '.join(missing)}")
        return 1

    build_dir = config.project_root / args.dir
    default_gen = config.get_cmake_generator()
    if args.generator:
        generator = args.generator
    elif config.find_program("ninja"):
        generator = "Ninja"
    else:
        generator = default_gen

    if generator != "Ninja" and ("Visual Studio" in generator or generator == "Xcode"):
        logger.warning(
            f"{generator} does not generate compile_commands.json. "
            "Use Ninja: install it and re-run without -g, or pass -g Ninja."
        )

    logger.info("Generating compile_commands.json for clangd...")
    logger.info(f"  Build directory: {build_dir}")
    logger.info(f"  Generator: {generator}")

    cmake_args = []
    if args.cmake_defines:
        for define in args.cmake_defines:
            cmake_args.append(f"-D{define}")

    builder = Builder(config)
    if not builder.configure(
        build_dir,
        BuildType.DEBUG,
        generator,
        extra_args=cmake_args,
        verbose=args.verbose,
    ):
        return 1

    cc_path = build_dir / "compile_commands.json"
    if not cc_path.exists():
        logger.error("compile_commands.json was not generated")
        if "Visual Studio" in generator or generator == "Xcode":
            logger.info("Use Ninja generator for this script (install Ninja, then re-run without -g)")
        return 1

    if not args.no_copy:
        root_cc = config.project_root / "compile_commands.json"
        try:
            shutil.copy2(cc_path, root_cc)
            logger.success(f"Copied compile_commands.json to {root_cc}")
        except Exception as e:
            logger.warning(f"Could not copy to root: {e}")

    logger.success("clangd completion data ready")
    return 0


if __name__ == "__main__":
    sys.exit(main())
