#pragma once
#include <string>
#include <vector>

namespace sail {

class InstallCommand {
public:
    int execute(const std::vector<std::string>& args);

protected:
    virtual std::string getInstallDir() const;
    std::string createTempDir() const;
    bool cloneRepository(const std::string& url, const std::string& targetDir, bool shallow = true);
    bool buildProject(const std::string& sourceDir, const std::string& buildDir);
    bool installBinaries(const std::string& buildDir, const std::string& installDir);
    void cleanupTempDir(const std::string& tempDir);
    
private:
    bool m_fullClone = false;  // Flag to control shallow vs full clone
};

} // namespace sail