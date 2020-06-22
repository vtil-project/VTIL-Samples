cmake_minimum_required(VERSION 3.15)

project(VTIL-Samples)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

include(FetchContent)

FetchContent_Declare(
    VTIL-Core
    GIT_REPOSITORY https://github.com/vtil-project/VTIL-Core
    GIT_SHALLOW    true
)
FetchContent_MakeAvailable(VTIL-Core)

add_subdirectory(Logger)
add_subdirectory(Intro)
add_subdirectory(Simplification)