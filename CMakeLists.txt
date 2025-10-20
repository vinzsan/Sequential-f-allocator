cmake_minimum_required(VERSION 3.10)

project(FoundMySelf)
#link_directories(${CMAKE_SOURCE_DIR})
add_executable(Test allocate.c)
#add_library(seqbrk STATIC allocate.c)
#target_link_libraries(Test PRIVATE seqbrk)
#set_target_properties(Test PROPERTIES
# BUILD_RPATH "${CMAKE_BINARY_DIR}"
#)
message("HEAP : test untuk brk\n")
#add_library(libshared.so PRIVATE main.cpp)
