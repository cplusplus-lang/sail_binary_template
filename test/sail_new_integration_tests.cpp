#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

#include <sail/template_manager.hpp>

namespace fs = std::filesystem;

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        std::random_device random_device;
        path_ = fs::temp_directory_path() / ("sail_integration_test_" + std::to_string(random_device()));
        fs::create_directories(path_);
    }
    
    ~TemporaryDirectory() {
        if (fs::exists(path_)) {
            fs::remove_all(path_);
        }
    }
    
    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;
    TemporaryDirectory(TemporaryDirectory&&) = default;
    TemporaryDirectory& operator=(TemporaryDirectory&&) = default;
    
    [[nodiscard]] const fs::path& path() const { return path_; }
    
private:
    fs::path path_;
};

TEST_CASE("Integration - Basic project creation", "[integration]") {
    const TemporaryDirectory temp_dir;
    const std::string project_name = "integration_test_project";
    
    sail::TemplateManager manager;
    const bool result = manager.create_project(project_name, temp_dir.path(), false);
    REQUIRE(result == true);
    
    const fs::path project_path = temp_dir.path() / project_name;
    REQUIRE(fs::exists(project_path));
    REQUIRE(fs::is_directory(project_path));
}

TEST_CASE("Integration - Essential files created", "[integration]") {
    const TemporaryDirectory temp_dir;
    const std::string project_name = "file_test_project";
    
    sail::TemplateManager manager;
    REQUIRE(manager.create_project(project_name, temp_dir.path(), false) == true);
    
    const fs::path project_path = temp_dir.path() / project_name;
    
    // Check essential files exist
    REQUIRE(fs::exists(project_path / "CMakeLists.txt"));
    REQUIRE(fs::exists(project_path / "README.md"));
    
    // Files should not be empty
    REQUIRE(fs::file_size(project_path / "CMakeLists.txt") > 0);
    REQUIRE(fs::file_size(project_path / "README.md") > 0);
}

TEST_CASE("Integration - Project name replacement", "[integration]") {
    const TemporaryDirectory temp_dir;
    const std::string project_name = "ProjectNameTest";
    
    sail::TemplateManager manager;
    REQUIRE(manager.create_project(project_name, temp_dir.path(), false) == true);
    
    const fs::path cmake_file = temp_dir.path() / project_name / "CMakeLists.txt";
    REQUIRE(fs::exists(cmake_file));
    
    std::ifstream file(cmake_file);
    const std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    
    // Should contain actual project name
    REQUIRE(content.find(project_name) != std::string::npos);
    
    // Should not contain placeholders
    REQUIRE(content.find("%%PROJECT_NAME%%") == std::string::npos);
    REQUIRE(content.find("myproject") == std::string::npos);
}

TEST_CASE("Integration - Multiple projects", "[integration]") {
    const TemporaryDirectory temp_dir;
    sail::TemplateManager manager;
    
    const std::string project1 = "project_one";
    const std::string project2 = "project_two";
    
    // Create first project
    REQUIRE(manager.create_project(project1, temp_dir.path(), false) == true);
    REQUIRE(fs::exists(temp_dir.path() / project1));
    
    // Create second project
    REQUIRE(manager.create_project(project2, temp_dir.path(), false) == true);
    REQUIRE(fs::exists(temp_dir.path() / project2));
    
    // Both should exist independently
    REQUIRE(fs::exists(temp_dir.path() / project1 / "CMakeLists.txt"));
    REQUIRE(fs::exists(temp_dir.path() / project2 / "CMakeLists.txt"));
}

TEST_CASE("Integration - File count consistency", "[integration]") {
    const TemporaryDirectory temp_dir;
    const std::string project_name = "file_count_test";
    
    sail::TemplateManager manager;
    REQUIRE(manager.create_project(project_name, temp_dir.path(), false) == true);
    
    const fs::path project_path = temp_dir.path() / project_name;
    
    // Count created files
    std::size_t created_files = 0;
    for (const auto& entry : fs::recursive_directory_iterator(project_path)) {
        if (entry.is_regular_file()) {
            ++created_files;
        }
    }
    
    // Should have created a reasonable number of files
    REQUIRE(created_files > 5);
    
    // Template manager should report file count
    REQUIRE(manager.file_count() > 0);
    REQUIRE(manager.get_template_files().size() == manager.file_count());
}