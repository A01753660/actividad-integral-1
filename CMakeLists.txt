cmake_minimum_required(VERSION 3.16)
project(actividad_integral_1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(actividad_integral_1 main.cpp)


configure_file(equipo1.csv ${CMAKE_CURRENT_BINARY_DIR}/equipo1.csv COPYONLY)