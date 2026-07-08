project(gr-test VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${GR_ENGINE_CORE}/include
)

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


