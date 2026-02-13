#!/usr/bin/env python3
"""
RealmEngine Build Configuration
Cross-platform build configuration and utilities
"""

import os
import platform
import shutil
import subprocess
import sys
from enum import Enum
from pathlib import Path
from typing import Optional, List, Tuple


class BuildType(Enum):
    """Build configuration types"""
    DEBUG = "Debug"
    RELEASE = "Release"
    RELWITHDEBINFO = "RelWithDebInfo"
    MINSIZEREL = "MinSizeRel"


class Platform(Enum):
    """Supported platforms"""
    WINDOWS = "Windows"
    LINUX = "Linux"
    MACOS = "Darwin"
    UNKNOWN = "Unknown"


class Colors:
    """Terminal color codes"""
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    MAGENTA = '\033[0;35m'
    CYAN = '\033[0;36m'
    NC = '\033[0m'  # No Color
    
    @staticmethod
    def is_terminal_supports_color():
        """Check if terminal supports colors"""
        if platform.system() == "Windows":
            # Windows 10+ supports ANSI colors
            return True
        return sys.stdout.isatty()


class Logger:
    """Logging utilities with colored output"""
    
    def __init__(self, use_colors: bool = True):
        self.use_colors = use_colors and Colors.is_terminal_supports_color()
    
    def _colorize(self, msg: str, color: str) -> str:
        """Add color to message if colors are enabled"""
        if self.use_colors:
            return f"{color}{msg}{Colors.NC}"
        return msg
    
    def info(self, msg: str):
        """Print info message"""
        print(f"{self._colorize('[INFO]', Colors.BLUE)} {msg}")
    
    def success(self, msg: str):
        """Print success message"""
        print(f"{self._colorize('[SUCCESS]', Colors.GREEN)} {msg}")
    
    def warning(self, msg: str):
        """Print warning message"""
        print(f"{self._colorize('[WARNING]', Colors.YELLOW)} {msg}")
    
    def error(self, msg: str):
        """Print error message"""
        print(f"{self._colorize('[ERROR]', Colors.RED)} {msg}")
    
    def debug(self, msg: str):
        """Print debug message"""
        print(f"{self._colorize('[DEBUG]', Colors.MAGENTA)} {msg}")


class BuildConfig:
    """Build configuration management"""
    
    def __init__(self):
        self.platform = self._detect_platform()
        self.logger = Logger()
        self.project_root = self._find_project_root()
        self.build_dir = self.project_root / "build"
        self.bin_dir = self.project_root / "bin"
        self.src_dir = self.project_root / "src"
        self.scripts_dir = self.project_root / "scripts"
    
    @staticmethod
    def _detect_platform() -> Platform:
        """Detect current platform"""
        system = platform.system()
        try:
            return Platform(system)
        except ValueError:
            return Platform.UNKNOWN
    
    @staticmethod
    def _find_project_root() -> Path:
        """Find project root directory"""
        # Start from script location
        current = Path(__file__).resolve().parent.parent
        
        # Look for CMakeLists.txt
        if (current / "CMakeLists.txt").exists():
            return current
        
        # Fallback to current working directory
        return Path.cwd()
    
    def get_cpu_count(self) -> int:
        """Get number of CPU cores"""
        try:
            return os.cpu_count() or 4
        except:
            return 4
    
    def find_program(self, name: str) -> Optional[str]:
        """Find a program in PATH"""
        return shutil.which(name)
    
    def get_cmake_generator(self) -> str:
        """Get appropriate CMake generator for platform"""
        if self.platform == Platform.WINDOWS:
            # Check for Visual Studio versions
            vs_generators = [
                "Visual Studio 17 2022",
                "Visual Studio 16 2019",
                "Visual Studio 15 2017",
            ]
            # Use default VS generator
            return vs_generators[0] if self.find_program("cmake") else "Ninja"
        
        elif self.platform == Platform.LINUX:
            if self.find_program("ninja"):
                return "Ninja"
            return "Unix Makefiles"
        
        elif self.platform == Platform.MACOS:
            if self.find_program("ninja"):
                return "Ninja"
            return "Unix Makefiles"
        
        return "Unix Makefiles"
    
    def get_build_command(self, generator: str) -> List[str]:
        """Get build command for given generator"""
        if generator == "Ninja":
            return ["ninja"]
        elif "Visual Studio" in generator:
            return ["cmake", "--build", "."]
        else:
            return ["make"]
    
    def get_executable_name(self) -> str:
        """Get executable name for platform"""
        if self.platform == Platform.WINDOWS:
            return "RealmEngine.exe"
        return "RealmEngine"
    
    def get_executable_path(self, build_type: BuildType) -> Path:
        """Get path to built executable"""
        exe_name = self.get_executable_name()
        
        if self.platform == Platform.WINDOWS:
            # Visual Studio puts executables in build_type subdirectory
            return self.bin_dir / exe_name
        
        return self.bin_dir / exe_name
    
    def get_encoding(self) -> str:
        """Get system encoding"""
        try:
            import locale
            return locale.getpreferredencoding(False) or 'utf-8'
        except:
            return 'utf-8'
    
    def run_command(
        self,
        cmd: List[str],
        cwd: Optional[Path] = None,
        check: bool = True,
        capture_output: bool = False,
        shell: bool = False
    ) -> subprocess.CompletedProcess:
        """Run a command with proper error handling"""
        try:
            result = subprocess.run(
                cmd,
                cwd=cwd,
                check=check,
                capture_output=capture_output,
                text=True,
                encoding=self.get_encoding(),
                errors='replace',
                shell=shell
            )
            return result
        except subprocess.CalledProcessError as e:
            self.logger.error(f"Command failed: {' '.join(cmd)}")
            if e.stdout:
                print(e.stdout)
            if e.stderr:
                print(e.stderr)
            if check:
                sys.exit(1)
            return e
        except FileNotFoundError:
            self.logger.error(f"Command not found: {cmd[0]}")
            if check:
                sys.exit(1)
            raise
    
    def check_cmake(self) -> bool:
        """Check if CMake is available"""
        if not self.find_program("cmake"):
            self.logger.error("CMake not found. Please install CMake.")
            self.logger.info("Visit: https://cmake.org/download/")
            return False
        
        # Check CMake version
        try:
            result = self.run_command(
                ["cmake", "--version"],
                capture_output=True,
                check=False
            )
            if result.returncode == 0:
                version_line = result.stdout.splitlines()[0]
                self.logger.info(f"Found {version_line}")
            return True
        except:
            return False
    
    def check_build_tools(self) -> Tuple[bool, List[str]]:
        """Check if required build tools are available"""
        missing = []
        
        if not self.find_program("cmake"):
            missing.append("cmake")
        
        if self.platform == Platform.WINDOWS:
            # Check for MSVC or MinGW
            if not self.find_program("cl") and not self.find_program("g++"):
                missing.append("MSVC or MinGW")
        else:
            # Check for GCC or Clang
            if not self.find_program("g++") and not self.find_program("clang++"):
                missing.append("g++ or clang++")
        
        return len(missing) == 0, missing
    
    def print_config(self, build_type: BuildType, generator: str, jobs: int):
        """Print build configuration"""
        self.logger.info("=" * 60)
        self.logger.info("RealmEngine Build Configuration")
        self.logger.info("=" * 60)
        self.logger.info(f"Platform:        {self.platform.value}")
        self.logger.info(f"Build Type:      {build_type.value}")
        self.logger.info(f"Generator:       {generator}")
        self.logger.info(f"Parallel Jobs:   {jobs}")
        self.logger.info(f"Project Root:    {self.project_root}")
        self.logger.info(f"Build Directory: {self.build_dir}")
        self.logger.info(f"Output Directory: {self.bin_dir}")
        self.logger.info("=" * 60)


# Singleton instance
_config_instance: Optional[BuildConfig] = None


def get_build_config() -> BuildConfig:
    """Get global build configuration instance"""
    global _config_instance
    if _config_instance is None:
        _config_instance = BuildConfig()
    return _config_instance
