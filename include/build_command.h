#pragma once
#include <string>
#include <vector>

namespace sail {

class BuildCommand {
public:
    int execute(const std::vector<std::string>& args);

private:
    bool isReleaseBuild(const std::vector<std::string>& args) const;
    bool isVerbose(const std::vector<std::string>& args) const;
    std::string getBuildDir(bool release) const;
    bool copyCPMFile(const std::string& buildDir) const;
    int runCMakeConfigure(const std::string& buildDir, bool release) const;
    int runCMakeBuild(const std::string& buildDir, bool verbose) const;
    void printBuildHelp() const;
    
    // CMake structure generation functions
    bool ensureCMakeStructure() const;
    bool createRootCMakeListsFile() const;
    bool createCMakeDirectory() const;
    bool extractEmbeddedCMakeFiles(const std::string& sailDir) const;
    bool createSailTomlModule(const std::string& cmakeDir) const;
    bool createCMakeListsFile(const std::string& cmakeDir, const std::string& projectName, bool isBin) const;
    bool isLibraryProject() const;
    std::string getProjectNameFromToml() const;
    std::string getCppStandardFromToml() const;
};

} // namespace sail