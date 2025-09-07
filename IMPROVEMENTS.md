# Sail Template Generation Improvements

This document outlines potential improvements for the `sail new` template generation functionality.

## Current State Analysis

### Strengths
- ✅ Self-contained binary with embedded templates (like Cargo)
- ✅ Basic template substitution (`myproject` → actual project name)
- ✅ Path substitution (directory names updated correctly)
- ✅ Clean C++ API with modern practices
- ✅ CMake integration for template embedding
- ✅ 48 template files successfully embedded and generated

### Current Limitations
- Limited to single hardcoded template
- Basic placeholder replacement (only `myproject`)
- Simple binary file detection and exclusion
- No user customization options
- No progress indication for large operations
- No validation of generated output
- No git integration

## Proposed Improvements

### 1. Template Variables & Configuration System
**Priority: HIGH**

#### Current Behavior
```cpp
// Only supports basic project name replacement
{"myproject", project_name}
```

#### Proposed Enhancement
```cpp
struct TemplateConfig {
    // Project info
    std::string project_name;
    std::string description;
    std::string version = "0.1.0";
    
    // Author info  
    std::string author_name;
    std::string author_email;
    std::string github_username;
    
    // Project settings
    std::string license = "MIT";
    std::string cpp_standard = "23";
    bool enable_testing = true;
    bool enable_fuzzing = false;
    bool enable_docs = true;
    bool enable_benchmarking = false;
    
    // Build options
    std::string cmake_minimum_version = "3.21";
    bool use_vcpkg = false;
    bool use_conan = false;
};
```

#### Template Variables
```cmake
# In CMakeLists.txt template
project(
  {{PROJECT_NAME}}
  VERSION {{VERSION}}
  DESCRIPTION "{{DESCRIPTION}}"
  HOMEPAGE_URL "https://github.com/{{GITHUB_USERNAME}}/{{PROJECT_NAME}}"
  LANGUAGES CXX C
)

# In README.md template  
# {{PROJECT_NAME}}

{{DESCRIPTION}}

## Author
{{AUTHOR_NAME}} <{{AUTHOR_EMAIL}}>

## License
{{LICENSE}}
```

#### CLI Interface
```bash
sail new my_project \
  --author "John Doe" \
  --email "john@example.com" \
  --description "A modern C++ project" \
  --license MIT \
  --cpp-std 23 \
  --enable-testing \
  --disable-fuzzing

# Interactive mode
sail new my_project --interactive
```

### 2. Multiple Project Templates
**Priority: HIGH**

#### Template Types
```cpp
enum class TemplateType {
    ConsoleApp,    // CLI application with argument parsing
    Library,       // Static/shared library only
    GuiApp,        // FTXUI-based GUI application
    HeaderOnly,    // Header-only library
    Game,          // Game template with graphics
    WebServer,     // HTTP server template
    Custom         // User-defined template
};
```

#### CLI Interface
```bash
sail new my_app --template=console
sail new my_lib --template=library
sail new my_gui --template=gui
sail new my_header_lib --template=header-only

# List available templates
sail template list
```

#### Template Structure
```
templates/
├── console/           # CLI application template
├── library/           # Library template  
├── gui/              # FTXUI GUI template
├── header-only/      # Header-only library
└── custom/           # User custom templates
```

### 3. Enhanced Binary File Handling
**Priority: MEDIUM**

#### Current Behavior
```cmake
# Simple extension-based detection
if(FILE_EXT MATCHES "\\.(png|jpg|jpeg|gif|bmp|ico|exe|bin|so|dylib|dll)$")
    message(WARNING "Skipping binary file: ${FILE}")
    continue()
endif()
```

#### Proposed Enhancement
```cpp
class BinaryFileHandler {
public:
    enum class BinaryType {
        Image,
        Executable, 
        Archive,
        Unknown
    };
    
    struct BinaryFile {
        std::string path;
        BinaryType type;
        std::vector<uint8_t> data;  // For small files
        std::string base64_data;    // Encoded data
        bool should_embed;
    };
    
    static bool is_binary_file(const std::filesystem::path& path);
    static BinaryType detect_binary_type(const std::filesystem::path& path);
    static std::string encode_binary_file(const std::vector<uint8_t>& data);
};
```

#### Binary File Support
- **Images**: Encode small icons/logos as base64 in templates
- **Archives**: Support for including example assets
- **Smart Detection**: Content-based binary detection, not just extensions
- **Size Limits**: Only embed binary files under configurable size limit

### 4. Template Customization System
**Priority: MEDIUM**

#### Custom Template Creation
```bash
# Create custom template from existing project
sail template create my_template --from-project ./existing_project

# Initialize new custom template 
sail template init my_template

# Edit template
sail template edit my_template

# Package template for sharing
sail template package my_template --output my_template.tar.gz

# Install shared template
sail template install ./my_template.tar.gz
```

#### Template Configuration
```toml
# template.toml
[template]
name = "my_custom_template"
description = "Custom C++ project template"
version = "1.0.0"
author = "John Doe <john@example.com>"

[variables]
project_name = { type = "string", required = true }
use_opengl = { type = "bool", default = false, description = "Include OpenGL support" }
database = { type = "enum", options = ["sqlite", "postgresql", "none"], default = "none" }

[files]
# Conditional file inclusion
"src/graphics/" = { condition = "use_opengl" }
"src/database/" = { condition = "database != 'none'" }
```

### 5. Performance Optimizations
**Priority: LOW**

#### Current Issues
- All templates loaded on every `TemplateManager` instantiation
- String-based template storage (not compressed)
- No caching of processed templates

#### Proposed Optimizations
```cpp
class TemplateCache {
public:
    // Lazy loading of templates
    const TemplateFile* get_template_file(std::string_view path);
    
    // Compressed storage
    std::vector<uint8_t> compress_template_data(std::string_view content);
    std::string decompress_template_data(const std::vector<uint8_t>& compressed);
    
    // Cache processed templates
    void cache_processed_template(const std::string& key, const std::string& content);
    std::optional<std::string> get_cached_template(const std::string& key);
};
```

#### Benefits
- **Faster Startup**: Only load templates when needed
- **Smaller Binary**: Compressed template storage
- **Better Performance**: Cache frequently used templates

### 6. Better Error Handling & Progress
**Priority: MEDIUM**

#### Enhanced User Experience
```cpp
class ProjectGenerator {
    // Progress reporting
    using ProgressCallback = std::function<void(const std::string& step, float progress)>;
    
    bool create_project(const ProjectConfig& config, ProgressCallback callback = nullptr);
    
    // Detailed error reporting
    struct GenerationError {
        enum Type { ValidationError, FileSystemError, TemplateError };
        Type type;
        std::string message;
        std::string suggestion;
        std::filesystem::path affected_path;
    };
    
    std::vector<GenerationError> validate_config(const ProjectConfig& config);
};
```

#### CLI Features
```bash
# Dry run mode
sail new my_project --dry-run

# Verbose output
sail new my_project --verbose

# Progress indication
Creating project 'my_project'...
[██████████████████████████████] 100% (48/48 files)
✓ Project created successfully
```

### 7. Git Integration
**Priority: MEDIUM**

#### Automatic Git Setup
```bash
# Initialize git repository
sail new my_project --git

# Initialize with remote
sail new my_project --git --remote=https://github.com/user/my_project.git

# Create with initial commit
sail new my_project --git --initial-commit="Initial project setup"

# Add .gitignore based on project type
sail new my_project --git --gitignore=cpp
```

#### Implementation
```cpp
class GitIntegration {
public:
    static bool init_repository(const std::filesystem::path& project_path);
    static bool add_remote(const std::filesystem::path& project_path, 
                          const std::string& remote_url);
    static bool create_initial_commit(const std::filesystem::path& project_path,
                                    const std::string& message);
    static bool add_gitignore(const std::filesystem::path& project_path,
                            const std::string& template_name);
};
```

### 8. Project Structure Validation
**Priority: LOW**

#### Validation Features
```cpp
class ProjectValidator {
public:
    struct ValidationResult {
        bool is_valid;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
    };
    
    static ValidationResult validate_cmake_syntax(const std::filesystem::path& cmake_file);
    static ValidationResult validate_project_structure(const std::filesystem::path& project_path);
    static ValidationResult validate_template_substitution(const std::vector<TemplateFile>& files);
};
```

## Implementation Roadmap

### Phase 1: Template Variables System (Week 1-2)
1. Design and implement `TemplateConfig` structure
2. Extend template substitution engine
3. Add CLI argument parsing for template variables
4. Update existing template with new variable placeholders
5. Add interactive mode for template configuration

### Phase 2: Multiple Template Support (Week 3-4)  
1. Refactor template embedding to support multiple templates
2. Create template type enumeration and selection logic
3. Design and implement different project templates (console, library, GUI)
4. Add template listing and selection CLI commands
5. Update CMake template generation for multi-template support

### Phase 3: Enhanced Features (Week 5-6)
1. Implement better binary file handling with encoding
2. Add git integration features
3. Implement progress reporting and better error handling
4. Add dry-run and validation features

### Phase 4: Advanced Features (Week 7-8)
1. Implement custom template system
2. Add template sharing and packaging
3. Performance optimizations and caching
4. Comprehensive testing and documentation

## Breaking Changes

### API Changes
- `TemplateManager::create_project()` signature will change to accept `TemplateConfig`
- New CLI argument structure for template selection and configuration

### Migration Path
- Maintain backward compatibility for simple `sail new <name>` usage
- Provide migration guide for advanced users
- Deprecate old APIs gradually

## Testing Strategy

### Unit Tests
- Template variable substitution
- Binary file handling
- Project validation
- Git integration

### Integration Tests
- End-to-end project generation
- Multiple template types
- Custom template creation and usage

### Performance Tests  
- Template loading and generation speed
- Memory usage optimization
- Large project generation

## Documentation Updates

### User Documentation
- Updated CLI usage examples
- Template creation guide
- Configuration reference
- Migration guide

### Developer Documentation
- API reference updates
- Architecture documentation
- Contributing guidelines for new templates

---

## Conclusion

These improvements would transform the `sail new` command from a basic project generator into a comprehensive, flexible template system comparable to modern tools like `cargo new`, `npm create`, or `dotnet new`. The modular approach allows for incremental implementation while maintaining backward compatibility.

The proposed changes prioritize user experience, flexibility, and maintainability while preserving the core strengths of the current implementation: self-contained binary distribution and fast project generation.