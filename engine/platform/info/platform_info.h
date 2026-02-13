#pragma once

#include <string>

namespace RealmEngine
{
    /// Static utility class for querying platform and hardware information.
    /// GPU and display queries require an active OpenGL context / GLFW initialization.
    class PlatformInfo
    {
    public:
        // --- Operating System ---

        static std::string getOSName();
        static std::string getOSVersion();

        // --- CPU ---

        static std::string getCPUName();
        static int         getLogicalCoreCount();

        // --- Memory (megabytes) ---

        static int getTotalMemoryMB();
        static int getAvailableMemoryMB();

        // --- GPU (requires active OpenGL context) ---

        static std::string getGPUVendor();
        static std::string getGPURenderer();
        static std::string getOpenGLVersion();
        static std::string getGLSLVersion();

        // --- Display (requires GLFW initialization) ---

        static int         getMonitorCount();
        static void        getPrimaryMonitorResolution(int& width, int& height);
        static void        getPrimaryMonitorPhysicalSize(int& width_mm, int& height_mm);
        static std::string getPrimaryMonitorName();

        /// Print a comprehensive summary of all platform information to the log.
        static void logPlatformInfo();
    };
} // namespace RealmEngine
