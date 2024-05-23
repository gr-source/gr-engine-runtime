cmake_minimum_required(VERSION 3.22.1)

project(gr-engine VERSION 0.1.0 LANGUAGES C CXX HOMEPAGE_URL https://github.com/gr-source/gr-engine.git)

# Definindo diretórios de saída
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

#defina a versão do c++
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    message(WARNING "CMAKE_CXX_STANDARD não definido. Usando C++17 como padrão.")
endif()

# Adicionando o executável principal
add_executable(${PROJECT_NAME} src/main.cpp)

# Encontrando as bibliotecas necessárias
find_package(glfw3 REQUIRED)
if(NOT glfw3_FOUND)
    message(FATAL_ERROR "GLFW não encontrado. Instale GLFW para compilar este projeto.")
endif()

find_package(GLEW REQUIRED)
if(NOT GLEW_FOUND)
message(FATAL_ERROR "GLEW não encontrado. Instale GLEW para compilar este projeto.")
endif()

find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
    message(FATAL_ERROR "OpenGL não encontrado. Verifique a instalação do OpenGL.")
endif()

find_package(gr-math REQUIRED)
if(NOT gr-math_FOUND)
    message(FATAL_ERROR "gr-math não encontrado. Verifique a instalação do gr-math.")
endif()

find_package(Bullet REQUIRED)
if(NOT BULLET_FOUND)
    message(FATAL_ERROR "Bullet não encontrado. Verifique a instalação do Bullet.")
endif()

# Adicionando subdiretórios
add_subdirectory(engine)
add_subdirectory(imgui)

#### MAIN ####

target_include_directories(${PROJECT_NAME} PRIVATE imgui/src imgui/src/backends engine/src)

target_link_libraries(${PROJECT_NAME} PRIVATE engine glfw imgui assimp ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})

#### ENGINE ####

target_include_directories(engine PRIVATE imgui/src imgui/src/backends ${BULLET_INCLUDE_DIRS})

target_link_libraries(engine PRIVATE ${OPENGL_LIBRARIES} ${GRMATH_LIBRARIES} ${BULLET_LIBRARIES})

target_compile_definitions(engine PRIVATE IS_GAME=0 OPENGL_DEBUG=1)


#### IMGUI ####

target_include_directories(imgui PRIVATE imgui/src imgui/src/backends)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

