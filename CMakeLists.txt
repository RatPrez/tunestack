cmake_minimum_required(VERSION 4.0)
project(tunestack)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG master
)
FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_Declare(taglib
    GIT_REPOSITORY https://github.com/taglib/taglib.git
    GIT_TAG master
)

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(imgui stb taglib)

find_package(SDL3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui_lib PUBLIC SDL3)

add_executable(tunestack ${SOURCES})
target_compile_features(tunestack PUBLIC cxx_std_20)

target_include_directories(tunestack PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_include_directories(tunestack PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${taglib_SOURCE_DIR}/taglib
    ${taglib_SOURCE_DIR}/taglib/toolkit
    ${taglib_SOURCE_DIR}/taglib/mpeg
    ${taglib_SOURCE_DIR}/taglib/mpeg/id3v2
    ${taglib_SOURCE_DIR}/taglib/mpeg/id3v2/frames
    ${taglib_SOURCE_DIR}/taglib/mpeg/id3v1
    ${taglib_SOURCE_DIR}/taglib/ogg
    ${taglib_SOURCE_DIR}/taglib/flac
    ${taglib_BINARY_DIR}
)

target_link_libraries(tunestack PRIVATE
    SDL3
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    imgui_lib
    tag
)
