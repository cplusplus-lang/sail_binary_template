#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace sail {

class TemplateManager {
public:
    struct TemplateFile {
        std::string path;
        std::string content;
        bool is_binary;
    };

    TemplateManager();

    /// Create a new project from the embedded template
    /// @param project_name The name of the project to create
    /// @param target_directory The directory where to create the project (defaults to current directory)
    /// @param overwrite Whether to overwrite existing files
    /// @return true if successful, false otherwise
    bool create_project(const std::string& project_name, 
                       const std::filesystem::path& target_directory = std::filesystem::current_path(),
                       bool overwrite = false);

    /// Get all template files
    [[nodiscard]] const std::vector<TemplateFile>& get_template_files() const noexcept;

    /// Get a specific template file by path
    [[nodiscard]] const TemplateFile* get_template_file(const std::string& path) const noexcept;

    /// Get the number of template files
    [[nodiscard]] size_t file_count() const noexcept;

private:
    std::vector<TemplateFile> template_files_;
    std::unordered_map<std::string, size_t> file_index_;

    struct TemplateContent {
        std::string_view content;
    };
    struct ProjectName {
        std::string_view name;
    };
    
    /// Process template content by replacing placeholders
    [[nodiscard]] static std::string process_template_content(TemplateContent template_content, 
                                                              ProjectName project_name);

    /// Create directory structure for a file path
    [[nodiscard]] static bool create_directory_structure(const std::filesystem::path& file_path);

    /// Write file to filesystem
    [[nodiscard]] static bool write_file(const std::filesystem::path& file_path, 
                                        const std::string& content, 
                                        bool overwrite);
};

} // namespace sail