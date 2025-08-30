#pragma once
#include <string>
#include <vector>

namespace sail {

class CMakeBuilder {
public:
    bool configure(const std::string& sourceDir, const std::string& buildDir);
    bool build(const std::string& buildDir);
    std::vector<std::string> findExecutables(const std::string& buildDir);

private:
    bool runCommand(const std::string& command, const std::string& workingDir = "");
    void findExecutablesRecursive(const std::string& dir, std::vector<std::string>& executables);
};

} // namespace sail