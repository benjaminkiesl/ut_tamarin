cmake_minimum_required(VERSION 3.10.2)
project (uttamarin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# General options
include_directories(include third-party)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build UT Tamarin library
add_library(lib_uttamarin STATIC 
  src/app.cc
  src/tamarin_config.cc
  src/terminator.cc
  src/utility.cc
  src/bash_lemma_processor.cc
  src/verbose_lemma_processor.cc
  )
set_target_properties(lib_uttamarin PROPERTIES OUTPUT_NAME uttamarin)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(lib_uttamarin Threads::Threads)

# Build the executable for the main program
add_executable(ut_tamarin src/main.cc)
target_link_libraries(ut_tamarin lib_uttamarin)
