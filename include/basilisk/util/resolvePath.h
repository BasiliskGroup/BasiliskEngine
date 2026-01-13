#ifndef BSK_RESOLVE_PATH_H
#define BSK_RESOLVE_PATH_H

#include <string>
#include <filesystem>

namespace bsk::internal {

inline std::string resolveBasiliskPath(const std::string& relativePath) {
    std::filesystem::path basiliskPath = std::filesystem::current_path() / "..";
    return (basiliskPath / relativePath).string();
}

}

#endif

