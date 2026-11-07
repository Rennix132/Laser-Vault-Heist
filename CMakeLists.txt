cmake_minimum_required(VERSION 3.16)
project(LaserVaultHeist CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.3.9)
FetchContent_Declare(glm  GIT_REPOSITORY https://github.com/g-truc/glm.git  GIT_TAG 1.0.1)
FetchContent_MakeAvailable(glfw glm)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/Shader.cpp
    src/Mesh.cpp
    src/Camera.cpp
    external/glad/src/glad.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    external/glad/include
    external/stb
)

# Absolute path to the project root, used to find shaders/ and assets/
target_compile_definitions(${PROJECT_NAME} PRIVATE ASSET_DIR="${CMAKE_SOURCE_DIR}/")

target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm::glm)

# MSYS2/MinGW: link runtime statically so the .exe starts without extra DLLs
if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -static -static-libgcc -static-libstdc++)
endif()
