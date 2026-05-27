#include "filesystem.h"
#include <iostream>
#include <sys/mount.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

namespace Filesystem {

bool setup(const std::string& rootfs_path) {
    std::cout << "[+] Setting up isolated filesystem namespace...\n";

    // 1. Set mount propagation to MS_PRIVATE recursively on the root filesystem.
    // This ensures that any mounts or unmounts performed inside this container
    // mount namespace do not propagate back to the host filesystem.
    // Without this, the host's mounts could be affected or our private mounts
    // could leak out.
    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) != 0) {
        std::cerr << "[-] Filesystem error: failed to make mounts private: " << std::strerror(errno) << "\n";
        return false;
    }

    // 2. Perform chroot into the rootfs directory
    std::cout << "[+] Entering chroot jail: " << rootfs_path << "\n";
    if (chroot(rootfs_path.c_str()) != 0) {
        std::cerr << "[-] Filesystem error: failed to chroot into " << rootfs_path 
                  << " (" << std::strerror(errno) << ")\n";
        return false;
    }

    // 3. Change directory to the new root.
    // Essential because chroot alone does not change the current working directory.
    if (chdir("/") != 0) {
        std::cerr << "[-] Filesystem error: failed to change directory to / (" << std::strerror(errno) << ")\n";
        return false;
    }

    // 4. Mount the isolated procfs filesystem under /proc inside the chroot jail.
    // Since this process runs in a new PID namespace (created by clone), mounting
    // a fresh procfs ensures that process listings (like `ps`) reflect only the
    // processes running inside this container namespace, fully hiding the host's processes.
    std::cout << "[+] Mounting /proc filesystem...\n";
    if (mount("proc", "/proc", "proc", 0, NULL) != 0) {
        std::cerr << "[-] Filesystem error: failed to mount procfs at /proc: " << std::strerror(errno) << "\n";
        return false;
    }

    return true;
}

} // namespace Filesystem
