file(
  GLOB_RECURSE BEE_SOURCE
  "[a-z0-9]" *.hpp
  "[a-z0-9]" *.cpp
)

add_library(
  bee STATIC
  ${BEE_SOURCE}
)

target_include_directories(
  bee PRIVATE
  ${CMAKE_SOURCE_DIR}/src/bee
)

target_link_libraries(
  bee PUBLIC
  fmt::fmt
)

set_target_properties(
  bee PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED YES
  LINKER_LANGUAGE CXX
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_compile_options(
  bee PUBLIC
  -Wno-conversion-null
)
