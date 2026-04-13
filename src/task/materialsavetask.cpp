#include "gr-engine/task/materialsavetask.hpp"

#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/file/file.hpp"
#include "gr-base/core/file/stream.hpp"

#include <gr-base/core/file/fileStream.hpp>

MaterialSaveTask::MaterialSaveTask(const std::string& file, const MaterialAsset& asset) : m_file(file), m_asset(asset)
{}

// private
void MaterialSaveTask::onPreExecute()
{
    Console::LogInfo("[Task] Saving material '%s'...", m_file.c_str());
}

int MaterialSaveTask::doInBackground()
{
    FileStream stream(m_file, StreamType::OUTPUT);
    
    FileHeader header{{'g', 'm', 'a', 't'}, 1.0f};
    if (!stream.write(&header, sizeof(FileHeader)))
        return 1;

    if (!m_asset.serialize(&stream))
        return 2;

    stream.close();
    return 0;
}

void MaterialSaveTask::onPostExecute(int result)
{
    if (result != 0)
    {
        Console::LogError("[Task] fail in to save material '%s'!!!", m_file.c_str());

        if (gfile::exists(m_file))
            gfile::remove(m_file);
    }

    Console::LogInfo("[Task] Material saved in '%s'.", m_file.c_str());
}


