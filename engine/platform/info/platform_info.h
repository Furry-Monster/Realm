#pragma once

#include <cstddef>
#include <string>

namespace RealmEngine
{
    class RHIDevice;

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
        static int    getTotalMemoryMB();
        static int    getAvailableMemoryMB();
        static size_t getProcessRSSKB();

        // GPU (via RHI; requires valid device with active context)
        static std::string getGPUVendor(RHIDevice& device);
        static std::string getGPURenderer(RHIDevice& device);
        static std::string getAPIVersion(RHIDevice& device);
        static std::string getShadingLanguageVersion(RHIDevice& device);

        // display (requires GLFW)
        static int         getMonitorCount();
        static void        getPrimaryMonitorResolution(int& width, int& height);
        static void        getPrimaryMonitorPhysicalSize(int& width_mm, int& height_mm);
        static std::string getPrimaryMonitorName();

        static void logPlatformInfo(RHIDevice& device);
    };
} // namespace RealmEngine
