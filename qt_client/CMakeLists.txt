cmake_minimum_required(VERSION 3.16)
project(QtMessengerClient)

# Находим Qt6 компоненты
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Включаем автоматическую обработку MOC
qt_standard_project_setup()

# Исходные файлы
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MessengerClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/Database.h
)

set(HEADERS
    src/MainWindow.h
    src/MessengerClient.h
)

# Ресурсы (CSS файл) — опционально, пока закомментируем
# set(RESOURCES
#     style.css
# )
# qt6_add_resources(QT_MESSENGER_RESOURCES ${RESOURCES})

# Создаём исполняемый файл (только один раз!)
add_executable(qt_messenger_client ${SOURCES} ${HEADERS})  # ← убрал ${QT_MESSENGER_RESOURCES}

# Добавляем пути для include
target_include_directories(qt_messenger_client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

# Линкуем библиотеки
target_link_libraries(qt_messenger_client
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Threads::Threads
    ${SQLITE3_LIBRARY}  # SQLite уже найден в корневом CMakeLists.txt
)

