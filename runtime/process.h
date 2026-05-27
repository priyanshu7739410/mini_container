#ifndef PROCESS_H
#define PROCESS_H

#include <vector>
#include <string>

namespace Process {
    // Executes the program defined by args using execvp.
    // This is a terminating call in the child process unless an error occurs.
    bool execute(const std::vector<std::string>& args);
}

#endif // PROCESS_H
