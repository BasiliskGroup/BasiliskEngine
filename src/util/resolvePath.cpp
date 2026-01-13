#include <basilisk/util/resolvePath.h>
#include <dlfcn.h>
#include <filesystem>
#include <unistd.h>

namespace bsk::internal {

// Static variable to store Python's working directory
static std::filesystem::path python_working_directory;

// Non-inline function to get the .so file path
// This function itself is in the .so, so dladdr will return the correct path
std::filesystem::path getModulePath() {
    Dl_info info;
    if (dladdr((void*)&getModulePath, &info) != 0 && info.dli_fname != nullptr) {
        std::filesystem::path soPath(info.dli_fname);
        return soPath.parent_path();
    }
    // Fallback to current directory if dladdr fails
    return std::filesystem::current_path();
}

// Set the Python working directory (called from module initialization)
void setPythonWorkingDirectory(const std::string& path) {
    python_working_directory = std::filesystem::path(path);
}

// Get the current working directory
// Uses Python's working directory if set, otherwise falls back to getcwd()
std::filesystem::path getCurrentWorkingDirectory() {
    // If Python working directory was set, use it
    if (!python_working_directory.empty()) {
        return python_working_directory;
    }
    
    // Otherwise, try getcwd()
    char buf[4096];
    if (getcwd(buf, sizeof(buf)) != nullptr) {
        return std::filesystem::path(buf);
    }
    
    // Final fallback
    return std::filesystem::current_path();
}

}
