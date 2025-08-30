#pragma once
#include <string>

namespace sail {

class InstallCommand {
public:
    int execute(const std::string& packageUrl);

protected:
    virtual std::string getInstallDir() const;
    std::string createTempDir() const;
    bool cloneRepository(const std::string& url, const std::string& targetDir);
    bool buildProject(const std::string& sourceDir, const std::string& buildDir);
    bool installBinaries(const std::string& buildDir, const std::string& installDir);
    void cleanupTempDir(const std::string& tempDir);
};

} // namespace sail