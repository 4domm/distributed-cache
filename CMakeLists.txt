cmake_minimum_required(VERSION 3.28)
project(distributed-cache)

find_package(Poco REQUIRED COMPONENTS Net JSON Util Foundation)

add_executable(distributed-cache src/main.cpp
        src/hashmap.h
        src/hashmap.cpp
        src/storage.cpp
        src/network.cpp
        src/storage.h
        src/network.h)
target_link_libraries(distributed-cache
        Poco::Net
        Poco::JSON
        Poco::Util
        Poco::Foundation
)