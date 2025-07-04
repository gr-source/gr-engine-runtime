cmake_minimum_required(VERSION 3.22.1)

project(gr-engine VERSION 0.1.0 LANGUAGES C CXX HOMEPAGE_URL https://github.com/gr-source/gr-engine.git)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")

# Adicionando o executável principal

# Encontrando as bibliotecas necessárias
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    message(FATAL_ERROR "Please install libglfw3-dev")
    return()
endif()

find_package(GLEW QUIET)
if(NOT GLEW_FOUND)
    message(FATAL_ERROR "Please install libglew-dev")
    return()
endif()

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

add_executable(${PROJECT_NAME} src/main.cpp)

# Adicionando subdiretórios
add_subdirectory(engine)
add_subdirectory(imgui)

#### ENGINE ####
target_include_directories(engine PRIVATE imgui/src imgui/src/backends ${BULLET_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})

target_link_libraries(engine PRIVATE ${OPENGL_LIBRARIES} ${GRINI_LIBRARIES} ${GRMATH_LIBRARIES} ${GRRENDER_LIBRARIES} ${BULLET_LIBRARIES})


#### IMGUI ####
target_include_directories(imgui PRIVATE imgui/src imgui/src/backends)

#### MAIN ####

target_include_directories(${PROJECT_NAME} PRIVATE imgui/src imgui/src/backends engine/src)

target_link_libraries(${PROJECT_NAME} PRIVATE engine glfw imgui assimp ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${GRINI_LIBRARIES})

