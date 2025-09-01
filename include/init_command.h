#pragma once
#include <string>
#include <vector>

namespace sail {

class InitCommand {
public:
    int execute(const std::vector<std::string>& args);

private:
    bool initProject(const std::string& projectName, const std::string& projectPath, bool isBin = true);
    bool createSourceFile(const std::string& projectPath, const std::string& projectName, bool isBin);
    bool createSailTomlFile(const std::string& projectPath, const std::string& projectName);
    bool createGitIgnoreFile(const std::string& projectPath);
    void printUsage() const;
    std::string sanitizeProjectName(const std::string& name) const;
    std::string getCurrentDirectoryName() const;
    bool hasExistingSourceFiles(const std::string& projectPath) const;
};

} // namespace sail