cmake_minimum_required(VERSION 3.16)
project(Cytos LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

if(MSVC)
    add_compile_options(/O2 /Oi /Gy /fp:fast /W3)
    add_link_options(/OPT:REF /OPT:ICF)
    add_definitions(_CRT_SECURE_NO_WARNINGS)
elseif(MINGW OR WIN32)
    add_compile_options(-O3 -march=native -ffast-math)
    add_link_options(-O3)
else()
    add_compile_options(-O3 -march=native -flto -ffast-math -Wno-register)
    add_link_options(-O3 -march=native -flto)
endif()

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)

set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
set(GLFW_INSTALL OFF CACHE INTERNAL "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

file(GLOB_RECURSE SOURCE_FILES 
    "src/*.cpp"
    "src/*.c"
)

file(GLOB IMGUI_FILES 
    "include/external/imgui/*.cpp"
    "include/external/imgui/backends/imgui_impl_glfw.cpp"
    "include/external/imgui/backends/imgui_impl_opengl3.cpp"
)

add_executable(Cytos ${SOURCE_FILES} ${IMGUI_FILES})

target_include_directories(Cytos PRIVATE 
    include
    include/external
    include/external/imgui
    include/external/imgui/backends
)

if(MINGW OR (WIN32 AND NOT MSVC))
    target_link_libraries(Cytos PRIVATE glfw opengl32 gdi32)
elseif(MSVC)
    target_link_libraries(Cytos PRIVATE glfw opengl32)
elseif(APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(Cytos PRIVATE glfw OpenGL::OpenGL)
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(Cytos PRIVATE glfw OpenGL::GL)
endif()