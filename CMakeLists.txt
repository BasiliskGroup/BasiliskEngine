cmake_minimum_required(VERSION 3.14)
project(Basilisk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add glad
add_library(glad STATIC include/glad/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# add stb
add_library(stb INTERFACE include/stb/stb_image.h)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# get other includes glm, glfw
include(FetchContent)
add_subdirectory(include)

# Search recursively for all .cpp files in src/
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Then use that list to build the executable
add_executable(render ${SRC_FILES})

target_include_directories(render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(render glad glfw glm assimp stb)

# Set source and destination directories
set(SHADERS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADERS_DST_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
# Get all shader files
file(GLOB_RECURSE SHADER_FILES
    "${SHADERS_SRC_DIR}/*"
)
# Custom command to copy shaders
add_custom_command(
    OUTPUT ${SHADERS_DST_DIR}/.shader_copy_stamp
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADERS_SRC_DIR} ${SHADERS_DST_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${SHADERS_DST_DIR}/.shader_copy_stamp
    DEPENDS ${SHADER_FILES}
    COMMENT "Copying shaders to build directory"
)
# Custom target that depends on the above command
add_custom_target(copy_shaders ALL
    DEPENDS ${SHADERS_DST_DIR}/.shader_copy_stamp
)


# Set source and destination directories
set(TEXTURES_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/textures")
set(TEXTURES_DST_DIR "${CMAKE_CURRENT_BINARY_DIR}/textures")
# Get all texture files
file(GLOB_RECURSE TEXTURE_FILES
    "${TEXTURES_SRC_DIR}/*"
)
# Custom command to copy textures
add_custom_command(
    OUTPUT ${TEXTURES_DST_DIR}/.textures_copy_stamp
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${TEXTURES_SRC_DIR} ${TEXTURES_DST_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${TEXTURES_DST_DIR}/.textures_copy_stamp
    DEPENDS ${TEXTURE_FILES}
    COMMENT "Copying textures to build directory"
)
# Custom target that depends on the above command
add_custom_target(copy_textures ALL
    DEPENDS ${TEXTURES_DST_DIR}/.textures_copy_stamp
)


# Set source and destination directories
set(MODELS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/models")
set(MODELS_DST_DIR "${CMAKE_CURRENT_BINARY_DIR}/models")
# Get all texture files
file(GLOB_RECURSE MODEL_FILES
    "${MODELS_SRC_DIR}/*"
)

# Custom command to copy textures
add_custom_command(
    OUTPUT ${MODELS_DST_DIR}/.models_copy_stamp
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${MODELS_SRC_DIR} ${MODELS_DST_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${MODELS_DST_DIR}/.models_copy_stamp
    DEPENDS ${MODEL_FILES}
    COMMENT "Copying model to build directory"
)
# Custom target that depends on the above command
add_custom_target(copy_models ALL
    DEPENDS ${MODELS_DST_DIR}/.models_copy_stamp
)