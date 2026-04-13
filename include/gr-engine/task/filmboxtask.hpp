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
#include <gr-base/asset/meshasset.hpp>

#include <unordered_map>

class FilmboxLoadTask : public IAsyncTask<int>
{
public:
    FilmboxLoadTask(const std::string& file);

private:
    std::string m_file;

    std::unordered_map<std::string, MaterialAsset> m_materialCacheList;

    std::unordered_map<std::string, MeshAsset> m_meshCacheList;

    void onPreExecute() override;

    int doInBackground() override;

    void onPostExecute(int result) override;
};

