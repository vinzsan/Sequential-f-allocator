cmake_minimum_required(VERSION 3.10)

project(FoundMySelf)
add_executable(Test allocate.c)
target_link_libraries(Test PRIVATE m)
message("HEAP : test untuk brk\n")
#add_library(libshared.so PRIVATE main.cpp)
