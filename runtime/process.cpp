#include "process.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cerrno>

namespace Process {

bool execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "[-] Process error: no arguments provided to execute\n";
        return false;
    }

    // Convert std::vector<std::string> to char* const* array for execvp
    std::vector<char*> c_args;
    c_args.reserve(args.size() + 1);

    for (const auto& arg : args) {
        c_args.push_back(const_cast<char*>(arg.c_str()));
    }
    c_args.push_back(nullptr); // Null-terminate the list

    std::cout << "[+] Executing inside container: " << args[0] << "\n";
    
    // execvp replaces the current process image with a new process image.
    // If successful, this function never returns.
    execvp(c_args[0], c_args.data());

    // If execvp returns, an error occurred
    std::cerr << "[-] execvp failed for " << args[0] << ": " << std::strerror(errno) << "\n";
    return false;
}

} // namespace Process
