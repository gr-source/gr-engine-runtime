cmake_minimum_required(VERSION 4.2.1)

project(script VERSION 0.1.0 LANGUAGES C CXX)

set(GR_BASE_PATH "/home/NoirVoid/AppProjects/gr-base")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-gnu-unique")

file(GLOB SRC CONFIGURE_DEPENDS *.cpp)

add_library(${CMAKE_PROJECT_NAME} SHARED ${SRC})

add_subdirectory(${GR_BASE_PATH} gr-base)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE gr-base)

