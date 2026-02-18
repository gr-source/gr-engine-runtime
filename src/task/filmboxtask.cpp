#include "gr-engine/task/filmboxtask.hpp"

#include "gr-engine/filmbox/filmbox.hpp"

#include "gr-base/core/default.hpp"

#include "gr-engine/managers/viewmanager.hpp"

#include "gr-engine/view/popup/filmboximport.hpp"

#include <gr-base/core/file/file.hpp>

#include <gr-base/core/gameObject.hpp>

#include <gr-base/core/console.hpp>

#include <gr-base/script/skinnedMeshRender.hpp>
#include <gr-base/script/meshRender.hpp>
#include <gr-base/script/meshFilter.hpp>
#include <gr-base/script/transform.hpp>
// #include <gr-base/script/animator.hpp>

#include <gr-base/managers/assetsManager.hpp>

FilmboxLoadTask::FilmboxLoadTask(const std::string& file) : m_file(file)
{}

GameObjectHandle FilmboxLoadTask::doInBackground()
{
    Console::LogInfo("[ FilmboxTask ] Loading: %s", m_file.c_str());

    Filmbox filmbox;
    try
    {
        filmbox.load(m_file);
    }
    catch (const std::exception &e)
    {
        Console::LogError("[ FilmboxTask ] %s.", e.what());
        return GID_INVALID;
    }

    auto scene = filmbox.get_scene();

    return load_object(scene.get());
}

void FilmboxLoadTask::onPostExecute(GameObjectHandle result)
{
    if (result == GID_INVALID)
        return;

    ViewManager::AddPopup<FilmboxImport>(result, m_meshes);
}

GameObjectHandle FilmboxLoadTask::load_object(FbxScene *fbxscene)
{
    auto &root = fbxscene->getRootNode();

    GameObjectHandle first = GameObject::Create(gfile::getname(m_file.c_str()));

    for (auto &node : root.getChildren())
    {
        auto gameObject = GameObject::Get(first);

        auto gameObjectID = create_from(fbxscene, node, gameObject->GetTransform());
        if (gameObjectID == GID_INVALID)
            break;
    }
    return first;
}

GameObjectHandle FilmboxLoadTask::create_from(FbxScene *scene, const FbxNode &node, Transform *parent)
{
    auto gameObjectID = GameObject::Create(node.getName());
    
    auto gameObject = GameObject::Get(gameObjectID);
    auto transform = gameObject->GetTransform();
    if (parent != nullptr)
        transform->setParent(parent);

    transform->setLocalPosition(node.getPosition());
    transform->setLocalRotation(node.getRotation());
    transform->setLocalScale(node.getScale());

    if (node.hasMeshe())
        generateMesh(scene, &node, gameObject);

    for (auto &children : node.getChildren())
    {
        auto child = create_from(scene, children, transform);
        if (child == GID_INVALID)
            break;
    }
    return gameObjectID;
}

void FilmboxLoadTask::generateMesh(FbxScene *scene, const FbxNode *node, GameObject *go)
{
    std::unordered_map<std::string, FbxBone> m_bones;
    std::vector<FbxBone> bonesList;

    MeshFilter *meshFilter = go->getComponent<MeshFilter>() ? go->getComponent<MeshFilter>() : go->addComponent<MeshFilter>();
    // MeshRender *meshRender = go->hasComponent<MeshRender>() ? go->getComponent<MeshRender>() : go->addComponent<MeshRender>();
    
    auto meshes = node->getMeshes();
    for (std::size_t i=0;i<meshes.size();i++)
    {
        auto &fbxMesh = meshes[i];

        MeshID meshID = AssetsManager::CreateMesh(fbxMesh->getName() + "_" + node->getName() + "_" + std::to_string(i));
        m_meshes.push_back(meshID);

        meshFilter->addMesh(meshID);

        Mesh* mesh = AssetsManager::GetMesh(meshID);

        for (auto &vertice : fbxMesh->getVertices())
        {
            mesh->addVertex(vertice.position, vertice.normal, vertice.texCoord);

            if (!fbxMesh->hasBone()) {
                continue;
            }

            auto& boneWeight = mesh->addBoneWeight();

            float norm = vertice.weight[0] + vertice.weight[1] + vertice.weight[2] + vertice.weight[3];

            for (auto k=0;k<4;++k) {
                boneWeight.insert<int>(k, vertice.boneIndex[k]);
                boneWeight.insert<float>(k, norm > 0.0f ? vertice.weight[k] / norm : vertice.weight[k]);
            }
        }

        for (auto &indice : fbxMesh->getIndices())
        {
            mesh->addIndice(indice);
        }

        if (fbxMesh->hasBone())
        {
            for (auto &bone : fbxMesh->getBones())
            {
                if (m_bones.find(bone.getName()) != m_bones.end())
                    continue;

                m_bones.emplace(bone.getName(), bone);
            }
        }

        mesh->CalculateLocalBounds();
    }

    bonesList.resize(m_bones.size(), {"", 0});
    for (auto &&[name, bone] : m_bones)
    {
        bonesList[bone.getID()] = bone;
    }

    /*
    auto m_mesh = AssetsManager::FindMesh(mesh_name + '_' + node->getName());
    if (!m_mesh)
    {
        m_mesh = AssetsManager::CreateMesh(mesh_name + '_' + node->getName());
        for (auto &mesh : meshMap)
        {
            m_mesh->emplace_back(mesh);
        }
    }

    auto meshFilter = go->addComponent<MeshFilter>();
    if (!meshFilter) { return; }

    meshFilter->setMesh(m_mesh);
    */


    grr::gShader* shader = nullptr;
    grr::gShader* shaderInstanced = nullptr;
    {
        ShaderParams shaderParams;
        shaderParams.name = "simple_model";
        shaderParams.base = gfile::get_data_directory() + "/assets/shader/model/simple_model.glsl";
        shaderParams.flags = ShaderOptimize_dynamic;
        shaderParams.type = ShaderType::SHADER3D;

        auto name = GetShaderParamsName(shaderParams);

        shader = AssetsManager::FindShader(name);
        if (shader == nullptr)
        {
            auto info = AssetsManager::LoadShaderAsync(shaderParams);
            shader = info->get();
        }
    }

    {
        ShaderParams shaderParams;
        shaderParams.name = "simple_model";
        shaderParams.base = gfile::get_data_directory() + "/assets/shader/model/simple_model.glsl";
        shaderParams.flags = ShaderOptimize_dynamic | ShaderOptimize_instanced;
        shaderParams.type = ShaderType::SHADER3D;

        auto name = GetShaderParamsName(shaderParams);

        shaderInstanced = AssetsManager::FindShader(name);
        if (shaderInstanced == nullptr)
        {
            auto info = AssetsManager::LoadShaderAsync(shaderParams);
            shaderInstanced = info->get();
        }
    }

    auto renderer = m_bones.empty() ? static_cast<BaseRender *>(go->addComponent<MeshRender>()) : static_cast<BaseRender *>(go->addComponent<SkinnedMeshRender>());
    if (!renderer) { return; }

    for (auto &fbxMesh : node->getMeshes())
    {
        auto &fbxMaterial = scene->getMaterials()[fbxMesh->getMaterialIndex()];

        auto materialID = AssetsManager::CreateMaterial(fbxMaterial.getName());
        renderer->addMaterial(materialID);

        auto texturepath = fbxMaterial.getTextures();

        auto material = AssetsManager::GetMaterial(materialID);
        material->SetShader(shader, false);
        material->SetShader(shaderInstanced, true);

        material->setValue("albedo", fbxMaterial.getColor());
        material->setValue("metallic", fbxMaterial.metallic);
        material->setValue("roughness", fbxMaterial.roughness);
        material->setValue("ambientocclusion", fbxMaterial.ao);

        if (!texturepath[TextureType_Albedo].empty())
        {
            auto path = texturepath[TextureType_Albedo];
            path = path.find('/') != std::string::npos ? path : gfile::get_parent(m_file) + '/' + path;

            AssetsManager::LoadTexture(path, &Material::setTexture, material, 0);
        }
    }

    /*
    if (!m_bones.empty())
    {
        auto animator = go->addComponent<Animator>();
        if (!animator) { return; }

        SkinnedMeshRender *skinnedMeshRender = static_cast<SkinnedMeshRender *>(renderer);

        skinnedMeshRender->setBones(std::vector<Bone>(m_bones.size(), Bone{}));

        for (std::size_t i=0;i<bonesList.size();i++) {
            skinnedMeshRender->getBone(i)->setScale(bonesList[i].getScale());
            skinnedMeshRender->getBone(i)->setRotation(bonesList[i].getRotation());
            skinnedMeshRender->getBone(i)->setPosition(bonesList[i].getPosition());

            skinnedMeshRender->getBone(i)->setParentIndex(bonesList[i].getParentIndex());
        }
    }
    */
}




