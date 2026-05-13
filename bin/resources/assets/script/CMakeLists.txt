cmake_minimum_required(VERSION 4.2.1)
project(script VERSION 0.1.0 LANGUAGES C CXX)

set(GR_BASE_PATH "/home/NoirVoid/AppProjects/gr-base")
set(PLUGIN_PATH "/home/NoirVoid/AppProjects/gr-build-plugin/build/gr-plugin.so")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-gnu-unique")
endif()

file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)

set(HEADER_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/header.cpp)

add_custom_command(
    OUTPUT ${HEADER_OUTPUT}
    COMMAND clang
        -fplugin=${PLUGIN_PATH}
        -I${GR_BASE_PATH}/include
        -fsyntax-only
        -Xclang -plugin-arg-meu-plugin
        -Xclang ${HEADER_OUTPUT}
        ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${PLUGIN_PATH} ${SOURCES}
    COMMENT "Executando plugin Clang para gerar header.cpp"
    VERBATIM
    BYPRODUCTS ${HEADER_OUTPUT}
)

add_custom_target(GenerateHeader DEPENDS ${HEADER_OUTPUT})

list(APPEND SOURCES ${HEADER_OUTPUT})

add_library(${CMAKE_PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE "${GR_BASE_PATH}/lib/libgr-base.so")
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${GR_BASE_PATH}/include")

# Garantir que o header seja gerado antes da compilação da biblioteca
add_dependencies(${CMAKE_PROJECT_NAME} GenerateHeader)

add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove ${HEADER_OUTPUT} "${HEADER_OUTPUT}.o"
    COMMENT "Removendo header temporário"
)


