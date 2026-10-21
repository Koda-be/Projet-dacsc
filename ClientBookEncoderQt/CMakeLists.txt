cmake_minimum_required(VERSION 3.16)

project(ClientBookEncoderQt LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(ClientBookEncoderQt
    main.cpp
    mainwindowclientbookencoder.cpp
    mainwindowclientbookencoder.h
    ../sockets/sockets.c
)

target_include_directories(ClientBookEncoderQt PRIVATE
    ../headers
)

target_link_libraries(ClientBookEncoderQt PRIVATE Qt6::Widgets)
