cmake_minimum_required(VERSION 3.22)
project(tcpip C)

set(CMAKE_C_STANDARD 11)

add_executable(tcpip
        tcpserver.c)
