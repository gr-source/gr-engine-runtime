project(engine VERSION 0.1.0 LANGUAGES C CXX)


set(guizmo_src
    src/guizmo/ImGuizmo.cpp
)

set(guizmo_hdr
    src/guizmo/ImGuizmo.h
)

set(scriptManager_cpp
    src/scriptManager/scriptManager.cpp
    src/scriptManager/script.cpp
)

set(core_src
    src/core/statistics.cpp
    src/core/string.cpp
    src/core/screen.cpp
    src/core/time.cpp
    src/core/timer.cpp
)

set(core_hdr
    src/core/statistics.hpp
    src/core/string.hpp
    src/core/screen.hpp
    src/core/time.hpp
    src/core/timer.hpp
)

set(core_file_src
    src/core/file/initialization.cpp
    src/core/file/config.cpp
)

set(core_file_hdr
    src/core/file/initialization.hpp
    src/core/file/config.hpp
)

set(core_cmd_hdr
    src/core/cmd/cmd.hpp
)

set(core_cmd_src
    src/core/cmd/cmd.cpp
)

set(filmbox_src
    src/filmbox/fbxAnimation.cpp
    src/filmbox/fbxMaterial.cpp
    src/filmbox/fbxChannel.cpp
    src/filmbox/fbxVertex.cpp
    src/filmbox/fbxScene.cpp
    src/filmbox/fbxBone.cpp
    src/filmbox/fbxMesh.cpp
    src/filmbox/fbxNode.cpp
    src/filmbox/filmbox.cpp
)

set(task_src
    src/task/filmboxmanager.cpp
    src/task/filmboxtask.cpp
)

set(engine_src
    ${task_src}
    ${core_src}
    ${guizmo_src}
    ${filmbox_src}
    ${core_cmd_src}
    ${animation_src}
    ${core_file_src}
    ${simplexNoise_src}
    ${scriptManager_cpp}
    
    src/engine.cpp
    src/Global.cpp
    src/functionTimer.cpp
    src/settings.cpp
    src/screenRender.cpp
    src/shadowMap.cpp
    src/renderManager.cpp
    src/editorController.cpp

    src/ui/view.cpp
    src/ui/editor.cpp
    src/ui/project.cpp
    src/ui/ImGuiRender.cpp
    src/ui/projectMenu.cpp
    src/ui/shadereditor.cpp
)

set(engine_hdr
    ${core_hdr}
    ${guizmo_hdr}
    ${core_cmd_hdr}
    ${animation_hdr}
    ${core_file_hdr}
    ${scriptManager_hdr}

    src/engine.h
    src/Global.h
    src/settings.h
    src/screenRender.hpp
    src/renderManager.h
    src/editorController.h
    
    src/ui/view.hpp
    src/ui/editor.hpp
    src/ui/project.hpp
    src/ui/ImGuiRender.h
    src/ui/projectMenu.hpp
)

set(plugin
    src/plugin/pluginManager.cpp
    src/plugin/plugin.cpp
)

add_library(${PROJECT_NAME} STATIC ${engine_src} ${plugin} ${engine_hdr})

target_include_directories(${PROJECT_NAME} PUBLIC
    /home/felipy/Documentos/gr-projects/gr-base/include
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/src>
)

target_link_libraries(${PROJECT_NAME} PRIVATE /home/felipy/Documentos/gr-projects/gr-base/lib/libgr-base.so)

target_compile_definitions(${PROJECT_NAME} PUBLIC INTERNAL_REGISTER_SCRIPT)
