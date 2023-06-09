cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
)
FetchContent_Declare(
  gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG main
)

FetchContent_MakeAvailable(fmt)
FetchContent_MakeAvailable(gtest)

project(
  bee
  DESCRIPTION "bee programming language interpreter"
  LANGUAGES CXX
)

project(
  bee-cmd
  LANGUAGES CXX
)

project(
  bee-test
  LANGUAGES CXX
)

add_subdirectory(src/bee)
add_subdirectory(src/cmd)
add_subdirectory(src/test)
