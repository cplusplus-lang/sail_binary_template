#pragma once
#include <string>
#include <vector>

namespace sail {

class CLI {
public:
    int run(int argc, char* argv[]);

private:
    void printHelp() const;
    void printVersion() const;
    int executeInstall(const std::vector<std::string>& args);
    int executeList() const;
};

} // namespace sail