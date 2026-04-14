#include "gr-engine/task/meshsavetask.hpp"
#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/file/fileStream.hpp"

MeshSaveTask::MeshSaveTask(const std::string& file, const MeshAsset& asset) : m_file(file), m_asset(asset)
{}

// private
int MeshSaveTask::doInBackground()
{
    FileStream stream(m_file, StreamType::OUTPUT);

    FileHeader header{{'g', 'm', 's', 'h'}, 1.0f};
    if (!stream.write(&header, sizeof(FileHeader)))
        return 1;

    if (!m_asset.serialize(&stream))
        return 2;

    stream.close();

    return 0;
}

