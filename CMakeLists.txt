cmake_minimum_required(VERSION 3.28)
project(keyvalue)

set(CMAKE_CXX_STANDARD 17)

add_executable(keyvalue main.cpp
        hashmap.h
        hashmap.cpp)
