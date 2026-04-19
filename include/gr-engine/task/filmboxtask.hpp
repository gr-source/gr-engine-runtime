#pragma once

#include "gr-engine/filmbox/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <gr-base/core/thread/asynctask.hpp>

#include <gr-base/script/default.hpp>

#include <gr-base/core/default.hpp>

#include <gr-base/core/geometry/material.hpp>
#include <gr-base/core/geometry/mesh.hpp>

#include <gr-base/ecs/default.hpp>

#include <gr-base/asset/materialasset.hpp>
#include <gr-base/asset/modelasset.hpp>
#include <gr-base/asset/meshasset.hpp>

#include <unordered_map>

class FilmboxLoadTask : public IAsyncTask<int>
{
public:
    FilmboxLoadTask(const std::string& file);

private:
    std::string m_file;

    std::string m_shader;

    std::unordered_map<std::string, MaterialAsset> m_materialCacheList;

    std::unordered_map<std::string, MeshAsset> m_meshCacheList;

    struct GameObjectCache
    {
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;

        std::string meshName;

        std::string name;

        std::vector<GameObjectCache> childrent;
    };

    std::vector<GameObjectCache> m_objectCacheList;

    const FbxScene *m_scene;

    void onPreExecute() override;

    int doInBackground() override;

    void onPostExecute(int result) override;

    GameObjectCache foreach_node(const FbxNode* node);

    GameObjectHandle CreateGameObjectFromCache(GameObjectCache* cache, GameObjectHandle parent = HANDLE_INVALID);

    ModelAsset ConvertToAssetModel(GameObjectHandle handle);
};

