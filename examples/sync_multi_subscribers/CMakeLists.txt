cmake_minimum_required(VERSION 3.16)
project(sync_multi_subscribers_example)


add_executable(sync_multi_subscribers main.cpp 
    ../sync_eventbus_pointer_singleton.cpp
    ../sync_eventbus_pointer_singleton.h)

target_include_directories(sync_multi_subscribers PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

find_package(Threads REQUIRED)
target_link_libraries(sync_multi_subscribers Threads::Threads)
