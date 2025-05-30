cmake_minimum_required(VERSION 3.28)
project(distributed-cache)

find_package(Poco REQUIRED COMPONENTS Net JSON Util Foundation)
set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g -O1")
#set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread")
add_executable(distributed-cache main.cpp
        network.cpp
        storage.h
        lru_cache.h
        lfu_cache.h
        arc_cache.h
        twoq_cache.h
)

add_executable(bench_kv
        benchlru.cpp
        lru_cache.h
        storage.h
)

target_link_libraries(distributed-cache
        Poco::Net
        Poco::JSON
        Poco::Util
        Poco::Foundation
)