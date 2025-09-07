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
        path_ = fs::temp_directory_path() / ("sail_test_" + std::to_string(random_device()));
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

TEST_CASE("TemplateManager basic functionality", "[template_manager]") {
    const sail::TemplateManager manager;
    
    SECTION("Manager loads template files") {
        REQUIRE(manager.file_count() > 0);
        REQUIRE_FALSE(manager.get_template_files().empty());
    }
}

TEST_CASE("Project creation with valid names", "[create_project]") {
    const TemporaryDirectory temp_dir;
    sail::TemplateManager manager;
    
    SECTION("Creates project with simple name") {
        const std::string project_name = "test_project";
        const bool result = manager.create_project(project_name, temp_dir.path(), false);
        REQUIRE(result == true);
        
        const fs::path project_path = temp_dir.path() / project_name;
        REQUIRE(fs::exists(project_path));
        REQUIRE(fs::is_directory(project_path));
    }
    
    SECTION("Creates project with underscores") {
        const std::string project_name = "test_project_name";
        const bool result = manager.create_project(project_name, temp_dir.path(), false);
        REQUIRE(result == true);
        
        const fs::path project_path = temp_dir.path() / project_name;
        REQUIRE(fs::exists(project_path));
    }
    
    SECTION("Creates project with mixed case") {
        const std::string project_name = "TestProject";
        const bool result = manager.create_project(project_name, temp_dir.path(), false);
        REQUIRE(result == true);
        
        const fs::path project_path = temp_dir.path() / project_name;
        REQUIRE(fs::exists(project_path));
    }
}

TEST_CASE("Project creation with invalid names", "[create_project]") {
    const TemporaryDirectory temp_dir;
    sail::TemplateManager manager;
    
    SECTION("Rejects empty name") {
        const std::string project_name;
        const bool result = manager.create_project(project_name, temp_dir.path(), false);
        REQUIRE(result == false);
    }
    
    SECTION("Rejects name starting with number") {
        const std::string project_name = "123project";
        const bool result = manager.create_project(project_name, temp_dir.path(), false);
        REQUIRE(result == false);
    }
    
    SECTION("Rejects name with spaces") {
        const std::string project_name = "project name";
        const bool result = manager.create_project(project_name, temp_dir.path(), false);
        REQUIRE(result == false);
    }
    
    SECTION("Rejects name with special characters") {
        const std::string project_name = "project@name";
        const bool result = manager.create_project(project_name, temp_dir.path(), false);
        REQUIRE(result == false);
    }
}

TEST_CASE("Project overwrite behavior", "[create_project]") {
    const TemporaryDirectory temp_dir;
    sail::TemplateManager manager;
    const std::string project_name = "test_overwrite";
    
    SECTION("Fails when directory exists without overwrite") {
        // Create project first time
        REQUIRE(manager.create_project(project_name, temp_dir.path(), false) == true);
        
        // Try to create again without overwrite
        REQUIRE(manager.create_project(project_name, temp_dir.path(), false) == false);
    }
    
    SECTION("Succeeds when directory exists with overwrite") {
        // Create project first time
        REQUIRE(manager.create_project(project_name, temp_dir.path(), false) == true);
        
        // Try to create again with overwrite
        REQUIRE(manager.create_project(project_name, temp_dir.path(), true) == true);
    }
}

TEST_CASE("Template creates expected files", "[create_project]") {
    const TemporaryDirectory temp_dir;
    sail::TemplateManager manager;
    const std::string project_name = "MyTestProject";
    
    REQUIRE(manager.create_project(project_name, temp_dir.path(), false) == true);
    
    const fs::path project_path = temp_dir.path() / project_name;
    
    // Check for CMakeLists.txt
    REQUIRE(fs::exists(project_path / "CMakeLists.txt"));
    
    // Check for README.md
    REQUIRE(fs::exists(project_path / "README.md"));
}

TEST_CASE("Template replaces project name", "[create_project]") {
    const TemporaryDirectory temp_dir;
    sail::TemplateManager manager;
    const std::string project_name = "MyTestProject";
    
    REQUIRE(manager.create_project(project_name, temp_dir.path(), false) == true);
    
    const fs::path project_path = temp_dir.path() / project_name;
    const fs::path cmake_file = project_path / "CMakeLists.txt";
    
    if (fs::exists(cmake_file)) {
        std::ifstream file(cmake_file);
        const std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        // Should contain the actual project name
        REQUIRE(content.find(project_name) != std::string::npos);
    }
}

TEST_CASE("Template file access", "[template_manager]") {
    const sail::TemplateManager manager;
    
    SECTION("file_count is consistent") {
        const std::size_t count1 = manager.file_count();
        const std::size_t count2 = manager.get_template_files().size();
        
        REQUIRE(count1 == count2);
        REQUIRE(count1 > 0);
    }
    
    SECTION("get_template_file returns nullptr for invalid path") {
        REQUIRE(manager.get_template_file("nonexistent") == nullptr);
    }
}

TEST_CASE("Edge cases", "[create_project]") {
    const TemporaryDirectory temp_dir;
    sail::TemplateManager manager;
    
    SECTION("Handles nested directory creation") {
        const fs::path nested_path = temp_dir.path() / "level1" / "level2";
        fs::create_directories(nested_path);
        
        const std::string project_name = "nested_project";
        const bool result = manager.create_project(project_name, nested_path, false);
        
        REQUIRE(result == true);
        
        const fs::path project_path = nested_path / project_name;
        REQUIRE(fs::exists(project_path));
        REQUIRE(fs::is_directory(project_path));
    }
}