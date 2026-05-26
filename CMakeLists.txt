cmake_minimum_required(VERSION 3.16)
project(Messenger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим pthread
find_package(Threads REQUIRED)

# Находим SQLite3 старым способом (работает везде)
find_library(SQLITE3_LIBRARY sqlite3)
find_path(SQLITE3_INCLUDE_DIR sqlite3.h)

if(NOT SQLITE3_LIBRARY OR NOT SQLITE3_INCLUDE_DIR)
    message(WARNING "SQLite3 not found, installing...")
    execute_process(COMMAND sudo apt install libsqlite3-dev -y)
    find_library(SQLITE3_LIBRARY sqlite3)
    find_path(SQLITE3_INCLUDE_DIR sqlite3.h)
endif()

# Сервер
add_executable(messenger_server 
    server/src/main.cpp
)

# Клиент
add_executable(messenger_client 
    client/src/main.cpp
)

# Подключаем библиотеки
target_include_directories(messenger_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${SQLITE3_INCLUDE_DIR})
target_include_directories(messenger_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${SQLITE3_INCLUDE_DIR})

target_link_libraries(messenger_server Threads::Threads ${SQLITE3_LIBRARY})
target_link_libraries(messenger_client Threads::Threads ${SQLITE3_LIBRARY})


# QT GUI КЛИЕНТ 
find_package(Qt6 COMPONENTS Core Widgets Network QUIET)

if(Qt6_FOUND)
    message(STATUS "Qt6 found! Building GUI client...")
    
    # Находим Qt6 пакеты
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
    
    # Подключаем qt_client поддиректорию
    add_subdirectory(qt_client)
else()
    message(STATUS "Qt6 not found. GUI client will not be built.")
    message(STATUS "Install Qt6: sudo apt install qt6-base-dev qt6-tools-dev")
endif()


# Выводим информацию о сборке
message(STATUS "SQLite3 include: ${SQLITE3_INCLUDE_DIR}")
message(STATUS "SQLite3 library: ${SQLITE3_LIBRARY}")