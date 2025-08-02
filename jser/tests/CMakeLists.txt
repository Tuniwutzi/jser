add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../deps/Catch2 catch2)

add_executable(jser_tests
    Deserialization.cpp
)
target_link_libraries(jser_tests PUBLIC Catch2::Catch2WithMain jser)