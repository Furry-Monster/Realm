#!/usr/bin/env python3
"""
RealmEngine Build Script
Cross-platform build script for Windows, Linux, and macOS
"""

import argparse
from pathlib import Path
import shutil
import sys


# Add scripts directory to path
sys.path.insert(0, str(Path(__file__).parent))

from typing import Optional

from build_config import BuildConfig, BuildType, Platform, get_build_config


class Builder:
    """Main build orchestrator"""

    def __init__(self, config: BuildConfig):
        self.config = config
        self.logger = config.logger

    def clean(self, build_dir: Path) -> bool:
        """Clean build directory"""
        if build_dir.exists():
            self.logger.info(f"Cleaning build directory: {build_dir}")
            try:
                shutil.rmtree(build_dir)
                self.logger.success("Build directory cleaned")
                return True
            except Exception as e:
                self.logger.error(f"Failed to clean build directory: {e}")
                return False
        else:
            self.logger.info("Build directory doesn't exist, nothing to clean")
            return True

    def configure(
        self,
        build_dir: Path,
        build_type: BuildType,
        generator: str,
        extra_args: Optional[list] = None,
        verbose: bool = False,
    ) -> bool:
        """Configure CMake"""
        self.logger.info("Configuring CMake...")
        self.logger.info(f"  Build type: {build_type.value}")
        self.logger.info(f"  Generator: {generator}")
        self.logger.info(f"  Build directory: {build_dir}")

        # Create build directory
        build_dir.mkdir(parents=True, exist_ok=True)

        # Prepare CMake command
        cmd = ["cmake", str(self.config.project_root)]
        cmd.extend(["-G", generator])

        # For Unix-like generators, set build type
        if generator not in ["Visual Studio", "Xcode"] and "Visual Studio" not in generator:
            cmd.append(f"-DCMAKE_BUILD_TYPE={build_type.value}")

        # Add extra arguments
        if extra_args:
            cmd.extend(extra_args)

        if verbose:
            cmd.append("-DCMAKE_VERBOSE_MAKEFILE=ON")

        build_enc = self.config.get_build_output_encoding()
        try:
            if verbose:
                result = self.config.run_command(
                    cmd, cwd=build_dir, check=False, encoding=build_enc
                )
                returncode = result.returncode
            else:
                returncode = self.config.run_command_streamed(
                    cmd, cwd=build_dir, encoding=build_enc
                )

            if returncode != 0:
                self.logger.error("CMake configuration failed")
                return False

            self.logger.success("CMake configuration completed")
            return True
        except Exception as e:
            self.logger.error(f"CMake configuration failed: {e}")
            return False

    def build(
        self,
        build_dir: Path,
        build_type: BuildType,
        generator: str,
        jobs: int,
        target: Optional[str] = None,
        verbose: bool = False,
    ) -> bool:
        """Build the project"""
        self.logger.info("Building project...")
        if target:
            self.logger.info(f"  Target: {target}")
        self.logger.info(f"  Jobs: {jobs}")

        # Prepare build command
        if generator == "Ninja":
            cmd = ["ninja"]
            if target:
                cmd.append(target)
            if jobs > 1:
                cmd.extend(["-j", str(jobs)])
        else:
            cmd = ["cmake", "--build", "."]

            # For multi-config generators (Visual Studio, Xcode)
            if "Visual Studio" in generator or generator == "Xcode":
                cmd.extend(["--config", build_type.value])

            if target:
                cmd.extend(["--target", target])

            if jobs > 1:
                cmd.extend(["-j", str(jobs)])

        # Run build
        build_enc = self.config.get_build_output_encoding()
        try:
            if verbose:
                result = self.config.run_command(
                    cmd, cwd=build_dir, check=False, encoding=build_enc
                )
                returncode = result.returncode
            else:
                _build_keywords = ("error", "warning", "building", "linking", "finished", "failed")

                def _build_filter(line: str) -> bool:
                    ll = line.lower()
                    return any(kw in ll for kw in _build_keywords)

                returncode = self.config.run_command_streamed(
                    cmd, cwd=build_dir, line_filter=_build_filter, encoding=build_enc
                )

            if returncode != 0:
                self.logger.error("Build failed")
                return False

            self.logger.success("Build completed successfully")
            return True
        except Exception as e:
            self.logger.error(f"Build failed: {e}")
            return False

    def run_executable(
        self, build_type: BuildType, target: str = "RealmEditor", args: list = None
    ) -> bool:
        """Run the built executable"""
        exe_path = self.config.get_executable_path(build_type, target=target)

        if not exe_path.exists():
            self.logger.error(f"Executable not found: {exe_path}")
            self.logger.info("Build may have failed or executable is in a different location")
            return False

        self.logger.info(f"Running executable: {exe_path}")
        if args:
            self.logger.info(f"Arguments: {' '.join(args)}")
        self.logger.info("=" * 60)

        # Prepare command
        cmd = [str(exe_path)]
        if args:
            cmd.extend(args)

        # Run executable
        try:
            result = self.config.run_command(cmd, check=False)
            return result.returncode == 0
        except KeyboardInterrupt:
            self.logger.warning("Execution interrupted by user")
            return False
        except Exception as e:
            self.logger.error(f"Failed to run executable: {e}")
            return False


def parse_arguments():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(
        description="RealmEngine Cross-Platform Build Script",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python build.py                          # Default build (Debug)
  python build.py -t Release -j 8          # Release build with 8 jobs
  python build.py -c -r                    # Clean build and run editor
  python build.py -r --run-target RealmSandbox  # Build and run sandbox
  python build.py --no-editor              # Build without editor
  python build.py --tests --sanitizers     # Build with tests + sanitizers
  python build.py --physics --animation     # Enable optional modules
  python build.py --no-render              # Disable render module
  python build.py --configure              # Only configure, don't build
  python build.py --build                  # Only build, skip configure
  python build.py --clangd                 # Generate compile_commands.json for clangd (configure only, Ninja, dir build_clangd)
  python build.py --clangd --no-copy       # Same, do not copy compile_commands.json to project root
  python build.py -D CMAKE_CXX_COMPILER=clang++  # Custom CMake variable
        """,
    )

    # Build options
    parser.add_argument(
        "-t",
        "--type",
        type=str,
        default="Debug",
        choices=["Debug", "Release", "RelWithDebInfo", "MinSizeRel"],
        help="Build type (default: Debug)",
    )

    parser.add_argument(
        "-d",
        "--dir",
        type=str,
        default="build",
        help="Build directory (default: build)",
    )

    parser.add_argument(
        "-g",
        "--generator",
        type=str,
        help="CMake generator (auto-detected if not specified)",
    )

    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        help="Number of parallel build jobs (default: CPU count)",
    )

    parser.add_argument("-T", "--target", type=str, help="Build specific target only")

    # Target selection
    parser.add_argument(
        "--no-editor",
        action="store_true",
        help="Disable building the editor (REALM_BUILD_EDITOR=OFF)",
    )

    parser.add_argument(
        "--no-sandbox",
        action="store_true",
        help="Disable building the sandbox (REALM_BUILD_SANDBOX=OFF)",
    )

    parser.add_argument(
        "--tests",
        action="store_true",
        help="Enable building tests (REALM_BUILD_TESTS=ON)",
    )

    parser.add_argument(
        "--sanitizers",
        action="store_true",
        help="Enable sanitizers (REALM_ENABLE_SANITIZERS=ON)",
    )

    # Engine modules (default ON in CMake: render, audio, camera)
    parser.add_argument("--no-render", action="store_true", help="Disable render module (REALM_BUILD_RENDER=OFF)")
    parser.add_argument("--no-audio", action="store_true", help="Disable audio module (REALM_BUILD_AUDIO=OFF)")
    parser.add_argument("--no-camera", action="store_true", help="Disable camera module (REALM_BUILD_CAMERA=OFF)")
    # Engine modules (default OFF in CMake)
    parser.add_argument("--physics", action="store_true", help="Enable physics module (REALM_BUILD_PHYSICS=ON)")
    parser.add_argument("--animation", action="store_true", help="Enable animation module (REALM_BUILD_ANIMATION=ON)")
    parser.add_argument("--particles", action="store_true", help="Enable particles module (REALM_BUILD_PARTICLES=ON)")
    parser.add_argument("--network", action="store_true", help="Enable network module (REALM_BUILD_NETWORK=ON)")

    # Actions
    parser.add_argument(
        "-c",
        "--clean",
        action="store_true",
        help="Clean build directory before building",
    )

    parser.add_argument(
        "-r", "--run", action="store_true", help="Run the executable after building"
    )

    parser.add_argument(
        "--run-target",
        type=str,
        default="RealmEditor",
        choices=["RealmEditor", "RealmSandbox"],
        help="Which executable to run with -r (default: RealmEditor)",
    )

    parser.add_argument(
        "--configure", action="store_true", help="Only configure CMake, don't build"
    )

    parser.add_argument("--build", action="store_true", help="Only build, skip CMake configuration")

    parser.add_argument(
        "--clangd",
        action="store_true",
        help="Generate compile_commands.json for clangd (configure only; uses build_clangd dir and Ninja by default)",
    )
    parser.add_argument(
        "--no-copy",
        action="store_true",
        help="With --clangd: do not copy compile_commands.json to project root",
    )

    # Other options
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")

    parser.add_argument(
        "-D",
        action="append",
        dest="cmake_defines",
        metavar="VAR=VALUE",
        help="Pass -D options to CMake (can be used multiple times)",
    )

    # Parse known args to allow passing arguments to executable
    args, unknown = parser.parse_known_args()
    args.exe_args = unknown if unknown and unknown[0] == "--" else []
    if args.exe_args and args.exe_args[0] == "--":
        args.exe_args = args.exe_args[1:]

    return args


def _run_clangd_mode(args, config, logger) -> int:
    """Generate compile_commands.json for clangd (configure only)."""
    tools_ok, missing = config.check_build_tools()
    if not tools_ok:
        logger.error(f"Missing required build tools: {', '.join(missing)}")
        return 1

    build_dir = config.project_root / args.dir
    generator = args.generator or ("Ninja" if config.find_program("ninja") else config.get_cmake_generator())

    if generator != "Ninja" and ("Visual Studio" in generator or generator == "Xcode"):
        logger.warning(
            f"{generator} does not generate compile_commands.json. "
            "Use Ninja: install it and re-run without -g, or pass -g Ninja."
        )

    logger.info("Generating compile_commands.json for clangd...")
    logger.info(f"  Build directory: {build_dir}")
    logger.info(f"  Generator: {generator}")

    cmake_args = []
    if args.no_editor:
        cmake_args.append("-DREALM_BUILD_EDITOR=OFF")
    if args.no_sandbox:
        cmake_args.append("-DREALM_BUILD_SANDBOX=OFF")
    if args.no_render:
        cmake_args.append("-DREALM_BUILD_RENDER=OFF")
    if args.no_audio:
        cmake_args.append("-DREALM_BUILD_AUDIO=OFF")
    if args.no_camera:
        cmake_args.append("-DREALM_BUILD_CAMERA=OFF")
    if args.physics:
        cmake_args.append("-DREALM_BUILD_PHYSICS=ON")
    if args.animation:
        cmake_args.append("-DREALM_BUILD_ANIMATION=ON")
    if args.particles:
        cmake_args.append("-DREALM_BUILD_PARTICLES=ON")
    if args.network:
        cmake_args.append("-DREALM_BUILD_NETWORK=ON")
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
            logger.info("Use Ninja generator (install Ninja, then re-run with -g Ninja)")
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


def main():
    """Main entry point"""
    args = parse_arguments()

    config = get_build_config()
    logger = config.logger

    if not config.check_cmake():
        return 1

    if args.clangd:
        if args.dir == "build":
            args.dir = "build_clangd"
        return _run_clangd_mode(args, config, logger)

    # Check build tools
    tools_ok, missing = config.check_build_tools()
    if not tools_ok:
        logger.error(f"Missing required build tools: {', '.join(missing)}")
        return 1

    # Setup build parameters
    build_type = BuildType(args.type)
    build_dir = config.project_root / args.dir
    generator = args.generator or config.get_cmake_generator()
    jobs = args.jobs or config.get_cpu_count()

    # Print configuration
    config.print_config(build_type, generator, jobs)

    # Create builder
    builder = Builder(config)

    # Clean if requested
    if args.clean and not builder.clean(build_dir):
        return 1

    # Prepare CMake defines
    cmake_args = []
    if args.no_editor:
        cmake_args.append("-DREALM_BUILD_EDITOR=OFF")
    if args.no_sandbox:
        cmake_args.append("-DREALM_BUILD_SANDBOX=OFF")
    if args.tests:
        cmake_args.append("-DREALM_BUILD_TESTS=ON")
    if args.sanitizers:
        cmake_args.append("-DREALM_ENABLE_SANITIZERS=ON")
    if args.no_render:
        cmake_args.append("-DREALM_BUILD_RENDER=OFF")
    if args.no_audio:
        cmake_args.append("-DREALM_BUILD_AUDIO=OFF")
    if args.no_camera:
        cmake_args.append("-DREALM_BUILD_CAMERA=OFF")
    if args.physics:
        cmake_args.append("-DREALM_BUILD_PHYSICS=ON")
    if args.animation:
        cmake_args.append("-DREALM_BUILD_ANIMATION=ON")
    if args.particles:
        cmake_args.append("-DREALM_BUILD_PARTICLES=ON")
    if args.network:
        cmake_args.append("-DREALM_BUILD_NETWORK=ON")
    if args.cmake_defines:
        for define in args.cmake_defines:
            cmake_args.append(f"-D{define}")

    # Configure
    if not args.build:
        if not builder.configure(
            build_dir,
            build_type,
            generator,
            extra_args=cmake_args,
            verbose=args.verbose,
        ):
            return 1

        if args.configure:
            logger.success("Configuration complete")
            return 0

    # Build
    if not builder.build(
        build_dir, build_type, generator, jobs, target=args.target, verbose=args.verbose
    ):
        return 1

    # Run if requested
    if args.run and not builder.run_executable(
        build_type, target=args.run_target, args=args.exe_args
    ):
        return 1

    logger.success("All done!")
    return 0


if __name__ == "__main__":
    sys.exit(main())
