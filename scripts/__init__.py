"""
RealmEngine Build Scripts Package
Cross-platform build system for Windows, Linux, and macOS
"""

__version__ = "1.0.0"
__author__ = "RealmEngine Team"

from .build_config import (
    BuildConfig,
    BuildType,
    Colors,
    Logger,
    Platform,
    get_build_config,
)


__all__ = [
    "BuildConfig",
    "BuildType",
    "Colors",
    "Logger",
    "Platform",
    "get_build_config",
]
