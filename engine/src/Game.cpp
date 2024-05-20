#include "Game.h"

#include "time.h"
#include "screen.h"
#include "memory.h"
#include "skybox.h"
#include "settings.h"
#include "renderer.h"
#include "gameObject.h"
#include "eventManager.h"
#include "objectManager.h"
#include "batchingManager.h"
#include "editorController.h"

#include "framebuffer/shadowbuffer.h"
#include "framebuffer/screenbuffer.h"
#include "framebuffer/imguibuffer.h"
#include "framebuffer/thumbnails.h"

#include "resources/Resources.h"

#include "ecs/EntityManager.h"

#include "component/playerController.h"
#include "component/cameraController.h"
#include "component/cameraComponent.h"
#include "component/lightComponent.h"
#include "component/meshCollider.h"
#include "component/boxCollider.h"
#include "component/MeshRender.h"
#include "component/Transform.h"
#include "component/Rigidbody.h"
#include "component/animator.h"

#include "guizmo/ImGuizmo.h"

#include "ui/ImGuiRender.h"

#include "core/scene/SceneManager.h"
#include "core/scene/Scene.h"

#include "ecs/EntityManager.h"

#include "filesystem/filesystem.h"

#include "ui/View.h"

#include <sstream>

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

#if !IS_GAME
enum class IconType : int {
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
	ICON_EDIT         = 14
};

std::unordered_map<IconType, Sprite*> iconsUI;

/* editor da game engine */
EditorController mEditorController;
#endif

std::unique_ptr<Filmbox> m_filmbox = nullptr;

GameObject& GenGameObject(const FbxNode& node, bool local) {
    GameObject& gameObject = ObjectManager::add(node.getname());

    Transform& transform = gameObject.getComponent<Transform>();

    // if (local) {
        // transform.localScale = node.scale;
        // transform.localRotation = node.rotation;
        // transform.localPosition = node.position;
    // } else {
        transform.scale = node.scale;
        transform.rotation = node.rotation;
        transform.position = node.position;

        if (!local)
        SceneManager::Add(&gameObject);
    // }

    if (node.hasmesh()) {
        gameObject.addComponent<MeshRender>(node.getmeshs(), node.materials);
    }
    return gameObject;
}

void CreateLight(GameObject& gameobject, const FbxNode& node, const FbxScene& scene) {
    const FbxLight* fbxLight = scene.findlight(node.getname());

    // forward e up removido
    LightComponent& light = gameobject.addComponent<LightComponent>(fbxLight->gettype(), fbxLight->getcolor(), fbxLight->getAttenuationConstant(), fbxLight->getAttenuationLinear(), fbxLight->getAttenuationQuadract(), fbxLight->getInneCone(), fbxLight->getOuterCone());
}

void CreateCamera(GameObject& gameobject, const FbxNode& node, const FbxScene& scene) {
    const FbxCamera* m_camera = scene.findcamera(node.getname());
    // forward e up removido
    CameraComponent& m_cameraComponent = gameobject.addComponent<CameraComponent>(m_camera->getfov(), m_camera->getnearplane(), m_camera->getfarplane(), m_camera->getaspect());

    if (gameobject.getname() == "Camera") {
        gameobject.addComponent<CameraController>();
    }
}

void CreateModel(GameObject& gameobject, const FbxNode& node, const FbxScene& scene) {
    Transform& transform = gameobject.gettransform();

    RigidbodyDef rigidbodyDef;
    rigidbodyDef.mass = 1.0f;

    if (gameobject.getname() == "player") {
        rigidbodyDef.isDynamic = true;

        gameobject.addComponent<BoxCollider>(transform.scale);

        gameobject.addComponent<Rigidbody>(rigidbodyDef);

        gameobject.addComponent<PlayerController>();
    } else if (gameobject.getname() == "ground") {
        rigidbodyDef.mass = 0.0f;

        gameobject.addComponent<BoxCollider>(transform.scale);

        gameobject.addComponent<Rigidbody>(rigidbodyDef);
    } else if (gameobject.getname() == "whall") {
        rigidbodyDef.mass = 0.0f;
        
        gameobject.addComponent<MeshCollider>(node.getmeshs()[0]);

        gameobject.addComponent<Rigidbody>(rigidbodyDef);
    } else if (gameobject.getname() == "Beta_Surface") {
        auto& animation = gameobject.addComponent<Animator>(node.getmeshs()[0].getskeleton());
        animation.m_animations.insert(scene.getanimations().begin(), scene.getanimations().end());
    }
}

GameObject& SetupNode(const FbxNode& node, const FbxScene& scene, Transform* transform) {
    GameObject& gameobject = GenGameObject(node, transform != nullptr);

    switch (node.gettype()) {
    case FbxNode::eLIGHT:
        CreateLight(gameobject, node, scene);
        break;
    case FbxNode::eCAMERA:
        CreateCamera(gameobject, node, scene);
        break;
    case FbxNode::eMODEL:
        CreateModel(gameobject, node, scene);
        break;
    default:
        break;
    }
    return gameobject;
}

void ReaderNode(const FbxNode& node, const FbxScene& scene, const FbxNode* parent = nullptr, Transform* transform = nullptr) {
    if (parent != nullptr) {
        GameObject& gameObject = SetupNode(node, scene, transform);

        if (transform == nullptr) {
            transform = &gameObject.gettransform();
        } else {
            transform->addparent(&gameObject);

            gameObject.gettransform().parent = transform;

            transform = &gameObject.gettransform();
        }
    }

    for (auto& child : node.children) {
        ReaderNode(child, scene, &node, transform);
    }
}

void SceneLoaded(const FbxScene& scene) {
    ReaderNode(scene.getroot(), scene, nullptr);

    #if IS_GAME
    Settings::IsGameRunning = true;
    EntityManager::OnPlayGame();
    #endif
    m_filmbox = nullptr;
}

Game::Game() : objectselected(nullptr) {
    m_filmbox = std::make_unique<Filmbox>("/home/grsource/Documentos/filmbox/anim.fbx");
    m_filmbox->SetLoaded(SceneLoaded);
    m_filmbox->load();

    #if !IS_GAME
    m_io = nullptr;
    m_guizmoOperation = ImGuizmo::TRANSLATE;
    m_guizmoManipulate = ImGuizmo::WORLD;

    m_io = &ImGui::GetIO();

    loadReources();

    Settings::IsGameRunning = false;
    #endif
}

void Game::Update() {
    #if !IS_GAME
    mEditorController.OnUpdate();
    #endif
}

#if !IS_GAME
void Game::Gui() {
    if (Resources::m_processCount != 0) {
        ProgressLoad();
        return;
    }
    
    if (ImGui::IsKeyReleased(ImGuiKey_S) && m_io->KeyCtrl) {
        SceneManager::Save(Filesystem::getpublic(Filesystem::eScene) + "/SceneMain.scn");
    }

    RenderDocking();

    RenderScenesUI();

    RenderViewportUI();

    RenderConsoleUI();

    RenderPropertiUI();
}

void Game::RenderDocking() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGuiIO& io = ImGui::GetIO();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("GR - GameEngine 3D", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGui::DockSpace(ImGui::GetID("##RenderDocking"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void Game::RenderScenesUI() {
    ImGui::Begin("Cenario", nullptr, ImGuiWindowFlags_NoResize);
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::TreeNodeEx(SceneManager::GetName().c_str(), ImGuiTreeNodeFlags_SpanFullWidth)) {
        for (auto& gameObject : SceneManager::GetObjects()) {
            ReadHierarchy(SceneManager::GetObjects(), gameObject);
        }
        ImGui::TreePop();
    }

    if (ImGui::IsKeyReleased(ImGuiKey_A) && m_io->KeyCtrl) {
        ImGui::OpenPopup("##AddNewObject");
    }

    if (ImGui::BeginPopup("##AddNewObject")) {
        MenuAdd();
        ImGui::EndPopup();
    }

    ImGui::End();
}

void Game::ReadHierarchy(std::vector<GameObject *>& list, GameObject* gameObject) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

    Transform& transform = gameObject->getComponent<Transform>();

    if (objectselected == gameObject) {
        flags |= ImGuiTreeNodeFlags_Selected;
    } else {
        flags &= ~ImGuiTreeNodeFlags_Selected;
    }
    
    if (!transform.hasparent()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    const bool isOpen = ImGui::TreeNodeEx(gameObject->name.c_str(), flags);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
        objectselected = gameObject;
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload("##GameObject", ImGuiDragDropFlags_None)) {
            GameObject* child = reinterpret_cast<GameObject*>(data->Data);
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        ImGui::SetTooltip("%s", gameObject->name.c_str());
        ImGui::SetDragDropPayload("##GameObject", gameObject->name.data(), gameObject->name.size(), ImGuiCond_Always);
        ImGui::EndDragDropSource();
    }

    if (isOpen) {
        for (auto& child : transform.getparents()) {
            ReadHierarchy(list, child);
        }
        ImGui::TreePop();
    }

}

void Game::RenderViewportUI() {
    static Imguibuffer& m_imguibuffer = Imguibuffer::getInstance();

    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGuizmo::SetDrawlist();
    float windowWidth = (float)ImGui::GetWindowWidth();
    float windowHeight = (float)ImGui::GetWindowHeight();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

    static ImVec2 lastSize = ImGui::GetContentRegionAvail();
    ImVec2 curSize = ImGui::GetContentRegionAvail();

    if (lastSize.x != curSize.x || lastSize.y != curSize.y) {
        Settings::ViewportSize = {curSize.x, curSize.y};

        m_imguibuffer.resize({curSize.x, curSize.y});
        lastSize = curSize;
    }
    
    ImGui::Image(reinterpret_cast<ImTextureID>(m_imguibuffer.gettexture()), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));

    CameraComponent& m_cameraComponent = CameraComponent::MainCamera();

    if (objectselected != nullptr) {
        if (ImGui::IsKeyReleased(ImGuiKey_T)) {
            m_guizmoOperation = ImGuizmo::TRANSLATE;
        }
        if (ImGui::IsKeyReleased(ImGuiKey_R)) {
            m_guizmoOperation = ImGuizmo::ROTATE;
        }
        if (ImGui::IsKeyReleased(ImGuiKey_S)) {
            m_guizmoOperation = ImGuizmo::SCALE;
        }
        if (ImGui::IsKeyReleased(ImGuiKey_Space)) {
            if (m_guizmoManipulate == ImGuizmo::WORLD) {
                m_guizmoManipulate = ImGuizmo::LOCAL;
            } else if (m_guizmoManipulate == ImGuizmo::LOCAL) {
                m_guizmoManipulate = ImGuizmo::WORLD;
            }
        }
        Transform& transform = objectselected->gettransform();

        Matrix4x4 model = transform.getglobalmatrix();

        if (ImGuizmo::Manipulate(m_cameraComponent.getviewmatrix().data, m_cameraComponent.getperspesctive().data, m_guizmoOperation, m_guizmoManipulate, model.data)) {
            model.decompose(transform.scale, transform.rotation, transform.position);
        }
    }

    const ImVec2 wSize =  ImGui::GetCursorPos();

    ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x / 2 - wSize.x, 32));
    if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(texturePlayGame->getID()), ImVec2(16, 16))) {
        Settings::IsGameRunning =! Settings::IsGameRunning;

        if (Settings::IsGameRunning) {
            texturePlayGame = iconsUI[IconType::ICON_STOP_GAME];

            EntityManager::OnStopPlayGame();
        } else {
            texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];

            EntityManager::OnPlayGame();
        }
    }

    ImGui::End();
}

void Game::RenderFilesUI() {
    static std::filesystem::path resources = Filesystem::getpublic(Filesystem::eResource);
    const ImVec2 iconSize = reinterpret_vector<ImVec2, Vector2>(Settings::IconSize);
    ImTextureID iconTexture = nullptr;
    Texture2D* m_texture = nullptr;
    static Texture2D* texture = nullptr;

    ImGui::Columns(5, "#Collums1", false);
    if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_BACK_FILE]->getID()), iconSize)) {
        if (resources != Filesystem::getpublic(Filesystem::eResource)) {
            resources = resources.parent_path();
        }
    }
    ImGui::Text("Voltar");

    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(resources)) {
        files.push_back(entry.path());
    }

    std::sort(files.begin(), files.end(), [](const std::filesystem::path& a, const std::filesystem::path& b) {
        return a.filename() < b.filename();
    });

    for (auto& entry : files) {
        ImGui::PushID(entry.string().c_str());
        ImGui::NextColumn();

        if (std::filesystem::is_directory(entry)) {
            if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_FOLDER]->getID()), iconSize)) {
                resources = entry;
            }
        } else {
            FileType fileType = getFileType(entry.extension());

            switch (fileType) {
                case FileType::OBJ:
                case FileType::GR:
                case FileType::GOT:
                    iconTexture = reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_GAMEOBJECT]->getID());
                    break;
                case FileType::JPG:
                case FileType::PNG: {
                    m_texture = Resources::Load<Texture2D>(entry.string());
                    if (!m_texture->isvalid()) {
                        iconTexture = reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_PNG]->getID());
                        break;
                    }
                    iconTexture = reinterpret_cast<ImTextureID>(m_texture->getID());
                    break;
                }
                default:
                    iconTexture = reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_FILE]->getID());
                    break;
            }

            if (ImGui::ImageButton(iconTexture, iconSize)) {
                texture = m_texture;
            }

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                const char* dragDropType = nullptr;

                switch (fileType) {
                    case FileType::OBJ:
                    case FileType::GR:
                    case FileType::GOT:
                        dragDropType = getDragDrop(DragDrop::GAMEOBJECT);
                        break;
                    case FileType::JPG:
                    case FileType::PNG:
                        dragDropType = getDragDrop(DragDrop::TEXTURE);
                        break;
                    case FileType::MT:
                        dragDropType = getDragDrop(DragDrop::MATERIAL);
                        break;
                    default:
                        break;
                }

                if (dragDropType) {
                    ImGui::SetDragDropPayload(dragDropType, entry.string().data(), entry.string().size());
                    ImGui::ImageButton(iconTexture, iconSize);
                }

                ImGui::EndDragDropSource();
            }
        }
        
        std::string filename = entry.filename().c_str();
        if (filename.length() > 5) {
            // Dividir em várias linhas
            int chunkSize = 7;
            for (size_t i = 0; i < filename.length(); i += chunkSize) {
                std::string chunk = filename.substr(i, chunkSize);
                ImGui::Text("%s", chunk.c_str());
            }
        } else {
            // Se for menor ou igual a 5, imprime em uma única linha
            ImGui::Text("%s", filename.c_str());
        }

        ImGui::PopID();
    }

    if (texture != nullptr) {
        TextureInfo(texture);
    }

    ImGui::Columns(1);
}

void Game::RenderConsoleUI() {
    static NavegationFlags flags = NavegationFlags_files;

    ImGui::Begin("Navegador", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar);
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

    if ((flags & NavegationFlags_files) == NavegationFlags_files) {
        RenderFilesUI();
    }

    if ((flags & NavegationFlags_models) == NavegationFlags_models) {
    }
    ImGui::End();
}

void Game::RenderPropertiUI() {
    static ImGuiIO& io = ImGui::GetIO();

    ImGui::Begin("Propriedades", nullptr, ImGuiWindowFlags_NoResize);

    if (ImGui::BeginTabBar("##propertimenubar")) {
        if (ImGui::BeginTabItem("Objeto", nullptr)) {

            if (objectselected != nullptr) {
                ImGui::Image(reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_GAMEOBJECT]->getID()), ImVec2(16, 16));

                ImGui::SameLine();

                ImGui::Text("[ %s - %s ]", objectselected->name.c_str(), objectselected->tag.c_str());

                objectselected->RenderProperty<Transform>(&Game::OnPropertyTransform, this);
                objectselected->RenderProperty<MeshRender>(&Game::OnPropertyMeshRender, this);
                objectselected->RenderProperty<CameraComponent>(&Game::OnPropertyCameraComponent, this);
                objectselected->RenderProperty<LightComponent>(&Game::OnPropertyLightComponent, this);
                objectselected->RenderProperty<Animator>(&Game::OnPropertyAnimator, this);
            }

            ImGui::EndTabItem();
        }
        
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        if (ImGui::BeginTabItem("Statisticas", nullptr)) {
            ImGui::SeparatorText("Scripts");
            ImGui::Text("Entidades: %d", EntityManager::getcount());
            ImGui::Text("Memoria: %s", getfilesize(Memory::totalMemoryUsage).c_str());

            ImGui::SeparatorText("Cena");
            ImGui::Text("Vertices: %d | %s", gameStatics.vertices, getfilesize(gameStatics.vertices).c_str());
            ImGui::Text("Indice: %d | %s", gameStatics.indices, getfilesize(gameStatics.indices).c_str());
            ImGui::Text("Eventos: %d", EventManager::getcount());
            ImGui::Text("Triangulos: %dK", (gameStatics.indices / 3));

            ImGui::SeparatorText("Resources");
            ImGui::Text("Sprites: %d", Resources::getspritecount());
            ImGui::Text("Textures: %d", Resources::gettexturecount());
            ImGui::Text("Shaders: %d", Resources::getshadercount());

            ImGui::SeparatorText("Threads");
            ImGui::Text("Update:  %fms  | FPS: %.2f",  gameStatics.updateMs,  (1.0f / gameStatics.updateMs));
            ImGui::Text("Render:  %fms  | FPS: %.2f",  gameStatics.renderMs,  (1.0f / gameStatics.renderMs));
            ImGui::Text("Physics: %fms  | FPS: %.2f",  gameStatics.physicsMs, (1.0f / gameStatics.physicsMs));
            ImGui::Text("Script:  %fms  | FPS: %.2f",  gameStatics.scriptMS,  (1.0f / gameStatics.scriptMS));

            ImGui::SeparatorText("Renderização");
            ImGui::Text("Objetos: %d/%d", BatchingManager::frustomDraw, BatchingManager::drawCount);
            ImGui::Text("Luzes: %ld",     LightComponent::m_lights.size());
            ImGui::Text("Cameras: %ld",   CameraComponent::m_cameras.size());
            
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Configuraçoes")) {
            ImGui::Text("Camera: %d", CameraComponent::CameraID);
            if (ImGui::Button("Change")) {
                CameraComponent::CameraID++;

                if (CameraComponent::CameraID >= CameraComponent::m_cameras.size()) {
                    CameraComponent::CameraID = 0;
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

/* ********** Propriedades ********** */
void Game::OnPropertyTransform(Transform* transform) {
    if (ImGui::TreeNode("Transform")) {
        const char* text[] {"X", "Y", "Z"};

        ImGui::Text("Posição:");
        ImGui::DragFloatEx("##Posicao", text, transform->position.data, 1.0f, 0.0f, 0.0f, "%.3f");

        ImGui::Text("Escala:");
        ImGui::DragFloatEx("##Escala", text, transform->scale.data, 1.0f, 1.0f, 0.0f, "%.3f");

        Vector3 euler = transform->rotation.eulerAngles();

        ImGui::Text("Rotação:");
        if (ImGui::DragFloatEx("##Rotacao", text, euler.data, 1.0f, -360.0f, 360.0f)) {
            transform->rotation = Math::normalize(Quaternion::Euler({Math::rad(euler.x), Math::rad(euler.y), Math::rad(euler.z)}));
        }

        ImGui::TreePop();
    }
}

void Game::OnPropertyAnimator(Animator *animator) {
    if (ImGui::TreeNode("Animator")) {
        ImGui::TreePop();
    }
}

void Game::OnPropertyMeshRender(MeshRender *meshRender) {
    if (ImGui::TreeNode("MeshRender")) {
        for (auto& entry : meshRender->materials) {
            Material& material = entry.second;
            
            if (ImGui::TreeNode(std::string(material.name + "(" + std::to_string(entry.first) + ")").c_str())) {
                {
                    ImGui::SeparatorText("Albedo");
                    if (material.texture[TexturePBR::ALBEDO] != nullptr && material.texture[TexturePBR::ALBEDO]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::ALBEDO]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::ALBEDO] = nullptr;
                        }
                    } else {
                        if (View::TextureInputUI(ImColor(material.albedo.r, material.albedo.g, material.albedo.b, material.albedo.a))) {
                            ImGui::OpenPopup("#PopupChangeColorAlbedo");
                        }
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::ALBEDO] = Resources::Load<Texture2D>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                {
                    ImGui::SeparatorText("Normal");
                    if (material.texture[TexturePBR::NORMAL] != nullptr && material.texture[TexturePBR::NORMAL]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::NORMAL]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::NORMAL] = nullptr;
                        }
                    } else {
                        View::TextureInputUI(ImColor(100, 100, 255));
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::NORMAL] = Resources::Load<Texture2D>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                {
                    ImGui::SeparatorText("Metalica");
                    if (material.texture[TexturePBR::METALLIC] != nullptr && material.texture[TexturePBR::METALLIC]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::METALLIC]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::METALLIC] = nullptr;
                        }
                    } else {
                        ImGui::DragFloat("##Metalica", &material.metallic, 0.005f, 0.0f, 1.0f);
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::METALLIC] = Resources::Load<Texture2D>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                {
                    ImGui::SeparatorText("Rugosidade");
                    if (material.texture[TexturePBR::ROUGHNESS] != nullptr && material.texture[TexturePBR::ROUGHNESS]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::ROUGHNESS]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::ROUGHNESS] = nullptr;
                        }
                    } else {
                        ImGui::DragFloat("##Rugosidade", &material.roughness, 0.005f, 0.0f, 1.0);
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::ROUGHNESS] = Resources::Load<Texture2D>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                {
                    ImGui::SeparatorText("AO");
                    if (material.texture[TexturePBR::AO] != nullptr && material.texture[TexturePBR::AO]->isvalid()) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(material.texture[TexturePBR::AO]->getID()), ImVec2(32, 32));
                        if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGuiKey_X)) {
                            material.texture[TexturePBR::AO] = nullptr;
                        }
                    } else {
                        ImGui::DragFloat("##AO", &material.ao, 0.005f, 0.0f, 1.0);
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(getDragDrop(DragDrop::TEXTURE), ImGuiDragDropFlags_None)) {
                            std::string path((char*)data->Data, data->DataSize);
                            if (!path.empty()) {
                                material.texture[TexturePBR::AO] = Resources::Load<Texture2D>(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                View::PopupChangeColor("Albedo", material.albedo);

                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

void Game::OnPropertyCameraComponent(CameraComponent *cameraComponent) {
    static int itemID = 0;

    if (ImGui::TreeNode("CameraComponent")) {
        ImGui::Text("Fov:");
        ImGui::DragFloat("##CameraComponentFov", &cameraComponent->m_fov, 1.0f, 1.0f, 179.0f);

        ImGui::Text("FarPlane:");
        ImGui::DragFloat("##CameraComponentFarPlane", &cameraComponent->m_farPlane);

        ImGui::Text("NearPlane:");
        ImGui::DragFloat("##CameraComponentNearPlane", &cameraComponent->m_nearPlane);

        ImGui::Text("Projeção:");
        ImGui::Combo("##projection", &itemID, "Perspectiva\0Ortografia\0");
        if (itemID == 0) {
            cameraComponent->m_projection = CameraComponent::ePerspective;
        } else if (itemID == 1) {
            cameraComponent->m_projection = CameraComponent::erthographic;
        }

        ImGui::TreePop();
    }
}

void Game::OnPropertyLightComponent(LightComponent *lightComponent) {
    if (ImGui::TreeNode("LightComponent")) {
        ImGui::Text("Constante:");
        ImGui::DragFloat("##constant", &lightComponent->m_attenuationConstant, 0.005f, 0.01f, 1.0f);

        ImGui::Text("Linear:");
        ImGui::DragFloat("##linear", &lightComponent->m_attenuationLinear, 0.005f, 0.01f, 1.0f);

        ImGui::Text("Quadract:");
        ImGui::DragFloat("##quadract", &lightComponent->m_attenuationQuadratic, 0.005f, 0.01f, 1.0f);

        ImGui::Text("InConner:");
        ImGui::DragFloat("##inconer", &lightComponent->m_angleInnerCone, 0.005f, 0.01f, 1.0f);

        ImGui::Text("IuterConner:");
        ImGui::DragFloat("##outerconner", &lightComponent->m_angleOuterCone, 0.005f, 0.01f, 1.0f);

        ImGui::Text("Cor:");
        ImVec4 color = {lightComponent->m_color.r, lightComponent->m_color.g, lightComponent->m_color.b, 1.0f};
        if (ImGui::ColorButton("##LightColor", color)) {
            ImGui::OpenPopup("##LightColorPopup");
        }
        if (ImGui::BeginPopup("##LightColorPopup")) {
            ImGui::ColorPicker4("#LightColorPick", lightComponent->m_color.data);
            ImGui::EndPopup();
        }
        ImGui::TreePop();
    }
}

void Game::TextureInfo(Texture2D *&texture) {
    bool IsOpen = true;

    const ImVec2 textureSize(100, 100);
    const ImVec2 window(300, 400);

    ImGui::SetWindowFocus("Texture Info");
    ImGui::SetNextWindowSize(window, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2((Screen::width / 2) - (window.x / 2), (Screen::height / 2) - (window.y / 2)), ImGuiCond_Appearing);
    ImGui::Begin("Texture Info", &IsOpen);

    const float width = ImGui::GetWindowWidth();

    ImGui::SetCursorPosX((width / 2) - (textureSize.x / 2));
    ImGui::Image(reinterpret_cast<ImTextureID>(texture->getID()), textureSize);

    ImGui::Spacing();

    ImGui::SeparatorText("Path");
    ImGui::Text("%s", texture->getpath().c_str());

    ImGui::SeparatorText("Dimensões");
    ImGui::Text("Width: %d", texture->getwidth());
    ImGui::Text("Height: %d", texture->getheight());

    ImGui::SeparatorText("Configurações");
    ImGui::Text("Espaço de cor: %s", Resources::gettextureformatname(texture->getinternalformat()));

    // ImGui::Image(reinterpret_cast<ImTextureID>(iconsUI[IconType::ICON_EDIT]->getID()), reinterpret_vector<ImVec2, Vector2>(Settings::IconSize));

    ImGui::End();

    if (!IsOpen) {
        texture = nullptr;
    }
}

void Game::MenuAdd() {
    if (ImGui::TreeNode("Luzes")) {
        if (ImGui::Button("Direct")) {
            auto& m_spot = ObjectManager::add("(Light) Direction");
            m_spot.addComponent<LightComponent>(LightComponent::eDirection);

            SceneManager::Add(&m_spot);

            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Point")) {
            auto& m_spot = ObjectManager::add("(Light) Point");
            m_spot.addComponent<LightComponent>(LightComponent::ePoint);

            SceneManager::Add(&m_spot);

            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Spot")) {
            auto& m_spot = ObjectManager::add("(Light) Spot");
            m_spot.addComponent<LightComponent>(LightComponent::eSpot);

            SceneManager::Add(&m_spot);
            
            ImGui::CloseCurrentPopup();
        }
        ImGui::TreePop();
    }
}

void Game::RenderRectSlide(const Color& color) {
    const ImVec2 rectMin = ImGui::GetCursorScreenPos();

    const ImVec2 size = ImGui::GetItemRectMax();

    ImGui::GetWindowDrawList()->AddRectFilled(rectMin, size, ImColor(color.r, color.g, color.b, color.a));
}

void Game::loadReources() {
    iconsUI.emplace(IconType::ICON_FILE,        Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-file-30.png"));
    iconsUI.emplace(IconType::ICON_FOLDER,      Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-folder-50.png"));
    iconsUI.emplace(IconType::ICON_BACK_FILE,   Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-go-back-50.png"));
    iconsUI.emplace(IconType::ICON_GAMEOBJECT,  Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-3d-object-50.png"));
    iconsUI.emplace(IconType::ICON_PENCIL,      Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-lápis-30.png"));
    iconsUI.emplace(IconType::ICON_LIGHT,       Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-lâmpada-globular-64.png"));
    iconsUI.emplace(IconType::ICON_TEXTURE,     Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-textura-de-renderização-32.png"));
    iconsUI.emplace(IconType::ICON_PNG,         Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-png-24.png"));
    iconsUI.emplace(IconType::ICON_PLAY_GAME,   Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-reproduzir-30.png"));
    iconsUI.emplace(IconType::ICON_STOP_GAME,   Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-parar-50.png"));
    iconsUI.emplace(IconType::ICON_BIN,         Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-resíduos-50.png"));
    iconsUI.emplace(IconType::ICON_IMPORT,      Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-importar-50.png"));
    iconsUI.emplace(IconType::ICON_ASCENDENTE,  Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-ordenar-para-a-direita-50.png"));
    iconsUI.emplace(IconType::ICON_DESCENDENTE, Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-ordem-descendente-50.png"));
    iconsUI.emplace(IconType::ICON_EDIT,        Resources::Load<Sprite>(Filesystem::getpublic(Filesystem::eIcon) + "/icons8-editar-24.png"));

    texturePlayGame = iconsUI[IconType::ICON_PLAY_GAME];

    SceneManager::Load(Filesystem::getpublic(Filesystem::eScene) + "/SceneMain.scn");

    mEditorController.OnInit();
}

void Game::ProgressLoad() {
    ImGuiIO& io = ImGui::GetIO();

    const ImVec2 windowSize = {300, 200};

    ImVec2 displaySize = io.DisplaySize;
    displaySize.x = (displaySize.x / 2.0f) - windowSize.x / 2.0f;
    displaySize.y = (displaySize.y / 2.0f) - windowSize.y / 2.0f;

    ImGui::SetNextWindowPos(displaySize);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("GameRender...", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove| ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::Text("Carregando UI");

    static u32 maxProgress = Resources::m_processCount;
    u32 current = Resources::m_processCount;
    if (current > maxProgress) {
        maxProgress = current;
    }

    ImGui::ProgressBar(1.0f - (float)current / maxProgress, ImVec2(windowSize.x, 16));

    ImGui::Text("%d/%d", current, maxProgress);

    if (current <= 1) {
        maxProgress = 0;
    }
    ImGui::End();
}

void Game::ImportFiles(const std::vector<std::filesystem::path> &files) {
    for (const auto& file : files) {
        if (std::filesystem::is_directory(file) || !std::filesystem::exists(file)) continue;

        if (file.filename().extension() != ".fbx") continue;

        // Filmbox filmbox(file.string());
        // filmbox.SetChangeGameObject(SetupGameObject);
        // filmbox.SetLoaded(SetupLoaded);
        // filmbox.load();
    }
}
#endif
