cmake_minimum_required(VERSION 3.28)
project(distributed-cache)

find_package(Poco REQUIRED COMPONENTS Net JSON Util Foundation)

add_executable(distributed-cache main.cpp
        hashmap.cpp
        network.cpp
)

target_link_libraries(distributed-cache
        Poco::Net
        Poco::JSON
        Poco::Util
        Poco::Foundation
)