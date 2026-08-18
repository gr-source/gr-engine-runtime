cmake_minimum_required(VERSION 4.2.1)

project("gr-script" VERSION 0.1.0 LANGUAGES C CXX)

if(NOT DEFINED GR_ENGINE_CORE)
    message(FATAL_ERROR "GR_ENGINE_CORE NÃO foi definida!")
    return()
endif()

if(NOT GR_ENGINE_CORE)
    message(FATAL_ERROR "A variável GR_ENGINE_CORE está definida, mas está vazia.")
    return()
endif()

message(STATUS "GR_ENGINE_CORE configurada com sucesso: ${GR_ENGINE_CORE}")

find_package(Bullet QUIET)
if(NOT Bullet_FOUND)
    message(FATAL_ERROR "Please install libbullet-dev")
    return()
endif()

# set(PLUGIN_PATH "/home/NoirVoid/AppProjects/gr-build-plugin/build/gr-plugin.so")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-gnu-unique")
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

# set(HEADER_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/header.cpp)

# add_custom_command(
#     OUTPUT ${HEADER_OUTPUT}
#     COMMAND clang
#         -fplugin=${PLUGIN_PATH}
#         -I${GR_ENGINE_CORE}/include
#         -fsyntax-only
#         -Xclang -plugin-arg-meu-plugin
#         -Xclang ${HEADER_OUTPUT}
#         ${SOURCES}
#     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#     DEPENDS ${PLUGIN_PATH} ${SOURCES}
#     COMMENT "Executando plugin Clang para gerar header.cpp"
#     VERBATIM
#     BYPRODUCTS ${HEADER_OUTPUT}
# )
#
# add_custom_target(GenerateHeader DEPENDS ${HEADER_OUTPUT})

# list(APPEND SOURCES ${HEADER_OUTPUT})

add_library(${CMAKE_PROJECT_NAME} SHARED main.cpp ${SOURCES})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    ${GR_ENGINE_CORE}/lib/libgr-base.so
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${CMAKE_PROJECT_NAME}>
    ${GR_ENGINE_CORE}/include
    ${GR_ENGINE_CORE}/imgui
    ${BULLET_INCLUDE_DIRS}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )

    add_link_options(
        -fsanitize=address,undefined
    )
endif()

# target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC DSCRIPT_BUILD)

# add_dependencies(${CMAKE_PROJECT_NAME} GenerateHeader)
#
# add_custom_command(
#     TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E remove ${HEADER_OUTPUT} "${HEADER_OUTPUT}.o"
#     COMMENT "Removendo header temporário"
# )


