#pragma once
#include <string>
#include <vector>

namespace sail {

class RunCommand {
public:
    int execute(const std::vector<std::string>& args);

private:
    bool isReleaseBuild(const std::vector<std::string>& args) const;
    bool isVerbose(const std::vector<std::string>& args) const;
    std::string getBinaryName(const std::vector<std::string>& args) const;
    std::vector<std::string> getExecutableArgs(const std::vector<std::string>& args) const;
    std::string getBuildDir(bool release) const;
    std::string findExecutable(const std::string& buildDir, const std::string& binaryName) const;
    int buildProject(bool release, bool verbose) const;
    int runExecutable(const std::string& executablePath, const std::vector<std::string>& execArgs) const;
    void printRunHelp() const;
    std::string getProjectName() const;
};

} // namespace sail