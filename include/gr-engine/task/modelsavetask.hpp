#pragma once

#include "gr-base/asset/modelasset.hpp"
#include "gr-base/core/thread/asynctask.hpp"

class ModelSaveTask : public IAsyncTask<int>
{
public:
    ModelSaveTask(const std::string& file, const ModelAsset& asset);

private:
    std::string m_file;

    ModelAsset m_asset;

    int doInBackground() override;
};


