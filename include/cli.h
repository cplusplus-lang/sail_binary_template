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
    int executeNew(const std::vector<std::string>& args);
    int executeBuild(const std::vector<std::string>& args);
    int executeRun(const std::vector<std::string>& args);
};

} // namespace sail