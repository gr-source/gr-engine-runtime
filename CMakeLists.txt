cmake_minimum_required(VERSION 3.22.1)

project("gr-engine-runtime" VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED GR_ENGINE_CORE)
    message(FATAL_ERROR "GR_ENGINE_CORE NÃO foi definida!")
endif()

if(NOT GR_ENGINE_CORE)
    message(FATAL_ERROR "A variável GR_ENGINE_CORE está definida, mas está vazia.")
endif()

message(STATUS "GR_ENGINE_CORE configurada com sucesso: ${GR_ENGINE_CORE}")

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

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${BULLET_INCLUDE_DIRS}
    ${GR_ENGINE_CORE}/include
    ${GR_ENGINE_CORE}/imgui
)

add_library(gr-base SHARED IMPORTED)
set_target_properties(gr-base PROPERTIES IMPORTED_LOCATION "${GR_ENGINE_CORE}/lib/libgr-base.so")

add_library(imgui SHARED IMPORTED)
set_target_properties(imgui PROPERTIES IMPORTED_LOCATION "${GR_ENGINE_CORE}/lib/libimgui.so")

target_link_libraries(${PROJECT_NAME} PUBLIC
    ${OPENGL_LIBRARIES}
    ${GRINI_LIBRARIES}
    ${GRMATH_LIBRARIES}
    ${GRRENDER_LIBRARIES}
    ${BULLET_LIBRARIES}
    ${GLEW_LIBRARIES}
    gr-base
    imgui
    glfw
    assimp
)

add_subdirectory(tests)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )

    add_link_options(
        -fsanitize=address,undefined
    )
endif()


