#pragma once

#include <string>

namespace RealmEngine
{
    // Queries for OS, CPU, GPU, memory, and display information.
    // GPU queries require an active GL context; display queries require GLFW init.
    class PlatformInfo
    {
    public:
        // OS
        static std::string getOSName();
        static std::string getOSVersion();

        // CPU
        static std::string getCPUName();
        static int         getLogicalCoreCount();

        // memory (megabytes)
        static int getTotalMemoryMB();
        static int getAvailableMemoryMB();

        // GPU (requires GL context)
        static std::string getGPUVendor();
        static std::string getGPURenderer();
        static std::string getOpenGLVersion();
        static std::string getGLSLVersion();

        // display (requires GLFW)
        static int         getMonitorCount();
        static void        getPrimaryMonitorResolution(int& width, int& height);
        static void        getPrimaryMonitorPhysicalSize(int& width_mm, int& height_mm);
        static std::string getPrimaryMonitorName();

        // prints all platform info to the log
        static void logPlatformInfo();
    };
} // namespace RealmEngine
