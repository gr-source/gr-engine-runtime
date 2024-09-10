#include "editor.hpp"

#include "postProcessing.hpp"
#include "editorController.h"
#include "renderManager.h"
#include "screenRender.hpp"
#include "console.hpp"
#include "renderer.h"
#include "settings.h"
#include "screen.h"
#include "engine.h"
#include "memory.h"
#include "skybox.h"
#include "timer.h"

#include "core/filmbox/filmbox.hpp"

#include "core/file/stream.hpp"
#include "core/file/file.hpp"

#include "core/statistics.hpp"
#include "core/assetsManager.hpp"
#include "core/scene.hpp"
#include "core/event.hpp"

#include "script/meshFilter.hpp"
#include "script/meshRender.hpp"
#include "script/transform.hpp"
#include "script/animator.hpp"
#include "script/camera.hpp"
#include "script/light.hpp"

#include "guizmo/ImGuizmo.h"

#include "ui/ImGuiRender.h"
#include "ui/View.h"

#include "scriptManager/scriptManager.hpp"

#include "geometry/batching.hpp"

#include "ecs/manager.hpp"

#include <linux/input-event-codes.h>
#include <filesystem>
#include <sstream>
#include <iomanip>

#define IMGUI_PAYLOAD_GAMEOBJECT "##payload_game_object"
#define IMGUI_PAYLOAD_FILE "##payload_file"

#define ITEMS_PER_PAGE 20

const std::string getfilesize(std::size_t sz) {
    float kb = sz / 1024.0f;
    float mb = kb / 1024.0f;
    float gb = mb / 1024.0f;

    std::ostringstream oss;

    if (gb >= 1.0f) {
        oss << std::fixed << std::setprecision(2) << gb << "GB";
    } else if (mb >= 1.0f) {
        oss << std::fixed << std::setprecision(2) << mb << "MB";
    } else if (kb >= 1.0f) {
        oss << std::fixed << std::setprecision(2) << kb << "KB";
    } else {
        oss << std::fixed << std::setprecision(2) << sz << "B";
    }
    return oss.str();
}

enum IconType {
    ICON_FOLDER       = 0,
    ICON_FILE         = 1,
    ICON_BACK_FILE    = 2,
    ICON_GAMEOBJECT   = 3,
	ICON_PENCIL       = 4,
	ICON_LIGHT        = 5,
	ICON_TEXTURE      = 6,
	ICON_PNG          = 7,
	ICON_PLAY_GAME    = 8,
	ICON_STOP_GAME    = 9,
	ICON_BIN          = 10,
	ICON_IMPORT       = 11,
	ICON_ASCENDENTE   = 12,
	ICON_DESCENDENTE  = 13,
	ICON_EDIT         = 14,
	ICON_ANIMATION    = 15,
	ICON_SHADER       = 16,
	ICON_SCENE        = 17,
};

std::unordered_map<IconType, grr::gTexture *> iconsUI;

std::vector<std::unique_ptr<Filmbox>> m_filmboxList;

grr::gTexture *Editor::s_texturePlayGame = nullptr;

std::unordered_map<std::string, std::shared_ptr<Scene>> Editor::m_scenes;

GameObject *Editor::s_objectSelected = nullptr;

ImGuiIO* Editor::s_io = nullptr;

bool Editor::bFileDoubleClick = false;

bool Editor::bCutAllFile = false;

bool Editor::bCopyAllFile = false;

bool Editor::bViewPortFocused = false;

bool Editor::bOpenPopupInfoDir = false;

bool Editor::bScriptReloaded = false;

bool Editor::bOpenPopupOptionGameObject = false;

bool Editor::pPopupFileManager = false;

ImVec4 Editor::FPSColor = {255, 255, 255, 255};

ImVec2 Editor::m_lastViewportSize = {0.0f, 0.0f};

ImGuizmo::OPERATION Editor::m_guizmoOperation = ImGuizmo::TRANSLATE;

ImGuizmo::MODE Editor::m_guizmoManipulate = ImGuizmo::WORLD;

grm::u32 Editor::s_maxProgress = 0;

grm::u8 Editor::m_currentPage = 0;

std::unordered_map<std::filesystem::path, bool> Editor::fLisDirSelected;

std::filesystem::path Editor::m_currentPath;

void Editor::Initialize() {
    s_io = &ImGui::GetIO();

    Settings::IsGameRunning = false;

    m_currentPath = File::GetAsset("");
}

void Editor::Shutdown() {
    m_scenes.clear();
}

// ===================================== WARNING =========================================================
#include "core/binary/gHeader.hpp"

void WriteNode(const FbxScene *scene, const FbxNode &node) {
    // Escreve os dados do modelo
    gModel pModel;
    pModel.m_scale = node.getScale();
    pModel.m_rotation = node.getRotation();
    pModel.m_position = node.getPosition();
    std::memcpy(pModel.m_name, node.getName().c_str(), 256);

    if (!Stream::write(&pModel, 1)) {
        return;
    }

    auto &nodeMesh = node.getMeshes();

    // escreve a quantidade de mesh
    grm::u32 meshCount = static_cast<grm::u32>(nodeMesh.size());
    if (!Stream::write(&meshCount, 1)) {
        std::cerr << "Failed to write mesh count to file." << std::endl;
        return;
    }

    // alloca as meshes na memória em forma de pilha (fácil acesso)
    std::vector<gMesh> meshes(meshCount);
    std::vector<gMaterial> pMateials;
    std::unordered_map<std::string, int> m_map;
    
    for (grm::u32 i = 0; i < meshCount; ++i) {
        auto &fbxMesh = nodeMesh[i];

        for (auto &mIdx : fbxMesh.getMaterials()) {
            auto& fbxMaterial = scene->getMaterial(mIdx);

            auto it = m_map.find(fbxMaterial.getName());
            if (it != m_map.end()) {
                continue;
            }
            m_map.emplace(fbxMaterial.getName(), 0);

            auto& m = pMateials.emplace_back();
            std::strcpy(m.m_name, fbxMaterial.getName().c_str());
            
            m.m_color = fbxMaterial.getColor();
            m.m_roughness = fbxMaterial.roughness;
            m.m_metallic = fbxMaterial.metallic;
            m.m_ambientOcclusion = fbxMaterial.ao;

            auto &textures = fbxMaterial.getTextures();

            m.m_textureCount = textures.size();
            m.m_textures = new gTexture[m.m_textureCount];

            int id = 0;
            for (auto &&[type, path] : textures) {
                auto &pTexture = m.m_textures[id];
                pTexture.m_type = gTextureType_albedo;
                std::strcpy(pTexture.m_path, path.c_str());
                id++;
            }
        }

        auto &vertices = fbxMesh.getVertices();
        auto &indices = fbxMesh.getIndices();

        auto &pMesh = meshes[i];
        std::memcpy(pMesh.m_name, fbxMesh.getName().c_str(), 256);

        pMesh.m_hasBone = fbxMesh.hasBone();
        
        pMesh.m_verticeCount = static_cast<grm::u32>(vertices.size());
        pMesh.m_vertices = new gVertex[pMesh.m_verticeCount];
        for (auto j=0;j<pMesh.m_verticeCount;++j) {
            pMesh.m_vertices[j].position = vertices[j].position;
            pMesh.m_vertices[j].normal   = vertices[j].normal;
            pMesh.m_vertices[j].texCoord = vertices[j].texCoord;
            pMesh.m_vertices[j].mIdx     = vertices[j].index;

            for (auto k=0;k<4;++k) {
                pMesh.m_vertices[j].weight[k]    = vertices[j].weight[k];
                pMesh.m_vertices[j].boneIndex[k] = vertices[j].boneIndex[k];
            }
        }

        pMesh.m_indicesCount = static_cast<grm::u32>(indices.size());
        pMesh.m_indices = new grm::u32[pMesh.m_indicesCount];
        for (int j=0;j<pMesh.m_indicesCount;++j) {
            pMesh.m_indices[j] = indices[j];
        }
        
        // Write bones from skeleton.
        if (!pMesh.m_hasBone) {
            continue;
        }

        auto &bones = fbxMesh.getBones();

        pMesh.m_boneCount = static_cast<grm::u32>(bones.size());
        pMesh.m_bones = new gBone[pMesh.m_boneCount];

        for (auto j=0;j<pMesh.m_boneCount;++j) {
            auto &fbxBone = bones[j];

            auto &name = fbxBone.getName();

            auto &pBone = pMesh.m_bones[j];
            std::strcpy(pBone.m_name, name.c_str());
            pBone.m_id = fbxBone.m_id;
            
            pBone.m_scale = fbxBone.getScale();
            pBone.m_rotation = fbxBone.getRotation();
            pBone.m_position = fbxBone.getPosition();
        }
    }

    // Escrever os dados das meshes no arquivo
    for (auto &mesh : meshes) {
        if (mesh.serialize() != 0) {
            Console::log(LogInfo::ERROR, "Falied to write mesh: " + std::string(mesh.m_name));
            continue;
        }
    }

    grm::u32 mSize = pMateials.size();
    if (!Stream::write(&mSize, 1)) {
        return Console::log(LogInfo::ERROR, "Falied to material data ");
    }

    for (auto &material : pMateials) {
        Stream::write(material.m_name, 256);
        Stream::write(&material.m_color, 1);
        Stream::write(&material.m_roughness, 1);
        Stream::write(&material.m_metallic, 1);
        Stream::write(&material.m_ambientOcclusion, 1);

        Stream::write(&material.m_textureCount, 1);
        Stream::write(material.m_textures, material.m_textureCount);

        delete[] material.m_textures;
    }

    auto &children = node.getChildren();

    grm::u32 nSize = children.size();
    Stream::write(&nSize, 1);

    // escreve os filhos
    for (const auto &child : children) {
        WriteNode(scene, child);
    }
}

void WriteAnimation(const FbxScene *fbxScene) {
    auto &animations = fbxScene->getAnimations();

    gHeader pHeader{{'g', 'r', 'a', 'n'}, 1.0f};

    for (auto &animation : animations) {
        auto name = animation.getName();

        if (!Stream::open(File::GetAsset("/animation/" + name + ".anm"), std::ios_base::out | std::ios_base::binary)) {
            continue;
        }
        Stream::write<gHeader>(&pHeader, 1);

        gAnimationHeader pAnimationHeader;
        std::strcpy(pAnimationHeader.m_name, name.c_str());
        pAnimationHeader.m_tick = animation.getTick();
        pAnimationHeader.m_duration = animation.getDuration();

        Stream::write<gAnimationHeader>(&pAnimationHeader, 1);

        auto &channels = animation.getChannels();

        grm::size sChannel = channels.size();
        gChannel *pChannels = new gChannel[sChannel];

        for (auto i=0;i<sChannel;++i) {
            auto &channel = channels[i];

            auto &name = channel.getName();
            auto &keysScale = channel.getKeysScale();
            auto &keysRotation = channel.getKeysRotation();
            auto &keysPosition = channel.getKeysPosition();

            auto &pChannel = pChannels[i];
            std::strcpy(pChannel.m_name, name.c_str());

            // gen key scale
            pChannel.m_keyScaleCount = keysScale.size();
            pChannel.m_keyScale = new gKeyScale[pChannel.m_keyScaleCount];
            for (auto i=0;i<pChannel.m_keyScaleCount;++i) {
                pChannel.m_keyScale[i].time = keysScale[i].time;
                pChannel.m_keyScale[i].value = keysScale[i].value;
            }

            //gen key rotation
            pChannel.m_keyRotationCount = keysRotation.size();
            pChannel.m_keyRotation = new gKeyRotation[pChannel.m_keyRotationCount];
            for (auto i=0;i<pChannel.m_keyRotationCount;++i) {
                pChannel.m_keyRotation[i].time = keysRotation[i].time;
                pChannel.m_keyRotation[i].value = keysRotation[i].value;
            }

            // gen key position
            pChannel.m_keyPositionCount = keysPosition.size();
            pChannel.m_keyPosition = new gKeyPosition[pChannel.m_keyPositionCount];
            for (auto i=0;i<pChannel.m_keyPositionCount;++i) {
                pChannel.m_keyPosition[i].time = keysPosition[i].time;
                pChannel.m_keyPosition[i].value = keysPosition[i].value;
            }
        }
        
        Stream::write<grm::size>(&sChannel, 1);
        for (auto i=0;i<sChannel;++i) {
            auto &pChannel = pChannels[i];

            // ame
            Stream::write<char>(pChannel.m_name, 256);

            // gen key scale
            Stream::write<grm::size>(&pChannel.m_keyScaleCount, 1);
            Stream::write<gKeyScale>(pChannel.m_keyScale, pChannel.m_keyScaleCount);

            //gen key rotation
            Stream::write<grm::size>(&pChannel.m_keyRotationCount, 1);
            Stream::write<gKeyRotation>(pChannel.m_keyRotation, pChannel.m_keyRotationCount);

            // gen key position
            Stream::write<grm::size>(&pChannel.m_keyPositionCount, 1);
            Stream::write<gKeyPosition>(pChannel.m_keyPosition, pChannel.m_keyPositionCount);
        }
        
        // ========================= CLEAN =========================
        for (auto i=0;i<sChannel;++i) {
            auto &pChannel = pChannels[i];

            // delete keys scale
            delete[] pChannel.m_keyScale;

            // delete keys rotation
            delete[] pChannel.m_keyRotation;

            // delete keys position
            delete[] pChannel.m_keyPosition;
        }
        
        delete[] pChannels;

        Stream::close();
    }
}

void WriteHeader(const FbxScene *scene, const FbxNode &node) {
    Console::log(LogInfo::INFO, "Write 3D Mode: " + node.getName());

    if (!Stream::open(File::GetAsset("/model/" + node.getName() + ".gmd"), std::ios_base::out | std::ios_base::binary)) {
        return Console::log(LogInfo::INFO, "Failed to write the 3D model: " + node.getName());
    }

    gHeader pHeader {{'g', 'r', 'm', 'd'}, 1.0f};
    if (!Stream::write(&pHeader, 1)) {
        Stream::close();
        return Console::log(LogInfo::INFO, "Failed to write the 3D model (0)");
    }

    WriteNode(scene, node);

    Stream::close();
}

void PostProcess(const FbxScene *scene, Filmbox *filmbox) {
    WriteAnimation(scene);
    
    auto& fRootNode = scene->getRootNode();
    for (const auto& node : fRootNode.getChildren()) {
        WriteHeader(scene, node);
    }
    delete filmbox;
}

void Editor::DropFile(const std::filesystem::path &path) {
    if (path.extension() != ".fbx") {
        return;
    }
    Console::log(LogInfo::INFO, "Loading fbx: " + path.extension().string());

    auto *filmbox = new Filmbox(path.string());
    filmbox->setPostProcess(PostProcess);
    filmbox->load();
}

void Editor::KeyBoardUp(grm::u32 key) {
    if (!(bViewPortFocused && key == KEY_ESC)) {
        return;
    }

    if (s_io)
        s_io->MouseDrawCursor = !s_io->MouseDrawCursor;
}

void Editor::LoadScript() {
    // ScriptManager::Initialize();

    // if (ScriptManager::SetScreenSize) {
    //     ScriptManager::SetScreenSize(Settings::ViewportSize);
    // }

    // RenderManager::Initialize();

    // ScreenRender::Initialize();

    // Manager::Replay();
}

void Editor::UnloadScript() {
    // Manager::Stop();

    // RenderManager::Shutdown();

    // ScreenRender::Shutdown();

    // ScriptManager::Shutdown();
}

// ===================================== END =========================================================
void Editor::RenderUI() {
    // ImGui::ShowDemoWindow();
    // return;
    if (AssetsManager::m_processCount != 0) {
        ProgressLoad();
        return;
    }
    
    RenderDocking();

    RenderViewportUI();

    RenderScenesUI();

    RenderConsoleUI();

    RenderPropertiUI();

    if (s_io->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_S)) {
        for (auto &&[name, ptr] : m_scenes) {
            Event::Add([&name, &ptr]{
                Scene::Use(&(*ptr));
                
                auto camera = Camera::Current();
                if (!camera) {
                    return;
                }

                auto lastApect = camera->getAspectRatio();


                PostProcessing::Bind(Engine::ImGuiPostProcess);
                auto &size = PostProcessing::GetSize();

                camera->setAspectRatio(static_cast<float>(size.x) / static_cast<float>(size.y));

                Engine::MainFramebuffer();

                // grr::gRender::SetRenderState(grr::GR_FRAMEBUFFER_SRGB, grr::GR_TRUE);

                camera->setAspectRatio(lastApect);

                PostProcessing::Save(File::GetAsset("/texture/" + name + ".jpg"), Engine::ImGuiPostProcess);

                ptr->save();
            });
        }
        // 
    }
}

void Editor::RenderDocking() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("GR - GameEngine 3D", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Project")) {
            if (ImGui::MenuItem("Close")) {
                CloseProject();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (s_io->ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGui::DockSpace(ImGui::GetID("##RenderDocking"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void Editor::CloseProject() {
    auto path = File::GetFile("/config.ini", true);
    if (!File::Exists(path)) {
        return;
    }
    File::Delete(path);

    ScriptManager::Shutdown();

    RenderManager::Shutdown();

    ScreenRender::Shutdown();

    Skybox::Shutdown();

    Editor::Shutdown();

    AssetsManager::Shutdown();

    GameObject::Clear();
}

void Editor::RenderScenesUI() {
    auto isOpen = ImGui::Begin("Cenario", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if (!isOpen) {
        return;
    }

    auto name = Scene::HasScene() ? Scene::GetCurrentScene()->getName() : "Default";

    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth)) {
        if (ImGui::BeginDragDropTarget()) {
            ImGuiDragDropFlags target_flags = 0;
            // target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
            // target_flags |= ImGuiDragDropFlags_SourceAllowNullID;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_GAMEOBJECT, target_flags)) {
                std::string name((char*)payload->Data, payload->DataSize);
                if (!name.empty()) {
                    auto target = GameObject::Find(name);

                    auto &ts = target->getTransform();
                    ts.setParent(nullptr);
                    
                    Scene::GetCurrentScene()->Add(target);
                }
            }
            ImGui::EndDragDropTarget();
        }
        
        RenderHierarchi();

        ImGui::TreePop();
    }

    if (ImGui::IsKeyReleased(ImGuiKey_Delete) && s_objectSelected) {
        Event::Add(&Scene::Delete, Scene::GetCurrentScene(), s_objectSelected);
        s_objectSelected = nullptr;
    }
    
    if (ImGui::IsKeyReleased(ImGuiKey_F2)) {
        ImGui::OpenPopup("##popup_rename_gameobject");
    }

    if (!bOpenPopupOptionGameObject && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("##popup_menu_add");
    }

    if (bOpenPopupOptionGameObject) {
        ImGui::OpenPopup("##popup_options_gameobject");
        bOpenPopupOptionGameObject = false;
    }

    PopupOptionsGameObject();

    PopupRenameGameObject();

    PopupMenuAdd();

    ImGui::End();
}

void Editor::RenderHierarchi() {
    if (!Scene::HasScene()) {
        return;
    }

    auto &list = Scene::GetCurrentScene()->GetGameObjects();

    if (m_currentPage > 0 && ImGui::Button("Before")) {
        --m_currentPage;
    }
    
    RenderPage(list, m_currentPage);
    
    if (list.size() >= m_currentPage && ImGui::Button("Next")) {
        ++m_currentPage;
    }
}

void Editor::RenderPage(std::vector<GameObject *> &list, int page) {
    int start = page * ITEMS_PER_PAGE;
    int end = std::min(start + ITEMS_PER_PAGE, static_cast<int>(list.size()));

    for (int i = start; i < end; ++i) {
        auto& transform = list[i]->getTransform();

        ReadHierarchy(&transform);
    }
}

void Editor::ReadHierarchy(Transform *transform) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

    auto gameObject = transform->getGameObject();
    auto name = gameObject->getName();

    if (s_objectSelected != gameObject) {
        flags &= ~ImGuiTreeNodeFlags_Selected;
    }

    if (transform->hasChildren()) {
        flags &= ~ImGuiTreeNodeFlags_Leaf;
    }

    auto isOpen = ImGui::TreeNodeEx(name.c_str(), flags);

    static int CountClick = 0;

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && s_objectSelected != gameObject) {
        ++CountClick;
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        s_objectSelected = gameObject;
        bOpenPopupOptionGameObject = true;
    }

    if (CountClick >= 2) {
        s_objectSelected = gameObject;
        CountClick = 0;
    }

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) {
        ImGui::Text("Moving \"%s\"", name.c_str());
        ImGui::SetDragDropPayload(IMGUI_PAYLOAD_GAMEOBJECT, name.data(), name.size());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        ImGuiDragDropFlags target_flags = 0;
        // target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
        // target_flags |= ImGuiDragDropFlags_SourceAllowNullID;

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_GAMEOBJECT, target_flags)) {
            std::string name((char*)payload->Data, payload->DataSize);
            if (!name.empty()) {
                auto target = GameObject::Find(name);
                if (!target) {
                    Console::log(LogInfo::ERROR, "GameObject do not found: " + name);
                } else {
                    auto ts = &target->getTransform();
                    if (ts != transform->getParent()) {
                        ts->setParent(transform);
                    
                        Event::Add(&Scene::Remove, Scene::GetCurrentScene(),  target);
                    }
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (isOpen) {
        for (auto& child : transform->getChildren()) {
            ReadHierarchy(child);
        }
        ImGui::TreePop();
    }
}

void Editor::PopupMenuAdd() {
    auto isOpen = ImGui::BeginPopup("##popup_menu_add");
    if (!isOpen) {
        return;
    }
    const char* item_name[1]{"Empty GameObject"};
    auto id = 0;

    ImGui::Text("Add Object");
    if (ImGui::ListBox("##ListBoxOptionsGame", &id, item_name, 1)) {
        if (id == 0) {
            s_objectSelected = &Scene::GetCurrentScene()->CreateGameObject("Empty");
        }
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::PopupOptionsGameObject() {
    bool isOpenRename = false;
    if (ImGui::BeginPopup("##popup_options_gameobject")) {
        const char* item_name[2] {"Delete", "Rename"};

        auto id = 0;
        if (ImGui::ListBox("##ListBoxOptionsGame", &id, item_name, 2)) {
            if (id == 0 && s_objectSelected) {
                Event::Add(&Scene::Delete, Scene::GetCurrentScene(), s_objectSelected);
                s_objectSelected = nullptr;
            } else if (id == 1 && s_objectSelected) {
                isOpenRename = true;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (isOpenRename) {
        ImGui::OpenPopup("##popup_rename_gameobject");
    }
}

void Editor::PopupRenameGameObject() {
    auto isOpen = ImGui::BeginPopup("##popup_rename_gameobject");
    if (!isOpen) {
        return;
    }
    ImGui::Text("Input new name");

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    auto name = s_objectSelected->getName();
    if (name.size() < 256) {
        name.resize(256, '\0');
    }
    
    if (ImGui::InputText("##input_new_name_gameobject", name.data(), name.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
        GameObject::Rename(s_objectSelected, name);

        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void Editor::RenderViewportUI() {
    auto isOpen = ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (!isOpen) {
        return;
    }
    isOpen = false;

    bViewPortFocused = ImGui::IsWindowFocused();

    if (ImGui::BeginTabBar("##SceneListTabBar")) {
       for (auto it = m_scenes.begin(); it != m_scenes.end();) {
            auto& [name, scene] = *it;
            
            bool isQuit = true;
            if (ImGui::BeginTabItem(name.c_str(), &isQuit)) {
                Scene::Use(&(*scene));
                RenderScene();
                ImGui::EndTabItem();
            }

            if (!isQuit) {
                s_objectSelected = nullptr;
                Scene::Use(nullptr);
                
                it = m_scenes.erase(it);
            } else {
                ++it;
            }
        }

        if (ImGui::TabItemButton("+")) {
            isOpen = true;
        }
        ImGui::EndTabBar();
    }

    if (isOpen) {
        ImGui::OpenPopup("##popup_menu_create_scene");
    }

    DropSceneInViewport();

    PopupMenuCreateScene();

    ImGui::End();
}

void Editor::DropSceneInViewport() {
    auto isOpen = ImGui::BeginDragDropTarget();
    if (!isOpen) {
        return;
    }

    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_FILE)) {
        std::string path((char*)payload->Data, payload->DataSize);
        if (!path.empty() && std::filesystem::path(path).filename().extension() == ".scn") {
            auto ptr = std::make_shared<Scene>("NoName");
            auto lastScene = Scene::GetCurrentScene();
            ptr->load(path);
            Scene::Use(lastScene);

            auto it = m_scenes.find(ptr->getName());
            if (it == m_scenes.end()) {
                m_scenes.emplace(ptr->getName(), std::move(ptr));
            }

        }
    }
    ImGui::EndDragDropTarget();
}

Matrix4x4 InverseMatrix(const Matrix4x4 &matrix) {
    float a = matrix.m00, b = matrix.m01, c = matrix.m02, d = matrix.m03;
    float e = matrix.m10, f = matrix.m11, g = matrix.m12, h = matrix.m13;
    float i = matrix.m20, j = matrix.m21, k = matrix.m22, l = matrix.m23;
    float m = matrix.m30, n = matrix.m31, o = matrix.m32, p = matrix.m33;

    float kp_lo = k * p - l * o;
    float jp_ln = j * p - l * n;
    float jo_kn = j * o - k * n;
    float ip_lm = i * p - l * m;
    float io_km = i * o - k * m;
    float in_jm = i * n - j * m;

    float a00 = +(f * kp_lo - g * jp_ln + h * jo_kn);
    float a01 = -(e * kp_lo - g * ip_lm + h * io_km);
    float a02 = +(e * jp_ln - f * ip_lm + h * in_jm);
    float a03 = -(e * jo_kn - f * io_km + g * in_jm);

    float det = a * a00 + b * a01 + c * a02 + d * a03;

    if (std::abs(det) < 1e-6f) {
        return Matrix4x4::identityMatrix;
    }

    float invDet = 1.0f / det;

    Matrix4x4 result = Matrix4x4::identityMatrix;
    result.m00 = a00 * invDet;
    result.m01 = -(b * kp_lo - c * jp_ln + d * jo_kn) * invDet;
    result.m02 = +(b * (g * p - h * o) - c * (f * p - h * n) + d * (f * o - g * n)) * invDet;
    result.m03 = -(b * (g * l - h * k) - c * (f * l - h * j) + d * (f * k - g * j)) * invDet;

    result.m10 = a01 * invDet;
    result.m11 = +(a * kp_lo - c * ip_lm + d * io_km) * invDet;
    result.m12 = -(a * (g * p - h * o) - c * (e * p - h * m) + d * (e * o - g * m)) * invDet;
    result.m13 = +(a * (g * l - h * k) - c * (e * l - h * i) + d * (e * k - g * i)) * invDet;

    result.m20 = a02 * invDet;
    result.m21 = -(a * ip_lm - b * io_km + d * in_jm) * invDet;
    result.m22 = +(a * (f * p - h * o) - b * (e * p - h * m) + d * (e * n - f * m)) * invDet;
    result.m23 = -(a * (f * k - g * j) - b * (e * k - g * i) + d * (e * j - f * i)) * invDet;

    result.m30 = a03 * invDet;
    result.m31 = +(a * jo_kn - b * io_km + c * in_jm) * invDet;
    result.m32 = -(a * (f * o - g * n) - b * (e * o - g * m) + c * (e * n - f * m)) * invDet;
    result.m33 = +(a * (f * k - g * j) - b * (e * k - g * i) + c * (e * j - f * i)) * invDet;

    return result;
}

void Editor::RenderScene() {
    auto cursorY = ImGui::GetCursorPosY();
    cursorY += 5.0f;

    ImVec2 curSize = ImGui::GetContentRegionAvail();
    if (m_lastViewportSize.x != curSize.x || m_lastViewportSize.y != curSize.y && curSize.x > 5.0f && curSize.y > 5.0f || bScriptReloaded) {
        auto &viewport = Settings::ViewportSize = {static_cast<int>(curSize.x), static_cast<int>(curSize.y)};

        if (ScriptManager::SetScreenSize) {
            ScriptManager::SetScreenSize(Settings::ViewportSize);
        }

        auto camera = Camera::Current();
        if (camera) {
            camera->setAspectRatio(static_cast<float>(viewport.x) / static_cast<float>(viewport.y));
        }

        PostProcessing::Resize(Engine::MainPostProcess, viewport);
        
        m_lastViewportSize = curSize;
        bScriptReloaded = false;
    }

    auto texture = PostProcessing::GetTextureColor(Engine::ImGuiPostProcess);
    if (texture) {
        ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), curSize, ImVec2(0, 1), ImVec2(1, 0));

        DropGameObjectInScene();
    }

    auto cursorPos =  ImGui::GetCursorPos();
    auto pos = ImGui::GetWindowPos();

    // Play button
    {
        ImGui::SetCursorPos(ImVec2(curSize.x / 2 - cursorPos.x, cursorY));

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(s_texturePlayGame->getID()), ImVec2(16, 16))) {
            Settings::IsGameRunning =! Settings::IsGameRunning;

            if (Settings::IsGameRunning) {
                s_texturePlayGame = iconsUI[IconType::ICON_STOP_GAME];

                UnloadScript();

                s_objectSelected = nullptr;
            } else {
                s_texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];
                bScriptReloaded = true;

                LoadScript();
            }
        }
    }

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(pos.x, pos.y + (cursorY - 5.0f), curSize.x, curSize.y);

    if (s_objectSelected && Camera::Current) {
        if (s_io->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_T)) {
            m_guizmoOperation = ImGuizmo::TRANSLATE;
        }
        if (s_io->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_R)) {
            m_guizmoOperation = ImGuizmo::ROTATE;
        }
        if (s_io->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_S)) {
            m_guizmoOperation = ImGuizmo::SCALE;
        }
        if (s_io->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_Space)) {
            if (m_guizmoManipulate == ImGuizmo::WORLD) {
                m_guizmoManipulate = ImGuizmo::LOCAL;
            } else if (m_guizmoManipulate == ImGuizmo::LOCAL) {
                m_guizmoManipulate = ImGuizmo::WORLD;
            }
        }
        
        auto& transform = s_objectSelected->getTransform();

        auto globalMatrix = transform.getGlobalMatrix();

        auto camera = Camera::Current();

        if (camera && ImGuizmo::Manipulate(camera->getViewmatrix().data, camera->getPerspesctive().data, m_guizmoOperation, m_guizmoManipulate, globalMatrix.data)) {
            // auto localMatrix = globalMatrix;
            // if (transform.hasParent()) {
            //     auto inverseParent = InverseMatrix(transform.getParent()->getGlobalMatrix());

            //     localMatrix = inverseParent * localMatrix;
            //     // localMatrix = localMatrix.transpose();
            // }
            Vector3 s;
            Quaternion r;
            Vector3 t;
            globalMatrix.decompose(s, r, t);

            transform.setLocalScale(s);
            transform.setLocalRotation(r);
            transform.setLocalPosition(t);
        }
    }

    if (TimeLast::FPS > 50) {
        FPSColor = {0, 255, 0, 255};
    } else if (TimeLast::FPS > 25) {
        FPSColor = {255, 255, 0, 255};
    } else if (TimeLast::FPS > 0) {
        FPSColor = {255, 0, 0, 255};
    }

    // Fps Draw
    {
        std::string fps = "FPS: 000.00f";

        auto textSize = ImGui::CalcTextSize(fps.c_str());

        auto curtorPos = ImVec2(curSize.x - textSize.x, cursorY + textSize.y);

        ImGui::SetCursorPos(curtorPos);

        ImGui::TextColored(FPSColor, "FPS: %.2f", Statistics::m_threadFrame.fps);
    }

    ImGui::SetCursorPos({8.0f, cursorY});

    ImGui::PushStyleColor(ImGuiCol_Text, {0.0f, 0.0f, 1.0f, 1.0f});

    ImGui::BeginGroup();
    if (ImGui::RadioButton("Local", m_guizmoManipulate == ImGuizmo::LOCAL)) {
        m_guizmoManipulate = ImGuizmo::LOCAL;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("World", m_guizmoManipulate == ImGuizmo::WORLD)) {
        m_guizmoManipulate = ImGuizmo::WORLD;
    }
    ImGui::EndGroup();

    ImGui::PopStyleColor();

    if (s_objectSelected && ImGui::IsWindowFocused() && s_io->KeyShift && ImGui::IsKeyReleased(ImGuiKey_D)) {
        s_objectSelected = &GameObject::Instantiate(s_objectSelected);
    }
}

void Editor::DropGameObjectInScene() {
    if (!Camera::Current) {
        return;
    }
    auto camera = Camera::Current();
    if (!camera) {
        return;
    }
    
    auto isOpen = ImGui::BeginDragDropTarget();
    if (!isOpen && camera) {
        return;
    }

    auto camTransform = camera->getTransform();

    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_FILE)) {
        std::string path((char*)payload->Data, payload->DataSize);
        if (!path.empty() && std::filesystem::path(path).filename().extension() == ".gmd") {
            s_objectSelected = AssetsManager::LoadGameObject(path);

            auto &transform = s_objectSelected->getTransform();
            transform.setLocalPosition(camera->getPosition() + (camTransform->getforward() * 5.5f));
        }
    }
    ImGui::EndDragDropTarget();
}

void Editor::PopupMenuCreateScene() {
    auto isOpen = ImGui::BeginPopup("##popup_menu_create_scene");
    if (!isOpen) {
        return;
    }

    ImGui::Text("Input scene name: ");

    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    char name[256] = {0};
    if (ImGui::InputText("##input_scene_name", name, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
        m_scenes.emplace(name, std::shared_ptr<Scene>(Scene::Create(name)));
        
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::RenderFilesUI() {
    for (auto &entry : std::filesystem::directory_iterator(m_currentPath)) {
        RenderTreeNodeFile(entry.path());
    }

    if (pPopupFileManager && !ImGui::IsPopupOpen("##popup_dir_manager")) {
        ImGui::OpenPopup("##popup_dir_manager");
    }

    PopupDirmanager();
    /*
    grr::gTexture *gtCurrentTexture = iconsUI[ICON_BACK_FILE];

    ImGui::BeginColumns("##colums_01", 5, ImGuiColumnsFlags_NoBorder);
    if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(gtCurrentTexture->getID()), {Settings::IconSize.x, Settings::IconSize.y}, ImVec2(0, 1), ImVec2(1, 0)) && m_currentFile.has_parent_path()) {
        m_currentFile = m_currentFile.parent_path();

        if (!bCutAllFile && !bCopyAllFile) {
            fLisDirSelected.clear();
        }
    }

    bool bSelelectAll = false;
    bool bPopupOpen = false;

    if (s_io->KeyCtrl && ImGui::IsKeyDown(ImGuiKey_A) && ImGui::IsWindowFocused()) {
        bSelelectAll = true;
        fLisDirSelected.clear();
    }

    if (s_io->KeyCtrl && ImGui::IsKeyDown(ImGuiKey_X) && ImGui::IsWindowFocused()) {
        for (auto &&[path, bCut] : fLisDirSelected) {
            bCut = true;
        }
        bCutAllFile = true;
        bCopyAllFile = false;
    }

    if (s_io->KeyCtrl && ImGui::IsKeyDown(ImGuiKey_C) && ImGui::IsWindowFocused()) {
        for (auto &&[path, bCut] : fLisDirSelected) {
            bCut = true;
        }
        bCutAllFile = false;
        bCopyAllFile = true;
    }

    if (s_io->KeyCtrl && ImGui::IsKeyDown(ImGuiKey_V) && ImGui::IsWindowFocused()) {
        for (auto &&[path, bCut] : fLisDirSelected) {
            if (bCopyAllFile)
                File::Copy(path, m_currentFile);
            else if (bCutAllFile)
                File::Move(path, m_currentFile);
        }
        fLisDirSelected.clear();
    }
    
    for (auto& file : std::filesystem::directory_iterator(m_currentFile)) {
        ImGui::NextColumn();

        auto &path = file.path();

        auto extension = path.extension().string();
        auto name = path.filename().string();

        ImVec4 tintColor(1.0f, 1.0f, 1.0f, 1.0f);

        ImGui::PushID(name.c_str());

        gtCurrentTexture = iconsUI[ICON_FILE];
        if (file.is_directory()) {
            gtCurrentTexture = iconsUI[ICON_FOLDER];
        } else {
            if (extension == ".gmd") {
                gtCurrentTexture = iconsUI[ICON_GAMEOBJECT];
            } else if (extension == ".anm") {
                gtCurrentTexture = iconsUI[ICON_ANIMATION];
            } else if (extension == ".scn") {
                gtCurrentTexture = iconsUI[ICON_SCENE];
            }else if (extension == ".frag" || extension == ".vert") {
                gtCurrentTexture = iconsUI[ICON_SHADER];
            }
        }

        auto it = fLisDirSelected.find(path);
        if (it != fLisDirSelected.end()) {
            tintColor = {0.5f, 0.0f, 0.8f, 1.0f};

            if ((*it).second)
                tintColor = {0.5f, 0.5f, 0.5f, 1.0f};
        }

        if (s_io->KeyShift) {
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                bFileDoubleClick = true;
            }
        } else {
            bFileDoubleClick = false;
        }

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(gtCurrentTexture->getID()), {Settings::IconSize.x, Settings::IconSize.y}, ImVec2(0, 1), ImVec2(1, 0), -1, {0.0f, 0.0f, 0.0f, 0.0f}, tintColor) && !s_io->KeyShift) {
            if (file.is_directory()) {
                m_currentFile = path;
            }
        }
        

        // para adicionar os arquivos a lista @fLisDirSelected
        if (bFileDoubleClick && ImGui::IsItemHovered() && it == fLisDirSelected.end() || bSelelectAll) {
            fLisDirSelected.emplace(path, false);
        }

        // Click com dois dedos touchpad
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            if (!fLisDirSelected.empty()) {
                fLisDirSelected.clear();
            }
            fLisDirSelected.emplace(path, false);
            bOpenPopupInfoDir = true;
        }

        if (!file.is_directory() && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) {
            auto str = path.string();

            ImGui::Text("Moving \"%s\"", str.c_str());
            ImGui::SetDragDropPayload(IMGUI_PAYLOAD_FILE, str.data(), str.size());
            ImGui::EndDragDropSource();
        }

        ImGui::Text("%s", name.c_str());

        ImGui::PopID();
    }

    ImGui::EndColumns();
    
    if (!bOpenPopupInfoDir && fLisDirSelected.empty() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered()) {
        ImGui::OpenPopup("##popup_file_manager");
    }

    if (bOpenPopupInfoDir || !fLisDirSelected.empty() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered()) {
        bPopupOpen = true;
        ImGui::OpenPopup("##popup_dir_manager");
        bOpenPopupInfoDir = false;
    }

    

    PopupFileManager();

    PopupCreateDirectory();
    */
}

void Editor::RenderTreeNodeFile(std::filesystem::path path) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
    flags |= ImGuiTreeNodeFlags_SpanFullWidth;
    flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
    flags |= ImGuiTreeNodeFlags_Selected;

    auto bDirectory = std::filesystem::is_directory(path);
    if (bDirectory) {
        flags &= ~ImGuiTreeNodeFlags_Leaf;
    }

    if (fLisDirSelected.find(path) == fLisDirSelected.end()) {
        flags &= ~ImGuiTreeNodeFlags_Selected;
    }

    auto name = path.filename();
    
    auto isOpen = ImGui::TreeNodeEx(name.c_str(), flags);
    if (!isOpen) {
        return;
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && !fLisDirSelected.empty()) {
        pPopupFileManager = true;
    } else if (s_io->KeyShift && ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        fLisDirSelected.emplace(path, true);
    } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && fLisDirSelected.find(path) == fLisDirSelected.end()) {
        fLisDirSelected.clear();
    } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && fLisDirSelected.find(path) != fLisDirSelected.end()) {
        fLisDirSelected.clear();
        fLisDirSelected.emplace(path, true);
    }

    if (!bDirectory && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) {
        auto str = path.string();

        ImGui::Text("%s", str.c_str());
        ImGui::SetDragDropPayload(IMGUI_PAYLOAD_FILE, str.data(), str.size());
        ImGui::EndDragDropSource();
    }

    if (!bDirectory) {
        return ImGui::TreePop();
    }

    std::vector<std::filesystem::directory_entry> dirs;
    for (auto &file : std::filesystem::directory_iterator(path)) {
        dirs.push_back(file);
    }

    std::sort(dirs.begin(), dirs.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.is_directory() != rhs.is_directory()) {
            return lhs.is_directory() > rhs.is_directory();
        }
        return lhs.path().filename() < rhs.path().filename();
    });

    for (auto &file : dirs) {
        RenderTreeNodeFile(file);
    }
    ImGui::TreePop();
}

void Editor::PopupDirmanager() {
    auto isOpen = true;
    if (ImGui::BeginPopupModal("##popup_dir_manager", &isOpen)) {
        for (auto &&[path, isSeleceted] : fLisDirSelected) {
            ImGui::Text("Dir: %s", path.filename().string().c_str());
        }

        auto idx = 0;

        const char *item_list[2] {"Delete"};

        if (ImGui::ListBox("##DirManager", &idx, item_list, 1)) {
            switch (idx) {
            case 0:
                for (auto &&[path, isSeleceted] : fLisDirSelected) {
                    File::Delete(path);
                }
                break;
            default:
                break;
            }
            fLisDirSelected.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (!isOpen && pPopupFileManager) {
        std::cout << "Close" << std::endl;
        fLisDirSelected.clear();
        pPopupFileManager = false;
    }
}

void Editor::PopupFileManager() {
    static bool isOpenPopup = false;
    if (ImGui::BeginPopup("##popup_file_manager")) {
        ImGui::Text("FileManager");

        auto idx = 0;

        const char *item_list[2] {"Create New Directory"};

        if (ImGui::ListBox("##FileManager", &idx, item_list, 1)) {
            switch (idx) {
            case 0:
                isOpenPopup = true;
                break;
            default:
                break;
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (isOpenPopup) {
        ImGui::OpenPopup("##popup_create_new_dir");

        isOpenPopup = false;
    }
}

void Editor::PopupCreateDirectory() {
    auto isOpen = ImGui::BeginPopup("##popup_create_new_dir");
    if (!isOpen) {
        return;
    }
    ImGui::Text("Input directory name:");
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);

    char name[256] = {0};
    if (ImGui::InputText("##InputDirectoryName", name, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
        File::CreateDirectory(m_currentPath.string() + "/" + std::string(name));
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void Editor::RenderConsoleUI() {
    static NavegationFlags flags = NavegationFlags_console;

    auto isOpen = ImGui::Begin("Navegador", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
    if (!isOpen) {
        return;
    }
    if (ImGui::BeginMenuBar()) {
        if (ImGui::MenuItem("Console")) {
            flags = NavegationFlags_console;
        }
        if (ImGui::MenuItem("Files")) {
            flags = NavegationFlags_files;
        }
        if (ImGui::MenuItem("Models")) {
            flags = NavegationFlags_models;
        }
        ImGui::EndMenuBar();
    }

    if (flags & NavegationFlags_files) {
        RenderFilesUI();
    }

    if (flags & NavegationFlags_models) {
    }

    if (flags & NavegationFlags_console) {

        auto logs = Console::GetListLogs();

        for (const auto& log : logs) {
            ImVec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
            std::string type = "[ DEFAULT ]";

            switch (log.type) {
            case LogInfo::LogType::INFO:
                color = {0.0f, 1.0f, 0.0f, 1.0f};
                type = "[ INFO ]";
                break;
            case LogInfo::LogType::WARNING:
                color = {1.0f, 1.0f, 0.0f, 1.0f};
                type = "[ WARNING ]";
                break;
            case LogInfo::LogType::ERROR:
                color = {1.0f, 0.0f, 0.0f, 1.0f};
                type = "[ ERROR ]";
                break;
            default:
                break;
            }

            ImGui::TextColored(color, "%s", type.c_str());
            ImGui::SameLine();
            ImGui::Text("%s", log.message.c_str());
        }

        if (Console::bChanged) {
            ImGui::SetScrollHereY(1.0f);
            Console::bChanged = false;
        }
    }

    ImGui::End();
}

void Editor::RenderPropertiUI() {
    auto isOpen = ImGui::Begin("Propriedades", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if (!isOpen) {
        return;
    }

    if (ImGui::BeginTabBar("##propertimenubar")) {
        RenderGameObjectPropertiUI();
        
        if (ImGui::BeginTabItem("Estatística", nullptr)) {
            RenderStatisticsMenu();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Configuraçoes")) {
            ImGui::Checkbox("Wireframe", &RenderManager::bWireFrame);
            ImGui::Checkbox("Debug Mode", &DebugDraw::bDebugMode);

            ImGui::Text("Camera: ");
            if (Camera::GetCameras) {
                const auto &viewport = Settings::ViewportSize;

                for (auto &camera : Camera::GetCameras()) {
                    auto gameObject = camera->getGameObject();

                    auto name = gameObject->getName();

                    ImGui::PushID(name.c_str());
                    if (ImGui::Button(name.c_str())) {
                        camera->setAspectRatio(static_cast<float>(viewport.x) / static_cast<float>(viewport.y));

                        Camera::SetEnable(camera);
                    }
                    ImGui::PopID();
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void Editor::RenderStatisticsMenu() {
    ImGui::SeparatorText("Scripts");
            
    ImGui::Text("Entidades: %ld", Manager::GetEntityCount());
    ImGui::Text("GameObject: %ld", GameObject::GetGameObjectCount());

    ImGui::Text("Memoria: %s", getfilesize(Memory::totalMemoryUsage).c_str());

    ImGui::SeparatorText("Cena");
    // ImGui::Text("Vertices: %d | %s", gameStatics.vertices, getfilesize(gameStatics.vertices * sizeof(Vertex3D)).c_str());
    // ImGui::Text("Indice: %d | %s", gameStatics.indices, getfilesize(gameStatics.indices * sizeof(grm::u32)).c_str());
    // ImGui::Text("Triangulos: %.2f", (gameStatics.indices / 3.0f));

    ImGui::SeparatorText("AssetsManager");
    ImGui::Text("Textures: %ld", AssetsManager::GetTextureCount());
    ImGui::Text("Shaders: %ld", AssetsManager::GetShaderCount());
    ImGui::Text("Mesh: %ld", AssetsManager::GetMeshCount());
    ImGui::Text("Material: %ld", AssetsManager::GetMaterialCount());
    ImGui::Text("Event: %ld", Event::GetCount());

    ImGui::SeparatorText("FrameTime: ");
    ImGui::Text("MainThread:  %fms  | FPS: %.2f",  Statistics::m_threadFrame.delta,  (1.0f / Statistics::m_threadFrame.delta));
    ImGui::Text("Render:      %fms  | FPS: %.2f",  Statistics::m_sceneFrame.delta,   (1.0f / Statistics::m_sceneFrame.delta));
    ImGui::Text("GUI:         %fms  | FPS: %.2f",  Statistics::m_guiFrame.delta,     (1.0f / Statistics::m_guiFrame.delta));
    ImGui::Text("Script:      %fms  | FPS: %.2f",  Statistics::m_scriptFrame.delta,  (1.0f / Statistics::m_scriptFrame.delta));

    ImGui::SeparatorText("Renderização");
    // ImGui::Text("Cameras: %ld",   Camera::Get.size());
    if (ScriptManager::GetListLight)
        ImGui::Text("Luzes: %ld",     ScriptManager::GetListLight().size());
    ImGui::Text("Draw Call Count: %ld", Statistics::m_drawCallCount);
    ImGui::Text("Draw Render: %ld/%ld", Statistics::m_totalRender, Statistics::m_drawRenderCount);
    ImGui::Text("Batched: %ld", Batching::GetBatchedCount());
}

void Editor::RenderGameObjectPropertiUI() {
    if (!s_objectSelected || !ScriptManager::GetComponentPool) {
        return;
    }

    auto &componentPool = ScriptManager::GetComponentPool();

    auto isOpenPopup = false;
    // ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::BeginTabItem("GameObject", nullptr)) {
        ImGui::Image(reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_GAMEOBJECT]->getID()), ImVec2(16, 16));

        ImGui::SameLine();

        ImGui::Text("[ %s - %s ]", s_objectSelected->getName().c_str(), s_objectSelected->getTag().c_str());

        ImGui::SameLine();

        auto isActive = s_objectSelected->isActive();
        if (ImGui::Checkbox("##check_box_is_active", &isActive)) {
            s_objectSelected->setActive(isActive);
        }

        s_objectSelected->RenderProperty<Transform>(Editor::OnPropertyTransform);
        s_objectSelected->RenderProperty<MeshFilter>(Editor::OnPropertyMeshFilter);
        s_objectSelected->RenderProperty<MeshRender>(Editor::OnPropertyMeshRender);
        s_objectSelected->RenderProperty<Light>(Editor::OnPropertyLight);

        for (auto &&[name, ptr] : s_objectSelected->getEntity()->getComponents()) {
            if (name == "Transform" || name == "Light" || name == "MeshRender" || name == "MeshFilter") {
                continue;
            }
            auto isOpen = ImGui::TreeNode(name.c_str());

            if (isOpen) {
                auto it = componentPool.find(name);
                if (it == componentPool.end()) {
                    Console::log(LogInfo::ERROR, "Invalid component: " + name);
                    ImGui::TreePop();
                    continue;
                }
                auto &fieldList = it->second->getFields();

                for (auto &field : fieldList) {
                    RenderField(field, &(*ptr));
                }

                ImGui::TreePop();
            }
        }

        if (ImGui::SmallButton("AddComponent")) {
            isOpenPopup = true;
        }

        ImGui::EndTabItem();
    }

    if (isOpenPopup) {
        ImGui::OpenPopup("##popup_add_component");
    }
    PopupAddComponent();
}

void Editor::RenderField(Field &field, Component *component) {
    auto name = field.getName();
    auto type = field.getType();

    if (type == "bool") {
        auto data = Field::GetFieldData<bool>(component, field);

        ImGui::PushStyleColor(ImGuiCol_Button, {0.0f, 0.0f, 0.0f, 1.0f});
        *data = ImGui::Button(name.c_str());
        ImGui::PopStyleColor();
        return;
    }

    ImGui::Text("%s:", name.c_str());
    name = "##" + name;
    
    if (type == "float") {
        auto data = Field::GetFieldData<float>(component, field);

        ImGui::DragFloat(name.c_str(), data);
    } else if (type == "int") {
        auto data = Field::GetFieldData<int>(component, field);

        ImGui::DragInt(name.c_str(), data);
    } else if (type == "bool") {
        auto data = Field::GetFieldData<bool>(component, field);

        ImGui::Checkbox(name.c_str(), data);
    } else if (type == "Vector3") {
        auto data = Field::GetFieldData<Vector3>(component, field);

        ImGui::DragFloat3(name.c_str(), (float *)data);
    }
}

void Editor::PopupAddComponent() {
    if (!ScriptManager::GetComponentPool && !s_objectSelected) {
        return;
    }
    
    auto isOpen = ImGui::BeginPopup("##popup_add_component");
    if (!isOpen) {
        return;
    }
    auto &componentPool = ScriptManager::GetComponentPool();

    for (auto &&[name, pool] : componentPool) {
        if (ImGui::SmallButton(name.c_str())) {
            Entity::addInternalComponent(s_objectSelected->getEntity(), name);

            ImGui::CloseCurrentPopup();
        }
    }
    
    ImGui::EndPopup();
}

/* ********** Propriedades ********** */
void Editor::OnPropertyTransform(Transform *transform) {
    auto isOpen = ImGui::TreeNode("Transform");
    if (!isOpen) {
        return;
    }
    const char* text[] {"X", "Y", "Z"};

    auto position = transform->getLocalPosition();
    ImGui::Text("Posição:");
    if (ImGui::DragFloatEx("##Posicao", text, position.data, 1.0f, 0.0f, 0.0f, "%.3f")) {
        transform->setLocalPosition(position);
    }

    auto scale = transform->getLocalScale();
    ImGui::Text("Escala:");
    if (ImGui::DragFloatEx("##Escala", text, scale.data, 1.0f, 1.0f, 0.0f, "%.3f")) {
        transform->setLocalScale(scale);
    }

    auto euler = Math::eulerAngles(transform->getLocalRotation());

    ImGui::Text("Rotação:");
    if (ImGui::DragFloatEx("##Rotacao", text, euler.data, 1.0f, -360.0f, 360.0f)) {
        transform->setLocalRotation(Math::normalize(Math::euler({Math::rad(euler.x), Math::rad(euler.y), Math::rad(euler.z)})));
    }

    ImGui::TreePop();
}

void Editor::OnPropertyMeshFilter(MeshFilter *MeshFilter) {
}

void Editor::OnPropertyMeshRender(MeshRender *meshRender) {
    auto isOpen = ImGui::TreeNode("MeshRender");
    if (!isOpen) {
        return;
    }

    auto elem = meshRender->flags;

    // int  = Batching::Batching_Flags_Dynamic;
        
    const char* elems_names[4] = {"Dynamic", "GroupMesh", "InstanceDynamic", "InstancedStatic"};
    
    const char* elem_name = (elem >= 0 && elem < 4) ? elems_names[elem] : "Unknown";

    ImGui::Text("Mesh Optimize:");
    if (ImGui::SliderInt("##mesh_optimize", &elem, 0, 4 - 1, elem_name)) {
        meshRender->setMeshOptimize(elem);

        // auto gameObject = meshRender->getGameObject();
        // auto &transform = gameObject->getTransform();

        // for (auto &child : transform.getChildren()) {
        //     auto obj = child->getGameObject();
        //     if (obj->hasComponent<MeshRender>()) {
        //         obj->get
        //     }
        // }
    }

    for (auto &material : meshRender->getMaterials()) {
        if (ImGui::TreeNode(material.getName().c_str())) {
            
            auto& albedo = material.getAlbedo();
            {
                ImGui::SeparatorText("Albedo");
                auto texture = material.getTexture(0);

                if (texture != nullptr && texture->isValid()) {
                    ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), {Settings::IconSize.x, Settings::IconSize.y});
                    if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                        texture = nullptr;
                    }
                } else {

                    if (View::TextureInputUI(ImColor(albedo.x, albedo.y, albedo.z, albedo.w))) {
                        ImGui::OpenPopup("#PopupChangeColorAlbedo");
                    }
                }

                /*
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                        std::string path((char*)data->Data, data->DataSize);
                        if (!path.empty()) {
                            material.texture[TexturePBR::ALBEDO] = Resources::Load<Texture>(path);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                */
            }

            Color clr = {albedo.x, albedo.y, albedo.z, albedo.w};
            if (View::PopupChangeColor("Albedo", clr)) {
                material.setAlbedo({clr.r, clr.g, clr.b, clr.a});
            }

            ImGui::TreePop();
        }
    }

    ImGui::TreePop();
}

void Editor::OnPropertyAnimator(Animator *animator) {
    auto &m_current = !animator->m_animation ? animator->m_animation : animator->m_nextAnimation;

    if (ImGui::TreeNode("Animator")) {
        auto &armature = animator->m_armature;

        if (armature) {
            ImGui::Text("Armature: %s", armature->getName().c_str());
        } else {
            ImGui::Text("Draging");
        }

        if (ImGui::BeginDragDropTarget()) {
            ImGuiDragDropFlags target_flags = 0;
            target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
            target_flags |= ImGuiDragDropFlags_SourceAllowNullID;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_GAMEOBJECT, target_flags)) {
                std::string name((char*)payload->Data, payload->DataSize);
                if (!name.empty()) {
                    armature = GameObject::Find(name);
                }
            }
            ImGui::EndDragDropTarget();
        }

        auto isOpen = ImGui::TreeNode("Animations");

        if (ImGui::BeginDragDropTarget()) {
            ImGuiDragDropFlags target_flags = 0;
            // target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
            // target_flags |= ImGuiDragDropFlags_SourceAllowNullID;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_FILE, target_flags)) {
                std::string path((char*)payload->Data, payload->DataSize);
                if (!path.empty() && std::filesystem::path(path).filename().extension() == ".anm") {
                    animator->insert(AssetsManager::LoadAnimation(path));
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (isOpen) {
            auto &animations = animator->m_animations;
            for (auto &&[name, animation] : animations) {
                ImGui::Selectable(name.c_str(), m_current == animation);
                if (ImGui::IsItemClicked()) {
                    m_current = animation;
                }
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}

void Editor::OnPropertyLight(Light *light) {
    if (ImGui::TreeNode("Light")) {
        int elem = Light::LightType::eDirection;
        
        const char* elems_names[3] = {"Directional", "Point", "Spot"};
        
        const char* elem_name = (elem >= 0 && elem < 3) ? elems_names[elem] : "Unknown";

        ImGui::Text("Type:");
        if (ImGui::SliderInt("##LightType", &elem, 0, 3 - 1, elem_name)) {
            light->setType(elem);
        }
        
        ImGui::Text("Constante:");
        ImGui::DragFloat("##constant", &light->m_attenuationConstant, 0.005f, 0.01f, 1.0f);

        ImGui::Text("Linear:");
        ImGui::DragFloat("##linear", &light->m_attenuationLinear, 0.005f, 0.01f, 1.0f);

        ImGui::Text("Quadract:");
        ImGui::DragFloat("##quadract", &light->m_attenuationQuadratic, 0.005f, 0.01f, 1.0f);

        ImGui::Text("InConner:");
        ImGui::DragFloat("##inconer", &light->m_angleInnerCone, 0.005f, 0.01f, 1.0f);

        ImGui::Text("IuterConner:");
        ImGui::DragFloat("##outerconner", &light->m_angleOuterCone, 0.005f, 0.01f, 1.0f);

        ImGui::Text("Cor:");
        ImVec4 color = {light->m_color.r, light->m_color.g, light->m_color.b, 1.0f};
        if (ImGui::ColorButton("##LightColor", color)) {
            ImGui::OpenPopup("##LightColorPopup");
        }
        if (ImGui::BeginPopup("##LightColorPopup")) {
            ImGui::ColorPicker4("#LightColorPick", light->m_color.data);
            ImGui::EndPopup();
        }

        ImGui::TreePop();
    }
}

void Editor::TextureInfo(Texture *&texture) {
    bool IsOpen = true;

    const ImVec2 textureSize(100, 100);
    const ImVec2 window(300, 400);

    ImGui::SetNextWindowSize(window, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2((Screen::width / 2) - (window.x / 2), (Screen::height / 2) - (window.y / 2)), ImGuiCond_Appearing);
    ImGui::Begin("Texture Info", &IsOpen);

    const float width = ImGui::GetWindowWidth();

    // ImGui::SetCursorPosX((width / 2) - (textureSize.x / 2));
    // ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), textureSize);

    // ImGui::Spacing();

    // ImGui::SeparatorText("Path");
    // ImGui::Text("%s", texture->getpath().c_str());

    // ImGui::SeparatorText("Dimensões");
    // ImGui::Text("Width: %d", texture->getwidth());
    // ImGui::Text("Height: %d", texture->getheight());

    ImGui::SeparatorText("Configurações");
    // ImGui::Text("Espaço de cor: %s", Resources::gettextureformatname(texture->getinternalformat()));

    if (!ImGui::IsWindowFocused()) {
        texture = nullptr;
    }
    ImGui::End();

    if (!IsOpen) {
        texture = nullptr;
    }
}

void Editor::LoadReources() {
    iconsUI.emplace(IconType::ICON_FILE,        AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-file-30.png", true)));
    iconsUI.emplace(IconType::ICON_FOLDER,      AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-folder-50.png", true)));
    iconsUI.emplace(IconType::ICON_BACK_FILE,   AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-go-back-50.png", true)));
    iconsUI.emplace(IconType::ICON_GAMEOBJECT,  AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-3d-object-50.png", true)));
    iconsUI.emplace(IconType::ICON_PENCIL,      AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-lápis-30.png", true)));
    iconsUI.emplace(IconType::ICON_LIGHT,       AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-lâmpada-globular-64.png", true)));
    iconsUI.emplace(IconType::ICON_TEXTURE,     AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-textura-de-renderização-32.png", true)));
    iconsUI.emplace(IconType::ICON_PNG,         AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-png-24.png", true)));
    iconsUI.emplace(IconType::ICON_PLAY_GAME,   AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-reproduzir-30.png", true)));
    iconsUI.emplace(IconType::ICON_STOP_GAME,   AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-parar-50.png", true)));
    iconsUI.emplace(IconType::ICON_BIN,         AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-resíduos-50.png", true)));
    iconsUI.emplace(IconType::ICON_IMPORT,      AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-importar-50.png", true)));
    iconsUI.emplace(IconType::ICON_ASCENDENTE,  AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-ordenar-para-a-direita-50.png", true)));
    iconsUI.emplace(IconType::ICON_DESCENDENTE, AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-ordem-descendente-50.png", true)));
    iconsUI.emplace(IconType::ICON_EDIT,        AssetsManager::LoadTexture(File::GetAsset("/icon/icons8-editar-24.png", true)));
    iconsUI.emplace(IconType::ICON_ANIMATION,   AssetsManager::LoadTexture(File::GetAsset("/icon/motion-graphics.png", true)));
    iconsUI.emplace(IconType::ICON_SHADER,      AssetsManager::LoadTexture(File::GetAsset("/icon/shade.png", true)));
    iconsUI.emplace(IconType::ICON_SCENE,       AssetsManager::LoadTexture(File::GetAsset("/icon/studio-set.png", true)));

    s_texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];
}

void Editor::ProgressLoad() {
    const ImVec2 windowSize = {300, 200};

    ImVec2 displaySize = s_io->DisplaySize;
    displaySize.x = (displaySize.x / 2.0f) - windowSize.x / 2.0f;
    displaySize.y = (displaySize.y / 2.0f) - windowSize.y / 2.0f;

    ImGui::SetNextWindowPos(displaySize);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("GameRender...", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove| ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("Carregando UI");

    if (!s_maxProgress) {
        s_maxProgress = AssetsManager::m_processCount;
    }

    grm::u32 current = AssetsManager::m_processCount;
    if (current > s_maxProgress) {
        s_maxProgress = current;
    }

    ImGui::ProgressBar(1.0f - (float)current / s_maxProgress, ImVec2(windowSize.x, 16));

    ImGui::Text("%d/%d", current, s_maxProgress);

    if (current <= 1) {
        s_maxProgress = 0;
    }
    ImGui::End();
}
