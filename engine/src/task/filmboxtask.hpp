#pragma once

#include "filmbox/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <gr-base/core/thread/asynctask.hpp>

#include <gr-base/script/default.hpp>

#include <gr-base/core/default.hpp>

#include <string>

class FilmboxLoadTask : public IAsyncTask<GameObjectID>
{
public:
    FilmboxLoadTask(const std::string& file);

private:
    std::vector<MeshID> m_meshes;

    std::string m_file;

    GameObjectID doInBackground() override;

    void onPostExecute(GameObjectID result) override;

    GameObjectID load_object(FbxScene *fbxscene);

    GameObjectID create_from(FbxScene *scene, const FbxNode &node, Transform *parent);

    void generateMesh(FbxScene *scene, const FbxNode *node, GameObject *go);
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




