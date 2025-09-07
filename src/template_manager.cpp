#include <sail/template_manager.hpp>
#include <embedded_templates.hpp>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <regex>
#include <utility>
#include <vector>

namespace sail {

TemplateManager::TemplateManager() {
    // Load embedded template files
    template_files_.reserve(templates::file_count);
    
    for (const auto& embedded_file : templates::embedded_files) {
        TemplateFile file;
        file.path = std::string{embedded_file.path};
        file.content = std::string{embedded_file.content};
        file.is_binary = embedded_file.is_binary;
        
        file_index_[file.path] = template_files_.size();
        template_files_.emplace_back(std::move(file));
    }
}

bool TemplateManager::create_project(const std::string& project_name, 
                                    const std::filesystem::path& target_directory,
                                    bool overwrite) {
    if (project_name.empty()) {
        std::cerr << "Error: Project name cannot be empty\n";
        return false;
    }

    // Validate project name (basic validation - alphanumeric and underscores)
    if (!std::regex_match(project_name, std::regex("^[a-zA-Z][a-zA-Z0-9_]*$"))) {
        std::cerr << "Error: Invalid project name. Use only letters, numbers, and underscores. Must start with a letter.\n";
        return false;
    }

    const auto project_path = target_directory / project_name;
    
    // Check if project directory already exists
    if (std::filesystem::exists(project_path) && !overwrite) {
        std::cerr << "Error: Directory '" << project_path << "' already exists. Use --overwrite to force creation.\n";
        return false;
    }

    std::cout << "Creating project '" << project_name << "' in " << project_path << "\n";

    // Create project directory
    try {
        std::filesystem::create_directories(project_path);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: Failed to create project directory: " << e.what() << "\n";
        return false;
    }

    // Process and write each template file
    size_t files_created = 0;
    for (const auto& template_file : template_files_) {
        if (template_file.is_binary) {
            std::cerr << "Warning: Skipping binary file: " << template_file.path << "\n";
            continue;
        }

        // Process the file path (replace myproject in paths)
        std::string processed_path = template_file.path;
        const std::string template_placeholder = "myproject";
        size_t pos = 0;
        while ((pos = processed_path.find(template_placeholder, pos)) != std::string::npos) {
            processed_path.replace(pos, template_placeholder.length(), project_name);
            pos += project_name.length();
        }

        const auto target_file_path = project_path / processed_path;
        
        // Create directory structure
        if (!create_directory_structure(target_file_path)) {
            std::cerr << "Error: Failed to create directory structure for " << target_file_path << "\n";
            return false;
        }

        // Process template content (replace placeholders)
        const auto processed_content = process_template_content(
            TemplateContent{template_file.content}, 
            ProjectName{project_name});

        // Write file
        if (write_file(target_file_path, processed_content, overwrite)) {
            ++files_created;
        } else {
            std::cerr << "Error: Failed to write file: " << target_file_path << "\n";
            return false;
        }
    }

    std::cout << "Successfully created project with " << files_created << " files\n";
    std::cout << "Next steps:\n";
    std::cout << "  cd " << project_name << "\n";
    std::cout << "  cmake -S . -B build\n";
    std::cout << "  cmake --build build\n";
    
    return true;
}

const std::vector<TemplateManager::TemplateFile>& TemplateManager::get_template_files() const noexcept {
    return template_files_;
}

const TemplateManager::TemplateFile* TemplateManager::get_template_file(const std::string& path) const noexcept {
    const auto iter = file_index_.find(path);
    if (iter != file_index_.end()) {
        return &template_files_[iter->second];
    }
    return nullptr;
}

size_t TemplateManager::file_count() const noexcept {
    return template_files_.size();
}

std::string TemplateManager::process_template_content(TemplateContent template_content, 
                                                    ProjectName project_name) {
    std::string processed{template_content.content};
    
    // Common template replacements - these should match the template's placeholders
    std::string project_name_lower{project_name.name};
    std::ranges::transform(project_name_lower, project_name_lower.begin(), 
                          [](unsigned char character) { return std::tolower(character); });
    
    std::string project_name_upper{project_name.name};
    std::ranges::transform(project_name_upper, project_name_upper.begin(), 
                          [](unsigned char character) { return std::toupper(character); });
    
    const std::vector<std::pair<std::string, std::string>> replacements = {
        {"myproject", std::string{project_name.name}},
        {"%%myproject%%", std::string{project_name.name}},
        {"%%PROJECT_NAME%%", std::string{project_name.name}},
        {"%%project_name%%", std::string{project_name.name}},
        {"%%PROJECT_NAME_LOWER%%", project_name_lower},
        {"%%PROJECT_NAME_UPPER%%", project_name_upper}
    };

    for (const auto& [placeholder, replacement] : replacements) {
        size_t pos = 0;
        while ((pos = processed.find(placeholder, pos)) != std::string::npos) {
            processed.replace(pos, placeholder.length(), replacement);
            pos += replacement.length();
        }
    }

    return processed;
}

bool TemplateManager::create_directory_structure(const std::filesystem::path& file_path) {
    try {
        const auto parent_path = file_path.parent_path();
        if (!parent_path.empty() && !std::filesystem::exists(parent_path)) {
            std::filesystem::create_directories(parent_path);
        }
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
        return false;
    }
}

bool TemplateManager::write_file(const std::filesystem::path& file_path, 
                                const std::string& content, 
                                bool overwrite) {
    if (std::filesystem::exists(file_path) && !overwrite) {
        std::cerr << "File already exists: " << file_path << "\n";
        return false;
    }

    try {
        std::ofstream file(file_path, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open file for writing: " << file_path << "\n";
            return false;
        }

        file.write(content.data(), static_cast<std::streamsize>(content.size()));
        
        if (!file) {
            std::cerr << "Failed to write content to file: " << file_path << "\n";
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception while writing file " << file_path << ": " << e.what() << "\n";
        return false;
    }
}

} // namespace sail