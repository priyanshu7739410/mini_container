#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

int main() {

    cout << "Program started\n";

    pid_t pid = fork();

    if(pid < 0) {
        cerr << "Fork failed\n";
        return 1;
    }

    if(pid == 0) {

        // Child process

        cout << "\nInside CHILD process\n";
        cout << "Child PID: " << getpid() << endl;
        cout << "Parent PID: " << getppid() << endl;

        char* args[] = {(char*)"/bin/bash", NULL};

        execvp(args[0], args);

        perror("execvp failed");
    }
    else {

        // Parent process

        cout << "\nInside PARENT process\n";
        cout << "Parent PID: " << getpid() << endl;
        cout << "Child PID: " << pid << endl;

        waitpid(pid, NULL, 0);

        cout << "\nChild process finished\n";
    }

    return 0;
}