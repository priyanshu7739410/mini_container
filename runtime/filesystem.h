#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>

namespace Filesystem {
    // Sets up the isolated rootfs environment (propagation, chroot, proc mount)
    bool setup(const std::string& rootfs_path);
}

#endif // FILESYSTEM_H
