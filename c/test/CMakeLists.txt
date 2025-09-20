cmake_minimum_required(VERSION 3.10)
project(MyGTestProject LANGUAGES CXX)

# Enable testing
enable_testing()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Download and add GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add your test source files here
add_executable(my_tests
    test_main.cpp
    test_model_gen.cpp)

target_include_directories(my_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/ ${CMAKE_CURRENT_BINARY_DIR}/lib)

# Link GoogleTest libraries
target_link_libraries(my_tests gtest gtest_main kea)

# Add test to CTest
add_test(NAME MyTests COMMAND my_tests)