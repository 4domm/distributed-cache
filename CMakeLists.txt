cmake_minimum_required(VERSION 3.28)
project(keyvalue C)

set(CMAKE_C_STANDARD 11)

add_executable(keyvalue main.c
        hashmap.c)
