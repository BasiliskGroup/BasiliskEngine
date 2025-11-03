include(FetchContent)

# GLFW - Check if already provided
if(NOT TARGET glfw)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if(NOT DEFINED GLFW_BUILD_WAYLAND)
            set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build GLFW for Wayland")
        endif()
        if(NOT DEFINED GLFW_BUILD_X11)
            set(GLFW_BUILD_X11 ON CACHE BOOL "Build GLFW for X11")
        endif()
    endif()

    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
    )
    FetchContent_MakeAvailable(glfw)
endif()

# GLM - Check if already provided
if(NOT TARGET glm)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
    )
    FetchContent_MakeAvailable(glm)
endif()

# ASSIMP - Check if already provided
if(NOT TARGET assimp)
    FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG        v5.4.3
    )
    FetchContent_MakeAvailable(assimp)
endif()