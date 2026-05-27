#define _GNU_SOURCE

#include <sched.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <unistd.h>

#include <iostream>

using namespace std;

int child_func(void* arg) {

    cout << "Inside container-like process\n";

    cout << "PID inside namespace: " << getpid() << endl;

    // Mount fresh proc filesystem
    mount(
        "proc",
        "/proc",
        "proc",
        0,
        NULL
    );

    system("ps aux");

    umount("/proc");

    return 0;
}

int main() {

    const int STACK_SIZE = 1024 * 1024;

    char* stack = new char[STACK_SIZE];

    char* stackTop = stack + STACK_SIZE;

    pid_t pid = clone(
        child_func,
        stackTop,

        CLONE_NEWPID |
        CLONE_NEWNS  |
        SIGCHLD,

        NULL
    );

    if(pid == -1) {
        perror("clone failed");
        return 1;
    }

    cout << "Host sees child PID: " << pid << endl;

    waitpid(pid, NULL, 0);

    delete[] stack;

    return 0;
}