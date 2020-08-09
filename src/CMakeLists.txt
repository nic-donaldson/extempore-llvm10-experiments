cmake_minimum_required(VERSION 3.15)
project(extempore_llvm10_experiments)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(Main main.cpp EXTZONES.cpp EXTClosureAddressTable.cpp EXTLLVM.cpp)
set_property(TARGET Main PROPERTY CXX_STANDARD 17)

llvm_map_components_to_libnames(llvm_libs analysis core executionengine instcombine object orcjit runtimedyld scalaropts support native)
target_link_libraries(Main ${llvm_libs})

add_custom_command(
  TARGET Main
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/ir/ ${CMAKE_BINARY_DIR}/ir/
  )
