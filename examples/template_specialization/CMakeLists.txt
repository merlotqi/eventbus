cmake_minimum_required(VERSION 3.16)
project(template_specialization_example)


add_executable(template_specialization main.cpp )

target_include_directories(template_specialization PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

find_package(Threads REQUIRED)
target_link_libraries(template_specialization Threads::Threads)
