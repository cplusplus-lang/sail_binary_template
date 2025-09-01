#pragma once
#include <string>

namespace sail {

class Utils {
public:
    static std::string getHomeDirectory();
    static std::string getSailDirectory();
    static std::string getSailBinDirectory();
    static bool createDirectoryRecursive(const std::string& path);
    static bool fileExists(const std::string& path);
    static bool directoryExists(const std::string& path);
    static std::string getTemporaryDirectory();
    static void removeDirectory(const std::string& path);
    static bool copyFile(const std::string& source, const std::string& destination);
    static bool isExecutable(const std::string& path);
    static void makeExecutable(const std::string& path);
    static std::string findProjectRoot(const std::string& startPath = ".");
    static bool isValidCppStandard(const std::string& standard);
};

} // namespace sail