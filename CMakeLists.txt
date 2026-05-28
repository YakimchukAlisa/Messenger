cmake_minimum_required(VERSION 3.16)
project(Messenger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для автоматической загрузки библиотек
include(FetchContent)

# Находим pthread
find_package(Threads REQUIRED)

# Находим SQLite3
find_library(SQLITE3_LIBRARY sqlite3)
find_path(SQLITE3_INCLUDE_DIR sqlite3.h)

if(NOT SQLITE3_LIBRARY OR NOT SQLITE3_INCLUDE_DIR)
    message(WARNING "SQLite3 not found, installing...")
    execute_process(COMMAND sudo apt install libsqlite3-dev -y)
    find_library(SQLITE3_LIBRARY sqlite3)
    find_path(SQLITE3_INCLUDE_DIR sqlite3.h)
endif()

# ========== JSON БИБЛИОТЕКА ==========
find_package(nlohmann_json QUIET)

if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, downloading...")
    FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
    )
    FetchContent_GetProperties(json)
    if(NOT json_POPULATED)
        FetchContent_Populate(json)
        add_library(nlohmann_json INTERFACE)
        target_include_directories(nlohmann_json INTERFACE ${json_SOURCE_DIR})
    endif()
else()
    message(STATUS "nlohmann_json found via system package")
endif()

# ========== СЕРВЕР ==========
add_executable(messenger_server 
    server/src/main.cpp
    common/Database.h
)

target_include_directories(messenger_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${SQLITE3_INCLUDE_DIR}
)

target_link_libraries(messenger_server 
    Threads::Threads 
    ${SQLITE3_LIBRARY}
    nlohmann_json
)

# ========== КОНСОЛЬНЫЙ КЛИЕНТ ==========
add_executable(messenger_client 
    client/src/main.cpp
)

target_include_directories(messenger_client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${SQLITE3_INCLUDE_DIR}
)

target_link_libraries(messenger_client 
    Threads::Threads 
    ${SQLITE3_LIBRARY}
    nlohmann_json
)

# ========== QT GUI КЛИЕНТ ==========
find_package(Qt6 COMPONENTS Core Widgets Network QUIET)

if(Qt6_FOUND)
    message(STATUS "Qt6 found! Building GUI client...")
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
    add_subdirectory(qt_client)
else()
    message(STATUS "Qt6 not found. GUI client will not be built.")
    message(STATUS "Install Qt6: sudo apt install qt6-base-dev qt6-tools-dev")
endif()

# Выводим информацию о сборке
message(STATUS "SQLite3 include: ${SQLITE3_INCLUDE_DIR}")
message(STATUS "SQLite3 library: ${SQLITE3_LIBRARY}")
message(STATUS "JSON support: ${nlohmann_json_FOUND}")