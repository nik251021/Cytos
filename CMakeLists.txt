cmake_minimum_required(VERSION 3.16)
project(Cytos LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

if(MSVC)
    add_compile_options(/O2 /Oi /Gy /fp:fast)
    add_link_options(/OPT:REF /OPT:ICF)
else()
    add_compile_options(-O3 -march=native -flto -ffast-math)
    add_link_options(-O3 -march=native -flto)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

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

find_package(OpenGL REQUIRED)

file(GLOB_RECURSE SOURCE_FILES 
    "src/*.cpp"
    "src/*.c"
)

add_executable(Cytos ${SOURCE_FILES})

target_include_directories(Cytos PRIVATE 
    include
    include/external
)

target_link_libraries(Cytos PRIVATE glfw OpenGL::GL)