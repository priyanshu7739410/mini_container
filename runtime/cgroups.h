#ifndef CGROUPS_H
#define CGROUPS_H

#include <string>
#include <sys/types.h>

namespace Cgroups {
    // Sets up cgroups limits for the given child process PID
    bool setup(pid_t pid, const std::string& memory_limit, const std::string& cpu_limit);
    
    // Cleans up cgroups directory
    bool cleanup();
}

#endif // CGROUPS_H
