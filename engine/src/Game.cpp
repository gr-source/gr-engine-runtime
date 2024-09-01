#include "Game.h"

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

#include "core/statistics.hpp"
#include "core/resources.h"
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

#include "filesystem/filesystem.h"
#include "filesystem/stream.h"

#include "scriptManager/scriptManager.hpp"

#include "geometry/batching.hpp"

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
};

std::unordered_map<IconType, grr::gTexture *> iconsUI;

std::vector<std::unique_ptr<Filmbox>> m_filmboxList;

/* Seleciona um objeto no scenes(UI)*/
GameObject *Game::s_objectSelected = nullptr;

/* Textura do play button */
grr::gTexture *Game::s_texturePlayGame = nullptr;

ImGuiIO* Game::s_io = nullptr;

bool Game::bFileDoubleClick = false;

bool Game::bCutAllFile = false;

bool Game::bCopyAllFile = false;

bool Game::bViewPortFocused = false;

bool Game::bOpenPopupInfoDir = false;

bool Game::bScriptReloaded = false;

bool Game::bOpenPopupOptionGameObject = false;

ImVec4 Game::FPSColor = {255, 255, 255, 255};

ImGuizmo::OPERATION Game::m_guizmoOperation = ImGuizmo::TRANSLATE;

ImGuizmo::MODE Game::m_guizmoManipulate = ImGuizmo::WORLD;

grm::u32 Game::s_maxProgress = 0;

grm::u8 Game::m_currentPage = 0;

std::unordered_map<std::filesystem::path, bool> Game::fLisDirSelected;

std::filesystem::path Game::m_currentFile;

void Game::Initialize() {
    s_io = &ImGui::GetIO();

    loadReources();

    Settings::IsGameRunning = false;

    m_currentFile = Filesystem::GetFile("");

    Scene::LoadScene("MainScene");
}

// ===================================== WARNING =========================================================
#include "core/binary/gHeader.hpp"

void WriteNode(std::ofstream &os, const FbxScene *scene, const FbxNode &node) {
    // Escreve os dados do modelo
    gModel pModel;
    std::strcpy(pModel.m_name, node.getName().c_str());

    pModel.m_scale = node.getScale();
    pModel.m_rotation = node.getRotation();
    pModel.m_position = node.getPosition();

    if (!os.write(reinterpret_cast<const char *>(&pModel), sizeof(gModel))) {
        return;
    }

    auto &meshIndex = node.getMeshes();

    // escreve a quantidade de mesh
    grm::u32 meshCount = static_cast<grm::u32>(meshIndex.size());
    if (!os.write(reinterpret_cast<const char *>(&meshCount), sizeof(grm::u32))) {
        std::cerr << "Failed to write mesh count to file." << std::endl;
        return;
    }

    // alloca as meshes na memória em forma de pilha (fácil acesso)
    std::vector<gMesh> meshes(meshCount);
    std::vector<gMaterial> pMateials;

    for (grm::u32 i = 0; i < meshCount; ++i) {
        auto &fbxMesh = scene->getMesh(meshIndex[i]);

        for (const auto &mIdx : fbxMesh.getMaterials()) {
            auto& fbxMaterial = scene->getMaterial(mIdx);

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
        std::strcpy(pMesh.m_name, fbxMesh.getName().c_str());

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
    for (const auto &mesh : meshes) {
        if (!os.write(reinterpret_cast<const char *>(mesh.m_name), sizeof(char) * 256) ||
            !os.write(reinterpret_cast<const char *>(&mesh.m_hasBone), sizeof(bool)) ||
            !os.write(reinterpret_cast<const char *>(&mesh.m_verticeCount), sizeof(grm::u32)) ||
            !os.write(reinterpret_cast<const char *>(mesh.m_vertices), mesh.m_verticeCount * sizeof(gVertex)) ||
            !os.write(reinterpret_cast<const char *>(&mesh.m_indicesCount), sizeof(grm::u32)) ||
            !os.write(reinterpret_cast<const char *>(mesh.m_indices), mesh.m_indicesCount * sizeof(grm::u32)) ||
            !os.write(reinterpret_cast<const char *>(&mesh.m_boneCount), sizeof(grm::u32)) ||
            !os.write(reinterpret_cast<const char *>(mesh.m_bones), mesh.m_boneCount * sizeof(gBone))
            ) {
            std::cerr << "Failed to write mesh data to file." << std::endl;
            return;
        }
    }

    grm::u32 mSize = pMateials.size();
    if (!os.write(reinterpret_cast<const char *>(&mSize), sizeof(grm::u32))) {
        std::cout << "Falied to wite mateials." << std::endl;
        return;
    }

    for (auto &material : pMateials) {
        os.write(reinterpret_cast<const char *>(material.m_name), 256 * sizeof(char));
        os.write(reinterpret_cast<const char *>(&material.m_color), sizeof(Color));
        os.write(reinterpret_cast<const char *>(&material.m_roughness), sizeof(float));
        os.write(reinterpret_cast<const char *>(&material.m_metallic), sizeof(float));
        os.write(reinterpret_cast<const char *>(&material.m_ambientOcclusion), sizeof(float));

        os.write(reinterpret_cast<const char *>(&material.m_textureCount), sizeof(grm::u32));
        os.write(reinterpret_cast<const char *>(material.m_textures), material.m_textureCount * sizeof(gTexture));

        delete[] material.m_textures;
    }

    // Liberar a memória alocada
    for (auto &mesh : meshes) {
        delete[] mesh.m_vertices;
        delete[] mesh.m_indices;

        if (mesh.m_hasBone) {
            delete[] mesh.m_bones;
        }
    }

    auto &children = node.getChildren();

    grm::u32 nSize = children.size();
    os.write(reinterpret_cast<const char *>(&nSize), sizeof(grm::u32));

    // escreve os filhos
    for (const auto &child : children) {
        WriteNode(os, scene, child);
    }
}

void WriteAnimation(const FbxScene *fbxScene) {
    auto &animations = fbxScene->getAnimations();

    gHeader pHeader{{'g', 'r', 'a', 'n'}, 1.0f};

    for (auto &animation : animations) {
        auto name = animation.getName();

        if (!Stream::open(Filesystem::GetFile("/animation/" + name + ".anm"), std::ios_base::out | std::ios_base::binary)) {
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
    Console::log(LogInfo::INFO, "Writing model...");

    WriteAnimation(scene);

    auto path = Filesystem::GetFile("/model/" + node.getName() + ".gmd");

    std::ofstream os(path, std::ios_base::binary);
    if (!os.is_open()) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }

    gHeader pHeader {{'g', 'r', 'm', 'd'}, 1.0f};
    if (!os.write(reinterpret_cast<const char *>(&pHeader), sizeof(gHeader))) {
        std::cerr << "Failed to write header to file." << std::endl;
        return;
    }

    WriteNode(os, scene, node);

    os.close();
}

void PostProcess(const FbxScene *scene, Filmbox *filmbox) {
    auto& fRootNode = scene->getRootNode();
    for (const auto& node : fRootNode.getChildren()) {
        WriteHeader(scene, node);
    }
    delete filmbox;
}

void Game::DropFile(const std::filesystem::path &path) {
    if (path.extension() != ".fbx") {
        return;
    }
    Console::log(LogInfo::INFO, "Loading fbx: " + path.extension().string());

    auto *filmbox = new Filmbox(path.string());
    filmbox->setPostProcess(PostProcess);
    filmbox->load();
}

void Game::KeyBoardUp(grm::u32 key) {
    if (!(bViewPortFocused && key == KEY_ESC)) {
        return;
    }

    if (s_io)
        s_io->MouseDrawCursor = !s_io->MouseDrawCursor;
}

// ===================================== END =========================================================
void Game::RenderUI() {
    if (Resources::m_processCount != 0) {
        ProgressLoad();
        return;
    }
    
    RenderDocking();

    RenderScenesUI();

    RenderViewportUI();

    RenderConsoleUI();

    RenderPropertiUI();

    if (s_io->KeyCtrl && ImGui::IsKeyReleased(ImGuiKey_S)) {
        Scene::Save();
    }
}

void Game::RenderDocking() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("GR - GameEngine 3D", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
    if (s_io->ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGui::DockSpace(ImGui::GetID("##RenderDocking"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void Game::RenderScenesUI() {
    auto isOpen = ImGui::Begin("Cenario", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if (!isOpen) {
        return;
    }

    auto name = Scene::GetSceneName();

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
                    
                    Scene::Add(target);
                }
            }
            ImGui::EndDragDropTarget();
        }
        
        // Render objects with scene
        {
            auto &list = Scene::GetGameObjects();

            if (m_currentPage > 0 && ImGui::Button("Before")) {
                --m_currentPage;
            }
            
            RenderPage(list, m_currentPage);
            
            if (list.size() >= m_currentPage && ImGui::Button("Next")) {
                ++m_currentPage;
            }
        }
        
        ImGui::TreePop();
    }

    if (ImGui::IsKeyReleased(ImGuiKey_Delete) && s_objectSelected) {
        Event::Add(Scene::Delete, s_objectSelected);
        s_objectSelected = nullptr;
    }
    
    if (ImGui::IsKeyReleased(ImGuiKey_F2) && ImGui::IsWindowFocused()) {
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

void Game::RenderPage(std::vector<GameObject *> &list, int page) {
    int start = page * ITEMS_PER_PAGE;
    int end = std::min(start + ITEMS_PER_PAGE, static_cast<int>(list.size()));

    for (int i = start; i < end; ++i) {
        auto& transform = list[i]->getTransform();

        ReadHierarchy(&transform);
    }
}

void Game::ReadHierarchy(Transform *transform) {
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
                    
                        Event::Add(&Scene::Remove, target);
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

void Game::PopupMenuAdd() {
    if (ImGui::BeginPopup("##popup_menu_add")) {
        ImGui::Text("Add Object");

        const char* item_name[1]{"Empty GameObject"};

        auto id = 0;
        if (ImGui::ListBox("##ListBoxOptionsGame", &id, item_name, 1)) {
            if (id == 0) {
                s_objectSelected = &Scene::CreateGameObject("Empty");
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Game::PopupOptionsGameObject() {
    bool isOpenRename = false;
    if (ImGui::BeginPopup("##popup_options_gameobject")) {
        const char* item_name[2] {"Delete", "Rename"};

        auto id = 0;
        if (ImGui::ListBox("##ListBoxOptionsGame", &id, item_name, 2)) {
            if (id == 0 && s_objectSelected) {
                Event::Add(Scene::Delete, s_objectSelected);
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

void Game::PopupRenameGameObject() {
    if (ImGui::BeginPopup("##popup_rename_gameobject")) {
        ImGui::Text("Input new name");

        if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
            ImGui::SetKeyboardFocusHere(0);

        char name[256] = {0};
        if (ImGui::InputText("##InputName", name, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
            GameObject::Rename(s_objectSelected, name);

            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Game::RenderViewportUI() {
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    bViewPortFocused = ImGui::IsWindowFocused();

    ImGuizmo::SetDrawlist();

    ImVec2 size = ImGui::GetWindowSize();

    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, size.x, size.y);

    static ImVec2 lastSize = ImGui::GetContentRegionAvail();
    ImVec2 curSize = ImGui::GetContentRegionAvail();

    if (lastSize.x != curSize.x || lastSize.y != curSize.y && curSize.x > 5.0f && curSize.y > 5.0f || bScriptReloaded) {
        auto &viewport = Settings::ViewportSize = {static_cast<int>(curSize.x), static_cast<int>(curSize.y)};

        if (ScriptManager::SetScreenSize) {
            ScriptManager::SetScreenSize(Settings::ViewportSize);
        }

        auto camera = Camera::Current();
        if (camera) {
            camera->setAspectRatio(static_cast<float>(viewport.x) / static_cast<float>(viewport.y));
        }

        PostProcessing::Resize(Engine::MainPostProcess, viewport);
        
        lastSize = curSize;
        bScriptReloaded = false;
    }

    auto texture = PostProcessing::GetTextureColor(Engine::ImGuiPostProcess);
    if (texture) {
        ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), curSize, ImVec2(0, 1), ImVec2(1, 0));
    }

    if (ImGui::BeginDragDropTarget()) {
        ImGuiDragDropFlags target_flags = 0;
        // target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
        // target_flags |= ImGuiDragDropFlags_SourceAllowNullID;

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_FILE, target_flags)) {
            std::string path((char*)payload->Data, payload->DataSize);
            if (!path.empty() && std::filesystem::path(path).filename().extension() == ".gmd") {
                s_objectSelected = Resources::LoadGameObject(path);
            }
        }
        ImGui::EndDragDropTarget();
    }

    // ImGui::SetCursorPos({curSize.x / 2.0f, curSize.y / 2.0f});
// 
    // ImGui::Text("X");

    if (s_objectSelected != nullptr) {
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

        auto model = transform.getGlobalMatrix();

        auto camera = Camera::Current();

        if (ImGuizmo::Manipulate(camera->getViewmatrix().data, camera->getPerspesctive().data, m_guizmoOperation, m_guizmoManipulate, model.data)) {
            Vector3 s;
            Quaternion r;
            Vector3 t;
            model.decompose(s, r, t);

            transform.setLocalScale(s);
            transform.setLocalRotation(r);
            transform.setLocalPosition(t);
        }
    }

    const ImVec2 wSize =  ImGui::GetCursorPos();

    // Play button
    {
        ImGui::SetCursorPos(ImVec2(curSize.x / 2 - wSize.x, 32));

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(s_texturePlayGame->getID()), ImVec2(16, 16))) {
            Settings::IsGameRunning =! Settings::IsGameRunning;

            if (Settings::IsGameRunning) {
                s_texturePlayGame = iconsUI[IconType::ICON_STOP_GAME];

                StopGame();

                s_objectSelected = nullptr;
            } else {
                s_texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];

                PlayGame();

                bScriptReloaded = true;
            }
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

        auto curtorPos = ImVec2(curSize.x - textSize.x, textSize.y * 2.5f);

        ImGui::SetCursorPos(curtorPos);

        ImGui::TextColored(FPSColor, "FPS: %.2f", TimeLast::FPS);
    }

    ImGui::SetCursorPos({8.0f, 35.0f});

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

    ImGui::End();
}

bool bPopupOpen = false;

void Game::RenderFilesUI() {
    grr::gTexture *gtCurrentTexture = iconsUI[ICON_BACK_FILE];

    ImGui::BeginColumns("##colums_01", 5, ImGuiColumnsFlags_NoBorder);
    if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(gtCurrentTexture->getID()), ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0)) && m_currentFile.has_parent_path()) {
        m_currentFile = m_currentFile.parent_path();

        if (!bCutAllFile && !bCopyAllFile) {
            fLisDirSelected.clear();
        }
    }

    bool bSelelectAll = false;

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
                Filesystem::Copy(path, m_currentFile);
            else if (bCutAllFile)
                Filesystem::Move(path, m_currentFile);
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
            }
            if (extension == ".anm") {
                gtCurrentTexture = iconsUI[ICON_ANIMATION];
            }
            if (extension == ".frag" || extension == ".vert") {
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

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(gtCurrentTexture->getID()), ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0), -1, {0.0f, 0.0f, 0.0f, 0.0f}, tintColor) && !s_io->KeyShift) {
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

    if (bPopupOpen)
        PopupDirmanager();

    PopupFileManager();

    PopupCreateDirectory();
}

void Game::PopupDirmanager() {
    if (ImGui::BeginPopupModal("##popup_dir_manager", &bPopupOpen)) {
        for (auto &&[path, isSeleceted] : fLisDirSelected) {
            ImGui::Text("Dir: %s", path.filename().string().c_str());
        }

        auto idx = 0;

        const char *item_list[2] {"Delete"};

        if (ImGui::ListBox("##DirManager", &idx, item_list, 1)) {
            switch (idx) {
            case 0:
                for (auto &&[path, isSeleceted] : fLisDirSelected) {
                    Filesystem::Delete(path);
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

    if (!bPopupOpen) {
        fLisDirSelected.clear();
    }

    // ImGui::
}

void Game::PopupFileManager() {
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

void Game::PopupCreateDirectory() {
    if (ImGui::BeginPopup("##popup_create_new_dir")) {
        ImGui::Text("Input directory name:");
        if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
            ImGui::SetKeyboardFocusHere(0);

        char name[256] = {0};
        if (ImGui::InputText("##InputDirectoryName", name, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
            Filesystem::CreateDirectory(m_currentFile.string() + "/" + std::string(name));
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Game::RenderConsoleUI() {
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

void Game::RenderPropertiUI() {
    auto isOpen = ImGui::Begin("Propriedades", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if (!isOpen) {
        return;
    }

    if (ImGui::BeginTabBar("##propertimenubar")) {
        RenderGameObjectPropertiUI();
        
        if (ImGui::BeginTabItem("Estatística", nullptr)) {
            ImGui::SeparatorText("Scripts");
            
            if (ScriptManager::GetEntityCount)
                ImGui::Text("Entidades: %ld", ScriptManager::GetEntityCount());

            if (GameObject::GetGameObjectCount)
                ImGui::Text("GameObject: %ld", GameObject::GetGameObjectCount());

            ImGui::Text("Memoria: %s", getfilesize(Memory::totalMemoryUsage).c_str());

            ImGui::SeparatorText("Cena");
            ImGui::Text("Vertices: %d | %s", gameStatics.vertices, getfilesize(gameStatics.vertices * sizeof(Vertex3D)).c_str());
            ImGui::Text("Indice: %d | %s", gameStatics.indices, getfilesize(gameStatics.indices * sizeof(grm::u32)).c_str());
            ImGui::Text("Triangulos: %.2f", (gameStatics.indices / 3.0f));

            ImGui::SeparatorText("AssetsManager");
            ImGui::Text("Textures: %ld", Resources::GetTextureCount());
            ImGui::Text("Shaders: %ld", Resources::GetShaderCount());
            ImGui::Text("Mesh: %ld", Resources::GetMeshCount());
            ImGui::Text("Event: %ld", Event::GetCount());

            ImGui::SeparatorText("Threads");
            ImGui::Text("Update:  %fms  | FPS: %.2f",  gameStatics.updateMs,  TimeLast::FPS);
            ImGui::Text("Render:  %fms  | FPS: %.2f",  gameStatics.renderMs,  TimeLast::RenderFPS);
            ImGui::Text("Physics: %fms  | FPS: %.2f",  gameStatics.physicsMs, (1.0f / gameStatics.physicsMs));
            ImGui::Text("Script:  %fms  | FPS: %.2f",  gameStatics.scriptMS,  (1.0f / gameStatics.scriptMS));

            ImGui::SeparatorText("Renderização");
            // ImGui::Text("Cameras: %ld",   Camera::Get.size());
            if (ScriptManager::GetListLight)
                ImGui::Text("Luzes: %ld",     ScriptManager::GetListLight().size());
            ImGui::Text("Draw Call Count: %ld", Statistics::m_drawCallCount);
            ImGui::Text("Draw Render: %ld/%ld", Statistics::m_totalRender, Statistics::m_drawRenderCount);
            
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

void Game::RenderGameObjectPropertiUI() {
    auto isOpenPopup = false;
    if (!s_objectSelected || !ScriptManager::GetComponentPool) {
        return;
    }

    auto &componentPool = ScriptManager::GetComponentPool();

    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    if (ImGui::BeginTabItem("GameObject", nullptr)) {
        ImGui::Image(reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_GAMEOBJECT]->getID()), ImVec2(16, 16));

        ImGui::SameLine();

        ImGui::Text("[ %s - %s ]", s_objectSelected->getName().c_str(), s_objectSelected->getTag().c_str());

        s_objectSelected->RenderProperty<Transform>(Game::OnPropertyTransform);
        s_objectSelected->RenderProperty<Light>(Game::OnPropertyLight);

        for (auto &&[name, ptr] : s_objectSelected->getEntity()->getComponents()) {
            if (name == "Transform" || name == "Light") {
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

void Game::RenderField(Field &field, Component *component) {
    auto name = field.getName();
    auto type = field.getType();

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

void Game::PopupAddComponent() {
    if (!ScriptManager::GetComponentPool && !ScriptManager::CreateComponentWithName && !s_objectSelected) {
        return;
    }
    if (ImGui::BeginPopup("##popup_add_component")) {
        auto &componentPool = ScriptManager::GetComponentPool();

        for (auto &&[name, pool] : componentPool) {
            if (ImGui::SmallButton(name.c_str())) {
                s_objectSelected->getEntity()->addInternalComponent(name);

                ImGui::CloseCurrentPopup();
            }
        }
        
        ImGui::EndPopup();
    }
}

/* ********** Propriedades ********** */
void Game::OnPropertyTransform(Transform* transform) {
    if (ImGui::TreeNode("Transform")) {
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

        Vector3 euler = Math::eulerAngles(transform->getLocalRotation());

        ImGui::Text("Rotação:");
        if (ImGui::DragFloatEx("##Rotacao", text, euler.data, 1.0f, -360.0f, 360.0f)) {
            transform->setLocalRotation(Math::normalize(Math::euler({Math::rad(euler.x), Math::rad(euler.y), Math::rad(euler.z)})));
        }

        ImGui::TreePop();
    }
}

void Game::OnPropertyAnimator(Animator *animator) {
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
                    animator->insert(Resources::LoadAnimation(path));
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

void Game::OnPropertyMeshRender(MeshRender *meshRender) {
    if (ImGui::TreeNode("MeshRender")) {
        for (auto&& [idx, material] : meshRender->getMaterials()) {
            if (ImGui::TreeNode(std::string(material.getName() + "(" + std::to_string(idx) + ")").c_str())) {
                
                auto& albedo = material.getAlbedo();
                {
                    ImGui::SeparatorText("Albedo");
                    auto texture = material.getTexture(0);

                    if (texture != nullptr && texture->isValid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), ImVec2(32, 32));
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
}

void Game::OnPropertyLight(Light *light) {
    if (ImGui::TreeNode("Light")) {
        static int elem = Light::LightType::eDirection;
        
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

void Game::TextureInfo(Texture *&texture) {
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

void Game::PlayGame() {
    ScriptManager::Initialize();

    if (ScriptManager::SetScreenSize) {
        ScriptManager::SetScreenSize(Settings::ViewportSize);
    }

    RenderManager::Initialize();

    ScreenRender::Initialize();
}

void Game::StopGame() {
    Scene::Clear();

    RenderManager::Shutdown();

    ScreenRender::Shutdown();

    ScriptManager::Shutdown();
}

void Game::loadReources() {
    iconsUI.emplace(IconType::ICON_FILE,        Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-file-30.png")));
    iconsUI.emplace(IconType::ICON_FOLDER,      Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-folder-50.png")));
    iconsUI.emplace(IconType::ICON_BACK_FILE,   Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-go-back-50.png")));
    iconsUI.emplace(IconType::ICON_GAMEOBJECT,  Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-3d-object-50.png")));
    iconsUI.emplace(IconType::ICON_PENCIL,      Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-lápis-30.png")));
    iconsUI.emplace(IconType::ICON_LIGHT,       Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-lâmpada-globular-64.png")));
    iconsUI.emplace(IconType::ICON_TEXTURE,     Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-textura-de-renderização-32.png")));
    iconsUI.emplace(IconType::ICON_PNG,         Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-png-24.png")));
    iconsUI.emplace(IconType::ICON_PLAY_GAME,   Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-reproduzir-30.png")));
    iconsUI.emplace(IconType::ICON_STOP_GAME,   Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-parar-50.png")));
    iconsUI.emplace(IconType::ICON_BIN,         Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-resíduos-50.png")));
    iconsUI.emplace(IconType::ICON_IMPORT,      Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-importar-50.png")));
    iconsUI.emplace(IconType::ICON_ASCENDENTE,  Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-ordenar-para-a-direita-50.png")));
    iconsUI.emplace(IconType::ICON_DESCENDENTE, Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-ordem-descendente-50.png")));
    iconsUI.emplace(IconType::ICON_EDIT,        Resources::LoadTexture(Filesystem::GetFile("/icon/icons8-editar-24.png")));
    iconsUI.emplace(IconType::ICON_ANIMATION,   Resources::LoadTexture(Filesystem::GetFile("/icon/motion-graphics.png")));
    iconsUI.emplace(IconType::ICON_SHADER,      Resources::LoadTexture(Filesystem::GetFile("/icon/shade.png")));

    s_texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];
}

void Game::ProgressLoad() {
    const ImVec2 windowSize = {300, 200};

    ImVec2 displaySize = s_io->DisplaySize;
    displaySize.x = (displaySize.x / 2.0f) - windowSize.x / 2.0f;
    displaySize.y = (displaySize.y / 2.0f) - windowSize.y / 2.0f;

    ImGui::SetNextWindowPos(displaySize);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("GameRender...", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove| ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("Carregando UI");

    if (!s_maxProgress) {
        s_maxProgress = Resources::m_processCount;
    }

    grm::u32 current = Resources::m_processCount;
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
