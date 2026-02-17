#include "platform/info/platform_info.h"

#include "core/log/log_macros.h"
#include "rhi/rhi_device.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstdio>
#include <string>
#include <thread>

#ifdef __linux__
#  include <sys/sysinfo.h>
#  include <sys/utsname.h>
#  include <fstream>
#elif defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <psapi.h>
#elif defined(__APPLE__)
#  include <mach/mach.h>
#  include <sys/sysctl.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

namespace RealmEngine
{
    // OS

    std::string PlatformInfo::getOSName()
    {
#ifdef __linux__
        return "Linux";
#elif defined(_WIN32)
        return "Windows";
#elif defined(__APPLE__)
        return "macOS";
#else
        return "Unknown";
#endif
    }

    std::string PlatformInfo::getOSVersion()
    {
#ifdef __linux__
        struct utsname info;
        if (uname(&info) == 0)
            return std::string(info.release);
        return "Unknown";
#elif defined(_WIN32)
        HKEY  h_key;
        DWORD size;
        char  buffer[256];

        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, R"(SOFTWARE\Microsoft\Windows NT\CurrentVersion)", 0, KEY_READ, &h_key) ==
            ERROR_SUCCESS)
        {
            std::string version;

            size = sizeof(buffer);
            if (RegQueryValueExA(h_key, "ProductName", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &size) ==
                ERROR_SUCCESS)
            {
                version = buffer;
            }

            size = sizeof(buffer);
            if (RegQueryValueExA(
                    h_key, "CurrentBuildNumber", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &size) ==
                ERROR_SUCCESS)
            {
                version += " (Build " + std::string(buffer) + ")";
            }

            RegCloseKey(h_key);
            return version.empty() ? std::string("Unknown") : version;
        }
        return "Unknown";
#elif defined(__APPLE__)
        char   str[256];
        size_t len = sizeof(str);
        if (sysctlbyname("kern.osrelease", str, &len, nullptr, 0) == 0)
            return std::string(str);
        return "Unknown";
#else
        return "Unknown";
#endif
    }

    // CPU

    std::string PlatformInfo::getCPUName()
    {
#ifdef __linux__
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string   line;
        while (std::getline(cpuinfo, line))
        {
            if (line.find("model name") != std::string::npos)
            {
                auto pos = line.find(':');
                if (pos != std::string::npos)
                {
                    std::string name  = line.substr(pos + 1);
                    auto        start = name.find_first_not_of(" \t");
                    return (start != std::string::npos) ? name.substr(start) : name;
                }
            }
        }
        return "Unknown";
#elif defined(_WIN32)
        HKEY  h_key;
        char  cpu_name[256];
        DWORD buffer_size = sizeof(cpu_name);

        if (RegOpenKeyExA(
                HKEY_LOCAL_MACHINE, R"(HARDWARE\DESCRIPTION\System\CentralProcessor\0)", 0, KEY_READ, &h_key) ==
            ERROR_SUCCESS)
        {
            if (RegQueryValueExA(
                    h_key, "ProcessorNameString", nullptr, nullptr, reinterpret_cast<LPBYTE>(cpu_name), &buffer_size) ==
                ERROR_SUCCESS)
            {
                RegCloseKey(h_key);
                return std::string(cpu_name);
            }
            RegCloseKey(h_key);
        }
        return "Unknown";
#elif defined(__APPLE__)
        char   str[256];
        size_t len = sizeof(str);
        // works on both Intel ("Intel Core i7-...") and Apple Silicon ("Apple M1/M2/...")
        if (sysctlbyname("machdep.cpu.brand_string", str, &len, nullptr, 0) == 0)
            return std::string(str);
        // fallback: returns model identifier like "MacBookPro18,1"
        len = sizeof(str);
        if (sysctlbyname("hw.model", str, &len, nullptr, 0) == 0)
            return std::string(str);
        return "Unknown";
#else
        return "Unknown";
#endif
    }

    int PlatformInfo::getLogicalCoreCount()
    {
        int count = static_cast<int>(std::thread::hardware_concurrency());
        return (count > 0) ? count : 1;
    }

    // memory

    int PlatformInfo::getTotalMemoryMB()
    {
#ifdef __linux__
        struct sysinfo info;
        if (sysinfo(&info) == 0)
        {
            // cast to 64-bit before multiplication to prevent overflow on 32-bit systems
            return static_cast<int>(static_cast<uint64_t>(info.totalram) * info.mem_unit / (1024 * 1024));
        }
        return 0;
#elif defined(_WIN32)
        MEMORYSTATUSEX mem_info {};
        mem_info.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&mem_info))
            return static_cast<int>(mem_info.ullTotalPhys / (1024 * 1024));
        return 0;
#elif defined(__APPLE__)
        int64_t mem;
        size_t  len = sizeof(mem);
        if (sysctlbyname("hw.memsize", &mem, &len, nullptr, 0) == 0)
            return static_cast<int>(mem / (1024 * 1024));
        return 0;
#else
        return 0;
#endif
    }

    size_t PlatformInfo::getProcessRSSKB()
    {
#ifdef __linux__
        std::ifstream status("/proc/self/status");
        std::string   line;
        while (std::getline(status, line))
        {
            if (line.compare(0, 6, "VmRSS:") == 0)
            {
                size_t pos = line.find_first_not_of(" \t", 6);
                if (pos != std::string::npos)
                    return static_cast<size_t>(std::stoull(line.substr(pos)));
            }
        }
        return 0;
#elif defined(_WIN32)
        PROCESS_MEMORY_COUNTERS pmc {};
        pmc.cb = sizeof(pmc);
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
            return static_cast<size_t>(pmc.WorkingSetSize / 1024);
        return 0;
#elif defined(__APPLE__)
        struct task_basic_info info {};
        mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &count) == KERN_SUCCESS)
            return static_cast<size_t>(info.resident_size / 1024);
        return 0;
#else
        return 0;
#endif
    }

    int PlatformInfo::getAvailableMemoryMB()
    {
#ifdef __linux__
        // Prefer /proc/meminfo MemAvailable (includes cached/reclaimable)
        {
            std::ifstream meminfo("/proc/meminfo");
            std::string   line;
            while (std::getline(meminfo, line))
            {
                if (line.rfind("MemAvailable:", 0) == 0)
                {
                    const char* p = line.c_str() + 13; // skip "MemAvailable:"
                    while (*p == ' ')
                        ++p;
                    char*    end = nullptr;
                    uint64_t kb  = std::strtoull(p, &end, 10);
                    if (end != p)
                        return static_cast<int>(kb / 1024);
                }
            }
        }
        // Fallback to sysinfo (less accurate)
        struct sysinfo info;
        if (sysinfo(&info) == 0)
        {
            uint64_t available = (static_cast<uint64_t>(info.freeram) + info.bufferram) * info.mem_unit;
            return static_cast<int>(available / (1024 * 1024));
        }
        return 0;
#elif defined(_WIN32)
        MEMORYSTATUSEX mem_info {};
        mem_info.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&mem_info))
            return static_cast<int>(mem_info.ullAvailPhys / (1024 * 1024));
        return 0;
#elif defined(__APPLE__)
        mach_port_t            host = mach_host_self();
        vm_statistics64_data_t vm_stats;
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;

        if (host_statistics64(host, HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&vm_stats), &count) == KERN_SUCCESS)
        {
            // page counts are 32-bit natural_t; widen before multiply to prevent overflow
            int64_t page_size       = sysconf(_SC_PAGESIZE);
            int64_t available_pages = static_cast<int64_t>(vm_stats.free_count) + vm_stats.inactive_count;
            return static_cast<int>(available_pages * page_size / (1024 * 1024));
        }
        return 0;
#else
        return 0;
#endif
    }

    // GPU (via RHI)

    std::string PlatformInfo::getGPUVendor(RHIDevice& device) { return device.getGPUVendor(); }

    std::string PlatformInfo::getGPURenderer(RHIDevice& device) { return device.getGPURenderer(); }

    std::string PlatformInfo::getAPIVersion(RHIDevice& device) { return device.getAPIVersion(); }

    std::string PlatformInfo::getShadingLanguageVersion(RHIDevice& device)
    {
        return device.getShadingLanguageVersion();
    }

    // display (requires GLFW)

    int PlatformInfo::getMonitorCount()
    {
        int count = 0;
        glfwGetMonitors(&count);
        return count;
    }

    void PlatformInfo::getPrimaryMonitorResolution(int& width, int& height)
    {
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        if (primary)
        {
            const GLFWvidmode* mode = glfwGetVideoMode(primary);
            if (mode)
            {
                width  = mode->width;
                height = mode->height;
                return;
            }
        }
        width  = 0;
        height = 0;
    }

    void PlatformInfo::getPrimaryMonitorPhysicalSize(int& width_mm, int& height_mm)
    {
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        if (primary)
        {
            glfwGetMonitorPhysicalSize(primary, &width_mm, &height_mm);
            return;
        }
        width_mm  = 0;
        height_mm = 0;
    }

    std::string PlatformInfo::getPrimaryMonitorName()
    {
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        if (primary)
        {
            const char* name = glfwGetMonitorName(primary);
            if (name)
                return std::string(name);
        }
        return "Unknown";
    }

    void PlatformInfo::logPlatformInfo(RHIDevice& device)
    {
        RE_LOG_INFO("=== Platform Information ===");
        RE_LOG_INFO("OS: " + getOSName() + " " + getOSVersion());
        RE_LOG_INFO("CPU: " + getCPUName());
        RE_LOG_INFO("Logical cores: " + std::to_string(getLogicalCoreCount()));
        RE_LOG_INFO("Total memory: " + std::to_string(getTotalMemoryMB()) + " MB");
        RE_LOG_INFO("Available memory: " + std::to_string(getAvailableMemoryMB()) + " MB");
        RE_LOG_INFO("GPU vendor: " + getGPUVendor(device));
        RE_LOG_INFO("GPU renderer: " + getGPURenderer(device));
        RE_LOG_INFO("API version: " + getAPIVersion(device));
        RE_LOG_INFO("Shading language version: " + getShadingLanguageVersion(device));

        int mon_w = 0;
        int mon_h = 0;
        getPrimaryMonitorResolution(mon_w, mon_h);
        RE_LOG_INFO("Monitor count: " + std::to_string(getMonitorCount()));
        RE_LOG_INFO("Primary monitor: " + getPrimaryMonitorName() + " (" + std::to_string(mon_w) + "x" +
                    std::to_string(mon_h) + ")");

        int phys_w = 0;
        int phys_h = 0;
        getPrimaryMonitorPhysicalSize(phys_w, phys_h);
        if (phys_w > 0 && phys_h > 0)
        {
            // DPI = pixels / (mm / 25.4)
            float dpi_x = static_cast<float>(mon_w) / (static_cast<float>(phys_w) / 25.4f);
            float dpi_y = static_cast<float>(mon_h) / (static_cast<float>(phys_h) / 25.4f);
            RE_LOG_INFO("Estimated DPI: " + std::to_string(static_cast<int>(dpi_x)) + "x" +
                        std::to_string(static_cast<int>(dpi_y)));
        }

        RE_LOG_INFO("============================");
    }

} // namespace RealmEngine
