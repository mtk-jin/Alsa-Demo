# CMake version minimum required.
cmake_minimum_required (VERSION 2.8)

SET(PRJ_NAME Demo)

# Project name, change it you need.
project (${PRJ_NAME})

# Combine source files to DIR_SRCS in current forld.
aux_source_directory(. DIR_SRCS)

#add_subdirectory(include)

#include_directories(./includ)

# Specify name to be build.
add_executable(${PRJ_NAME} ${DIR_SRCS})

# Link libs
target_link_libraries(${PRJ_NAME} asound)
target_link_libraries(${PRJ_NAME} m)
target_link_libraries(${PRJ_NAME} pthread)
