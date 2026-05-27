#include "container.h"
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

void print_usage() {
    std::cout << "Usage: sudo ./mycontainer run [options] <command> [args...]\n\n"
              << "Options:\n"
              << "  --memory <limit>    Memory limit (e.g. 50M, 100M, or raw bytes)\n"
              << "  --cpu <limit>       CPU limit in format \"quota period\" (e.g. \"10000 100000\" for 10% CPU)\n"
              << "  --hostname <name>   Set custom hostname inside UTS namespace (default: mini-container)\n"
              << "  --rootfs <path>     Path to the root directory (default: ./rootfs)\n\n"
              << "Examples:\n"
              << "  sudo ./mycontainer run /bin/sh\n"
              << "  sudo ./mycontainer run --memory 50M --hostname sandbox /bin/ls -la /\n";
}

int main(int argc, char* argv[]) {
    // 1. Verify superuser privileges
    if (geteuid() != 0) {
        std::cerr << "[-] Error: this runtime requires root privileges (sudo).\n";
        return 1;
    }

    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];
    if (command != "run") {
        std::cerr << "[-] Error: unknown command \"" << command << "\"\n";
        print_usage();
        return 1;
    }

    // Default configuration values
    ContainerConfig config;
    config.rootfs_path = "./rootfs";
    config.hostname = "mini-container";

    int arg_idx = 2;
    while (arg_idx < argc) {
        std::string current_arg = argv[arg_idx];

        if (current_arg == "--memory") {
            if (arg_idx + 1 >= argc) {
                std::cerr << "[-] Error: --memory requires a limit value\n";
                return 1;
            }
            config.memory_limit = argv[++arg_idx];
        } else if (current_arg == "--cpu") {
            if (arg_idx + 1 >= argc) {
                std::cerr << "[-] Error: --cpu requires a limit value\n";
                return 1;
            }
            config.cpu_limit = argv[++arg_idx];
        } else if (current_arg == "--hostname") {
            if (arg_idx + 1 >= argc) {
                std::cerr << "[-] Error: --hostname requires a value\n";
                return 1;
            }
            config.hostname = argv[++arg_idx];
        } else if (current_arg == "--rootfs") {
            if (arg_idx + 1 >= argc) {
                std::cerr << "[-] Error: --rootfs requires a path\n";
                return 1;
            }
            config.rootfs_path = argv[++arg_idx];
        } else {
            // Reached the command to execute inside the container
            break;
        }
        arg_idx++;
    }

    // Remaining arguments constitute the user command and its arguments
    if (arg_idx >= argc) {
        std::cerr << "[-] Error: no command provided to run inside the container\n";
        print_usage();
        return 1;
    }

    for (int i = arg_idx; i < argc; ++i) {
        config.args.push_back(argv[i]);
    }

    // Create and execute the container orchestration
    Container container(config);
    if (!container.start()) {
        std::cerr << "[-] Error: container failed to run\n";
        return 1;
    }

    return 0;
}
