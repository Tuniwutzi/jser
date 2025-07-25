cmake_minimum_required(VERSION 3.30.0)

project(jscheme LANGUAGES CXX)

add_library(
    jser
    INTERFACE
)
target_include_directories(
    jser
    INTERFACE
    jser/src
)
target_compile_features(
    jser
    INTERFACE
    cxx_std_26
)

set(path "/Users/jbuenker/dev/private/clang-p2996/install")
target_compile_options(
    jser
    INTERFACE
    "-freflection-latest"
    "-Wno-c23-extensions"

    # Options to make linking work

    "-stdlib=libc++"
    # We don't seem to need these, clang chooses the right includes
    # "-nostdinc++"
    # "-I${path}/include/c++/v1"
)
target_link_options(
    jser
    INTERFACE
    "-stdlib=libc++"
    # We do need these, because otherwise the system version of libc++ is used (which doesn't include std::print)
    "-L${path}/lib"
    "-Wl,-rpath,${path}/lib"
)

add_executable(
    jser_example
    jser/example/main.cpp
)

target_link_libraries(
    jser_example
    PUBLIC
    jser
)
