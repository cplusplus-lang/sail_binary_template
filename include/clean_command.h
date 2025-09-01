#pragma once
#include <string>
#include <vector>

namespace sail {

class CleanCommand {
public:
    int execute(const std::vector<std::string>& args);

private:
    void printUsage() const;
    bool parseArguments(const std::vector<std::string>& args);
    bool cleanBuildDirectory(const std::string& buildDir, bool dryRun, bool verbose);
    bool cleanAllBuildDirectories(bool dryRun, bool verbose);
    void printRemovalMessage(const std::string& path, bool dryRun, bool verbose);
    std::string formatFileSize(std::uintmax_t size) const;
    std::uintmax_t getDirectorySize(const std::string& path) const;
    
    // Command line options
    bool m_release = false;
    bool m_debug = false;
    bool m_dryRun = false;
    bool m_verbose = false;
    bool m_quiet = false;
    bool m_doc = false;
    bool m_help = false;
};

} // namespace sail