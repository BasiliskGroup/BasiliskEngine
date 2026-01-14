#include <basilisk/util/resolvePath.h>
#include <filesystem>
#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
#else
    #include <dlfcn.h>
    #include <unistd.h>
#endif

namespace bsk::internal {

// Static variable to store Python's working directory
static std::filesystem::path python_working_directory;

// Non-inline function to get the .so/.dll file path
// This function itself is in the .so/.dll, so dladdr/GetModuleFileName will return the correct path
std::filesystem::path getModulePath() {
#ifdef _WIN32
    char path[MAX_PATH];
    HMODULE hm = NULL;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCSTR)&getModulePath, &hm) == 0) {
        return std::filesystem::current_path();
    }
    if (GetModuleFileNameA(hm, path, sizeof(path)) == 0) {
        return std::filesystem::current_path();
    }
    std::filesystem::path dllPath(path);
    return dllPath.parent_path();
#else
    Dl_info info;
    if (dladdr((void*)&getModulePath, &info) != 0 && info.dli_fname != nullptr) {
        std::filesystem::path soPath(info.dli_fname);
        return soPath.parent_path();
    }
    // Fallback to current directory if dladdr fails
    return std::filesystem::current_path();
#endif
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
    
    // Otherwise, try getcwd() / _getcwd()
#ifdef _WIN32
    char buf[MAX_PATH];
    if (_getcwd(buf, sizeof(buf)) != nullptr) {
        return std::filesystem::path(buf);
    }
#else
    char buf[4096];
    if (getcwd(buf, sizeof(buf)) != nullptr) {
        return std::filesystem::path(buf);
    }
#endif
    
    // Final fallback
    return std::filesystem::current_path();
}

}
