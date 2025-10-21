cmake_minimum_required(VERSION 3.14)
project(Basilisk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add glad
add_library(glad STATIC include/glad/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# add stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)


# get other includes glm, glfw
include(FetchContent)
add_subdirectory(include)

# Collect all cpp files except entry points 
file(GLOB_RECURSE ENGINE_SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.tpp
)
list(REMOVE_ITEM ENGINE_SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# --- Make Example Executables ---
# Boilerplate Example
add_executable(basilisk main.cpp ${ENGINE_SRC})
target_include_directories(basilisk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(basilisk glad glfw glm assimp stb)
# --- Copy shaders ---
set(SHADERS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADERS_DST_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(GLOB_RECURSE SHADER_FILES "${SHADERS_SRC_DIR}/*")

add_custom_command( 
    OUTPUT ${SHADERS_DST_DIR}/.shader_copy_stamp
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADERS_SRC_DIR} ${SHADERS_DST_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${SHADERS_DST_DIR}/.shader_copy_stamp
    DEPENDS ${SHADER_FILES}
    COMMENT "Copying shaders to build directory"
)
add_custom_target(copy_shaders ALL DEPENDS ${SHADERS_DST_DIR}/.shader_copy_stamp)

# --- Copy textures ---
set(TEXTURES_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/textures")
set(TEXTURES_DST_DIR "${CMAKE_CURRENT_BINARY_DIR}/textures")
file(GLOB_RECURSE TEXTURE_FILES "${TEXTURES_SRC_DIR}/*")
add_custom_command(
    OUTPUT ${TEXTURES_DST_DIR}/.textures_copy_stamp
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${TEXTURES_SRC_DIR} ${TEXTURES_DST_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${TEXTURES_DST_DIR}/.textures_copy_stamp
    DEPENDS ${TEXTURE_FILES}
    COMMENT "Copying textures to build directory"
)
add_custom_target(copy_textures ALL DEPENDS ${TEXTURES_DST_DIR}/.textures_copy_stamp)

# --- Copy models ---
set(MODELS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/models")
set(MODELS_DST_DIR "${CMAKE_CURRENT_BINARY_DIR}/models")
file(GLOB_RECURSE MODEL_FILES "${MODELS_SRC_DIR}/*")
add_custom_command(
    OUTPUT ${MODELS_DST_DIR}/.models_copy_stamp
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${MODELS_SRC_DIR} ${MODELS_DST_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${MODELS_DST_DIR}/.models_copy_stamp
    DEPENDS ${MODEL_FILES}
    COMMENT "Copying model to build directory"
)
add_custom_target(copy_models ALL DEPENDS ${MODELS_DST_DIR}/.models_copy_stamp)
