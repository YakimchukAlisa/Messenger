cmake_minimum_required(VERSION 3.16)
project(QtMessengerClient)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MessengerClient.cpp
)

set(HEADERS
    src/MainWindow.h
    src/MessengerClient.h
)

add_executable(qt_messenger_client ${SOURCES} ${HEADERS})

target_include_directories(qt_messenger_client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

target_link_libraries(qt_messenger_client
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Threads::Threads
)

# Подключаем SQLite
find_library(SQLITE3_LIBRARY sqlite3)
target_link_libraries(qt_messenger_client ${SQLITE3_LIBRARY})
