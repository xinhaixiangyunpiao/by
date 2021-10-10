cmake_minimum_required(VERSION 3.5)

project(main)

include_directories("./include")

add_executable(main ./src/context.cpp ./src/strategy.cpp ./src/scheduler.cpp ./src/util.cpp ./src/controller.cpp ./src/webServer.cpp)

target_link_libraries(main libjsoncpp.a)