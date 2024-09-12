cmake_minimum_required(VERSION 3.22.1)

project(gr-engine VERSION 0.1.0 LANGUAGES C CXX HOMEPAGE_URL https://github.com/gr-source/gr-engine.git)

# Definindo diretórios de saída
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

#defina a versão do c++
set(CMAKE_CXX_STANDARD 17)

# Set the CMAKE_CXX_FLAGS to include profiling flags
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")

# Optional: Set the linker flags if needed
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    message(WARNING "CMAKE_CXX_STANDARD não definido. Usando C++17 como padrão.")
endif()

# Adicionando o executável principal
add_executable(${PROJECT_NAME} src/main.cpp)

# Encontrando as bibliotecas necessárias
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(gr-math REQUIRED)
find_package(gr-render REQUIRED)
find_package(Bullet REQUIRED)

# Adicionando subdiretórios
add_subdirectory(engine)
add_subdirectory(imgui)

#### MAIN ####

target_include_directories(${PROJECT_NAME} PRIVATE imgui/src imgui/src/backends engine/src)

target_link_libraries(${PROJECT_NAME} PRIVATE engine glfw imgui assimp ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})

#### ENGINE ####

target_include_directories(engine PRIVATE imgui/src imgui/src/backends ${BULLET_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE engine glfw imgui assimp ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})

#### ENGINE ####

target_include_directories(engine PRIVATE imgui/src imgui/src/backends ${BULLET_INCLUDE_DIRS})

target_link_libraries(engine PRIVATE ${OPENGL_LIBRARIES} ${GRMATH_LIBRARIES} ${GRRENDER_LIBRARIES} ${BULLET_LIBRARIES})

target_compile_definitions(engine PRIVATE IS_GAME=0 DEBUG=1)


#### IMGUI ####

target_include_directories(imgui PRIVATE imgui/src imgui/src/backends)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

