cmake_minimum_required(VERSION 3.5.0)

project(${GR_PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(sourc_src
    script/gr-script.cpp
    #{GR_START}
    #{GR_END}
)

add_library(${GR_PROJECT_NAME} SHARED ${sourc_src} ${GR_LIBRARIES})

target_include_directories(${GR_PROJECT_NAME} PUBLIC ${GR_DIRECTORY})


