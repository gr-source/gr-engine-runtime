#pragma once

#include "gr-base/core/geometry/meshAsset.hpp"
#include "gr-engine/filmbox/default.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-engine/filmbox/fbxScene.hpp"

#include <gr-base/core/thread/asynctask.hpp>

#include <gr-base/script/default.hpp>

#include <gr-base/core/default.hpp>

#include <gr-base/core/geometry/materialAsset.hpp>
#include <gr-base/core/geometry/mesh.hpp>

#include <gr-base/ecs/default.hpp>

#include <string>
#include <unordered_map>
#include <vector>

class FilmboxLoadTask : public IAsyncTask<int>
{
public:
    FilmboxLoadTask(const std::string& file);

private:
    std::vector<MeshID> m_meshes;

    std::string m_file;

    struct GameObjectDataCache
    {
        std::string name;

        Vector3 position;
        Quaternion rotation;
        Vector3 scale;


        std::vector<std::string> materialsName;
        std::vector<std::string> meshesName;

        std::vector<std::unique_ptr<GameObjectDataCache>> children;
    };

    std::unordered_map<std::string, MaterialAsset> m_materialCacheList;

    std::unordered_map<std::string, MeshAsset> m_meshCacheList;

    std::unordered_map<std::string, MeshID> m_meshMapLookUp;

    std::unordered_map<std::string, MaterialID> m_materialMapLookUp;


    std::vector<std::unique_ptr<GameObjectDataCache>> m_gameObjectCacheList;

    int doInBackground() override;

    void onPostExecute(int result) override;

    std::unique_ptr<GameObjectDataCache> foreach_node(const FbxScene* scene, const FbxNode* node);

    void CreateGameObjectFromCache(GameObjectDataCache* cache, ComponentHandle parent);

    // GameObjectHandle load_object(FbxScene *fbxscene);

    // void generateMesh(FbxScene *scene, const FbxNode *node, GameObject *go);
};

/*
#include "default.hpp"

#include "gr-base/core/thread/asynctask.hpp"

#include "..//filmbox/filmbox.hpp"

#include "gr-base/ecs/default.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/ecs/gameObject.hpp"

class FilmboxManager;

class FilmboxTask : public IAsyncTask<int>
{
public:
    FilmboxTask(const std::string &path, FilmboxManager *filmboxManager);

private:
    const std::string m_path;

    GameObject *m_objectroot;

    FilmboxManager *m_filmboxManager;

    int doInBackground() override;

    void onPostExecute(int result) override;

    void load_object(FbxScene *scene);

    GameObject *create_from(FbxScene *scene, const FbxNode &node, Transform *parent = nullptr);

    void generateMesh(FbxScene *scene, const FbxNode *node, GameObject *go);
};
*/




