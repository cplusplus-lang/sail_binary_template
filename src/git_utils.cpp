#include "git_utils.h"
#include "utils.h"
#include <iostream>
#include <cstdlib>
#include <sstream>

namespace sail {

bool GitUtils::clone(const std::string& url, const std::string& targetDir) {
    std::string command = "git clone \"" + url + "\" \"" + targetDir + "\"";
    
    std::cout << "Cloning " << url << "..." << std::endl;
    int result = std::system(command.c_str());
    
    if (result == 0) {
        std::cout << "Successfully cloned repository" << std::endl;
        return true;
    } else {
        std::cerr << "Failed to clone repository" << std::endl;
        return false;
    }
}

bool GitUtils::isGitRepository(const std::string& dir) {
    std::string gitDir = dir + "/.git";
    return Utils::directoryExists(gitDir);
}

std::string GitUtils::extractRepoName(const std::string& url) {
    // Extract repository name from URL
    // e.g., https://github.com/cplusplus-lang/names -> names
    
    size_t lastSlash = url.find_last_of('/');
    if (lastSlash == std::string::npos) {
        return url;
    }
    
    std::string name = url.substr(lastSlash + 1);
    
    // Remove .git suffix if present
    if (name.size() > 4 && name.substr(name.size() - 4) == ".git") {
        name = name.substr(0, name.size() - 4);
    }
    
    return name;
}

} // namespace sail