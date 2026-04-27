cmake_minimum_required(VERSION 3.22.1)

project(gr-engine VERSION 0.1.0 LANGUAGES C CXX HOMEPAGE_URL https://github.com/gr-source/gr-engine.git)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(GR_BASE_PATH "/home/NoirVoid/AppProjects/gr-base")

find_package(OpenGL     REQUIRED)
find_package(gr-math    REQUIRED)
find_package(gr-render  REQUIRED)
find_package(gr-ini     REQUIRED)

find_package(Bullet QUIET)
if(NOT Bullet_FOUND)
    message(FATAL_ERROR "Please install libbullet-dev")
    return()
endif()

find_package(assimp QUIET)
if(NOT assimp_FOUND)
    message(FATAL_ERROR "Please install libassimp-dev")
    return()
endif()

find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    message(FATAL_ERROR "Please install libglfw3-dev")
    return()
endif()

find_package(GLEW QUIET)
if(NOT GLEW_FOUND)
    message(FATAL_ERROR "Please install GLEW.")
    return()
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

add_subdirectory(imgui)
add_subdirectory(tests)

add_subdirectory(${GR_BASE_PATH} gr-base)

include_directories(include)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES} ${GRINI_LIBRARIES} ${GRMATH_LIBRARIES} ${GRRENDER_LIBRARIES} ${BULLET_LIBRARIES} ${GLEW_LIBRARIES} gr-base imgui glfw assimp)


