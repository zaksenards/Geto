cmake_minimum_required(VERSION 3.0.1)
project(Geto)

set(GETO_SOURCE_PATH ${CMAKE_SOURCE_DIR}/src/)
set(GETO_HEADER_PATH ${CMAKE_SOURCE_DIR}/include)
set(GETO_EXEMPLE_SOURCES_PATH ${CMAKE_SOURCE_DIR}/exemples)
set(CMAKE_CXX_STANDARD 11)

set(GETO_LIBRARY_SOURCES ${GETO_SOURCE_PATH}/platform.cpp)

add_library(Geto SHARED ${GETO_LIBRARY_SOURCES})
target_include_directories(Geto PUBLIC ${GETO_HEADER_PATH})
add_compile_definitions(Geto PRIVATE PLATFORM_IMPLEMENTATION)
add_compile_definitions(Geto PRIVATE _CRT_SECURE_NO_WARNINGS)
add_compile_definitions(Geto PRIVATE DEBUG_BUILD)
add_compile_definitions(-Wall -Werror)

add_executable(simpleWindow ${GETO_EXEMPLE_SOURCES_PATH}/simpleWindow.cpp)
add_executable(keyboard ${GETO_EXEMPLE_SOURCES_PATH}/keyboard.cpp)
target_link_libraries(simpleWindow Geto)
target_link_libraries(keyboard Geto)
