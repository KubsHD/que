cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_OSX_DEPLOYMENT_TARGET 11)
set(CPP_RTTI_ENABLED ON)

project(Que LANGUAGES CXX C)
include(FetchContent)

add_compile_definitions(JPH_DEBUG_RENDERER JPH_OBJECT_STREAM)


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
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy
        GIT_TAG master
)

FetchContent_MakeAvailable(tracy)

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

fetchcontent_declare(
        Cuttlefish
        GIT_REPOSITORY "https://github.com/akb825/Cuttlefish"
        GIT_TAG master
)

FetchContent_MakeAvailable(Cuttlefish)

add_subdirectory(deps/fmod)
add_subdirectory(game)