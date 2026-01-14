#ifndef BSK_RESOLVE_PATH_H
#define BSK_RESOLVE_PATH_H

#include <string>
#include <filesystem>
#ifdef _WIN32
    #include <direct.h>
#else
    #include <unistd.h>
#endif

namespace bsk::internal {

// -----------------------------------------------------------------------------
// Get the module path (where the .so file is located)
// Non-inline function defined in resolvePath.cpp
// -----------------------------------------------------------------------------
std::filesystem::path getModulePath();

// -----------------------------------------------------------------------------
// Set the Python working directory (called from Python module initialization)
// Non-inline function defined in resolvePath.cpp
// -----------------------------------------------------------------------------
void setPythonWorkingDirectory(const std::string& path);

// -----------------------------------------------------------------------------
// Get the current working directory
// Uses Python's working directory if set, otherwise falls back to getcwd()
// Non-inline function defined in resolvePath.cpp
// -----------------------------------------------------------------------------
std::filesystem::path getCurrentWorkingDirectory();

// -----------------------------------------------------------------------------
// Resolve a user path relative to the current working directory
// Used for files provided by the user (textures, models, etc.)
// When running from Python, files are resolved relative to where Python is executed
// -----------------------------------------------------------------------------
inline std::string externalPath(const std::string& relativePath) {
    return (getCurrentWorkingDirectory() / relativePath).string();
}

// -----------------------------------------------------------------------------
// Resolve an internal package path relative to the module location
// Used for files that are part of the package (shaders, etc.)
// These files should be installed next to the .so file
// -----------------------------------------------------------------------------
inline std::string internalPath(const std::string& relativePath) {
    return (getModulePath() / relativePath).string();
}

} // namespace bsk::internal

#endif
