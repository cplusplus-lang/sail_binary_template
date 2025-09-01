#pragma once
#include <string>

namespace sail {

class GitUtils {
public:
    static bool clone(const std::string& url, const std::string& targetDir, bool shallow = false);
    static bool shallowClone(const std::string& url, const std::string& targetDir);
    static bool isGitRepository(const std::string& dir);
    static std::string extractRepoName(const std::string& url);
};

} // namespace sail