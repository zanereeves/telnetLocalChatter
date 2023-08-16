cmake_minimum_required(VERSION 3.26)
project(telnetChat C)

set(CMAKE_C_STANDARD 23)

add_library(telnetChat SHARED Server.c
            Server.h)



add_executable(run chat.c)

target_link_libraries(run telnetChat)