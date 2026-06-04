#include "container.h"
#include "filesystem.h"
#include "cgroups.h"
#include "process.h"

#include <iostream>
#include <sched.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <memory>

// Define standard stack size for cloned process (1 Megabyte)
const int STACK_SIZE = 1024 * 1024;

Container::Container(ContainerConfig config) : config_(std::move(config)) {}

Container::~Container() = default;

// Entry point function for clone. Since clone expects a plain C-function pointer,
// we provide a static method and pass 'this' object instance via arg.
int Container::child_entry(void* arg) {
    auto* container = static_cast<Container*>(arg);
    return container->run_child();
}

int Container::run_child() {
    // 1. Synchronize with parent process.
    // Close unused write end of the sync pipe in the child context.
    close(sync_pipe_[1]);

    std::cout << "[+] Child: Waiting for parent to configure environment...\n";
    
    // Read operation blocks until the parent writes a byte or closes the pipe.
    char sync_signal;
    ssize_t bytes_read = read(sync_pipe_[0], &sync_signal, 1);
    close(sync_pipe_[0]);

    if (bytes_read <= 0) {
        std::cerr << "[-] Namespace error: synchronization pipe failed during read\n";
        return 1;
    }

    std::cout << "[+] Inside container child process (PID: " << getpid() << ")\n";

    // 2. Hostname Isolation (UTS Namespace)
    if (!config_.hostname.empty()) {
        std::cout << "[+] Configuring hostname inside UTS namespace: " << config_.hostname << "\n";
        if (sethostname(config_.hostname.c_str(), config_.hostname.size()) != 0) {
            std::cerr << "[-] Namespace error: failed to set hostname (" << std::strerror(errno) << ")\n";
            return 1;
        }
    }

    // 3. Filesystem & Mount namespace setup (Mount propagation, Chroot, Procfs mounting)
    if (!Filesystem::setup(config_.rootfs_path)) {
        std::cerr << "[-] Container error: filesystem setup failed\n";
        return 1;
    }

    // 4. Command Execution (Execvp)
    if (!Process::execute(config_.args)) {
        std::cerr << "[-] Container error: failed to execute command\n";
        return 1;
    }

    return 0;
}

bool Container::start() {
    std::cout << "[+] Orchestrating container bootstrap...\n";

    // 1. Allocate stack memory for clone
    // Since stack grows downwards on x86, we pass stack + STACK_SIZE to clone()
    std::unique_ptr<char[]> stack(new char[STACK_SIZE]);
    char* stack_top = stack.get() + STACK_SIZE;

    // 2. Setup communication pipe for parent-child synchronization.
    // The parent must configure cgroups for the child BEFORE the child executes
    // user binaries, requiring synchronization.
    if (pipe(sync_pipe_) < 0) {
        std::cerr << "[-] Core error: failed to create synchronization pipe: " << std::strerror(errno) << "\n";
        return false;
    }

    // 3. Call clone system call with standard namespaces enabled
    // - CLONE_NEWPID: Private PID namespace (child becomes PID 1)
    // - CLONE_NEWNS: Isolated mount namespace (private mount table)
    // - CLONE_NEWUTS: UTS namespace (custom hostname)
    // - SIGCHLD: Notifies parent on child termination
    int flags = CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWUTS | SIGCHLD;
    
    std::cout << "[+] Spawning child process via clone()...\n";
    pid_t child_pid = clone(child_entry, stack_top, flags, this);

    if (child_pid < 0) {
        std::cerr << "[-] Core error: clone() failed: " << std::strerror(errno) << "\n";
        close(sync_pipe_[0]);
        close(sync_pipe_[1]);
        return false;
    }

    // --- PARENT PROCESS CONTEXT ---
    std::cout << "[+] Cloned child process successfully (Host PID: " << child_pid << ")\n";

    // Close unused read end of synchronization pipe in parent context
    close(sync_pipe_[0]);

    // 4. Configure Cgroups v2 on the child's host-level PID
    if (!Cgroups::setup(child_pid, config_.memory_limit, config_.cpu_limit)) {
        std::cerr << "[-] Warning: Cgroup configuration failed. Proceeding without resource limits.\n";
    }

    // 5. Signal the child process to proceed
    std::cout << "[+] Signaling child process to run execution...\n";
    char sync_signal = 'y';
    if (write(sync_pipe_[1], &sync_signal, 1) != 1) {
        std::cerr << "[-] Core error: failed to signal child process\n";
    }
    close(sync_pipe_[1]);

    // 6. Wait for the container's main process to terminate
    std::cout << "[+] Waiting for container (PID " << child_pid << ") to exit...\n";
    int status;
    if (waitpid(child_pid, &status, 0) < 0) {
        std::cerr << "[-] Core error: waitpid failed: " << std::strerror(errno) << "\n";
    }

    // 7. Cleanup resource configurations
    Cgroups::cleanup(child_pid);

    if (WIFEXITED(status)) {
        std::cout << "[+] Container exited cleanly with status: " << WEXITSTATUS(status) << "\n";
    } else if (WIFSIGNALED(status)) {
        std::cout << "[+] Container terminated by signal: " << WTERMSIG(status) << "\n";
    }

    return true;
}
