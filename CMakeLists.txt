cmake_minimum_required(VERSION 3.16)
project(Messenger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(messenger_server server/src/main.cpp)
target_include_directories(messenger_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(messenger_client client/src/main.cpp)
target_include_directories(messenger_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(messenger_server Threads::Threads)
target_link_libraries(messenger_client Threads::Threads)