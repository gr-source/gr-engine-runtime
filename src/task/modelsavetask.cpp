#include "gr-engine/task/modelsavetask.hpp"
#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/file/fileStream.hpp"

ModelSaveTask::ModelSaveTask(const std::string& file, const ModelAsset& asset) : m_file(file), m_asset(asset)
{}

// private
int ModelSaveTask::doInBackground()
{
    FileStream stream(m_file, StreamType::OUTPUT);

    FileHeader header{{'g', 'm', 'd', 'l'}, 1.0f};
    if (!stream.write(&header, sizeof(FileHeader)))
        return 1;

    if (!m_asset.serialize(&stream))
        return 2;

    stream.close();
    return 0;
}


