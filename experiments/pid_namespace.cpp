#define _GNU_SOURCE

#include <sched.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

using namespace std;

int child_func(void* arg) {

    cout << "Inside CHILD namespace\n";

    cout << "Child sees PID: " << getpid() << endl;
    cout << "Child sees Parent PID: " << getppid() << endl;

    system("ls /proc");

    return 0;
}

int main() {

    const int STACK_SIZE = 1024 * 1024;

    char* stack = new char[STACK_SIZE];

    char* stackTop = stack + STACK_SIZE;

    pid_t pid = clone(
        child_func,
        stackTop,

        CLONE_NEWPID | SIGCHLD,

        NULL
    );

    if(pid == -1) {
        perror("clone failed");
        return 1;
    }

    cout << "Host sees child PID as: " << pid << endl;

    waitpid(pid, NULL, 0);

    delete[] stack;

    return 0;
}