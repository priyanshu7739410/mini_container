#ifndef CONTAINER_H
#define CONTAINER_H

#include <string>
#include <vector>

struct ContainerConfig {
    std::string rootfs_path;
    std::string hostname;
    std::vector<std::string> args;
    std::string memory_limit;
    std::string cpu_limit;
};

class Container {
public:
    explicit Container(ContainerConfig config);
    ~Container();

    // Starts the container orchestration
    bool start();

private:
    ContainerConfig config_;
    int sync_pipe_[2]{-1, -1}; // Communication pipe for parent-child synchronization
    
    // The entry function for the cloned child process
    static int child_entry(void* arg);
    
    // Child setup details
    int run_child();
};

#endif // CONTAINER_H
