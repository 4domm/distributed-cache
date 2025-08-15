cmake_minimum_required(VERSION 3.28)
project(timkv)

find_package(Poco REQUIRED COMPONENTS Net JSON Util Foundation)
set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g -O1")
#set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread")
add_executable(timkv src/main.cpp
        src/network.cpp
        src/network.h
)

target_link_libraries(timkv
        Poco::Net
        Poco::JSON
        Poco::Util
        Poco::Foundation
)