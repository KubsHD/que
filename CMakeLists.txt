cmake_minimum_required(VERSION 3.30)

project(Que LANGUAGES CXX C)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)

find_package(Vulkan REQUIRED)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG master
)

FetchContent_MakeAvailable(glm)

FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt
        GIT_TAG master
)

FetchContent_MakeAvailable(entt)

FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics"
        GIT_TAG master
        SOURCE_SUBDIR "Build"
)
FetchContent_MakeAvailable(JoltPhysics)

FetchContent_Declare(
        OpenXR
        GIT_REPOSITORY "https://github.com/KhronosGroup/OpenXR-SDK"
        GIT_TAG main
)

FetchContent_MakeAvailable(OpenXR)

FetchContent_Declare(
        SDL
        GIT_REPOSITORY "https://github.com/libsdl-org/SDL"
        GIT_TAG main
)

FetchContent_MakeAvailable(SDL)

FetchContent_Declare(
        assimp
        GIT_REPOSITORY "https://github.com/assimp/assimp"
        GIT_TAG master
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(assimp)

add_subdirectory(deps/fmod)
add_subdirectory(game)