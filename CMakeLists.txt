cmake_minimum_required(VERSION 3.16)
project(OS_IHW2_3 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(OS_IHW2_3
        mark_7/clients.c
        mark_7/sellers.c
        utils.h)

target_link_libraries(OS_IHW2_3 rt pthread)
