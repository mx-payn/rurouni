// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/system/command.hpp"

namespace rr::system {

std::string execute_command(const std::string& command) {
    // open command pipe
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe)
        return "ERROR";
    char buffer[128];
    std::string result = "";
    // write result into buffer
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    // remove newline
    if (result.back() == '\n')
        result.pop_back();

    return result;
}

}  // namespace rr::system
