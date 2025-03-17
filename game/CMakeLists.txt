cmake_minimum_required(VERSION 3.30)

project(game LANGUAGES C CXX)


# Define the source directory
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Find all .cpp and .h files recursively
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.c")
file(GLOB_RECURSE HEADERS "${SRC_DIR}/*.h")

# Add executable or library
add_executable(Game ${SOURCES} ${HEADERS})

target_compile_options(Game PUBLIC -fms-extensions)

target_include_directories(Game PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(Game PUBLIC "${FETCHCONTENT_BASE_DIR}/assimp-src/include")
target_include_directories(Game PUBLIC "${FETCHCONTENT_BASE_DIR}/tracy-src/public")

target_compile_definitions(Game PUBLIC JPH_DEBUG_RENDERER JPH_OBJECT_STREAM)

target_precompile_headers(Game PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h>)

target_link_libraries(Game PRIVATE
        Vulkan::Vulkan glm::glm-header-only EnTT::EnTT OpenXR::headers OpenXR::openxr_loader Jolt::Jolt
        SDL3-static
        fmod
        assimp
        cuttlefish_lib
)
