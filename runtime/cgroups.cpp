#include "cgroups.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

namespace Cgroups {

static std::string get_cgroup_path(pid_t pid) {
    return "/sys/fs/cgroup/mycontainer-" + std::to_string(pid);
}

// Helper to write a value to a specific cgroup file
static bool write_file(const std::string& path, const std::string& value) {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "[-] Cgroup error: failed to open " << path << " (" << std::strerror(errno) << ")\n";
        return false;
    }
    file << value;
    if (file.fail()) {
        std::cerr << "[-] Cgroup error: failed to write to " << path << "\n";
        return false;
    }
    return true;
}

bool setup(pid_t pid, const std::string& memory_limit, const std::string& cpu_limit) {
    std::cout << "[+] Configuring Cgroups v2 for PID " << pid << "...\n";

    std::string cgroup_path = get_cgroup_path(pid);

    // 1. Ensure memory and cpu controllers are enabled in parent cgroup's subtree_control
    // This is optional since they are usually enabled, but is a robust systems practice
    write_file("/sys/fs/cgroup/cgroup.subtree_control", "+memory +cpu");

    // 2. Create the cgroup subdirectory if it doesn't exist
    if (mkdir(cgroup_path.c_str(), 0755) && errno != EEXIST) {
        std::cerr << "[-] Cgroup error: failed to create cgroup directory: " << std::strerror(errno) << "\n";
        return false;
    }

    // 3. Write PID to cgroup.procs to assign the process and all its future children to this cgroup
    if (!write_file(cgroup_path + "/cgroup.procs", std::to_string(pid))) {
        std::cerr << "[-] Cgroup error: failed to assign process to cgroup\n";
        return false;
    }

    // 4. Configure Memory Limit
    if (!memory_limit.empty()) {
        std::cout << "[+] Cgroup memory limit set to: " << memory_limit << "\n";
        if (!write_file(cgroup_path + "/memory.max", memory_limit)) {
            std::cerr << "[-] Cgroup error: failed to set memory.max to " << memory_limit << "\n";
        }
    }

    // 5. Configure CPU Limit (Format: "quota period", e.g., "10000 100000" for 10% usage of 1 core)
    if (!cpu_limit.empty()) {
        std::cout << "[+] Cgroup CPU limit set to: " << cpu_limit << "\n";
        if (!write_file(cgroup_path + "/cpu.max", cpu_limit)) {
            std::cerr << "[-] Cgroup error: failed to set cpu.max to " << cpu_limit << "\n";
        }
    }

    return true;
}

bool cleanup(pid_t pid) {
    std::string cgroup_path = get_cgroup_path(pid);
    std::cout << "[+] Cleaning up cgroup directory " << cgroup_path << "...\n";
    // Remove the directory
    if (rmdir(cgroup_path.c_str()) != 0) {
        if (errno != ENOENT) {
            std::cerr << "[-] Cgroup error: failed to remove cgroup directory " << cgroup_path 
                      << " (" << std::strerror(errno) << ")\n";
            return false;
        }
    }
    std::cout << "[+] Cgroups cleaned up successfully.\n";
    return true;
}

} // namespace Cgroups
