#pragma once

#include <string>
#include <memory>

#include "core/filmbox/default.hpp"
#include "ecs/default.hpp"

class Stream;

class gModel3D {
public:
    gModel3D() = delete;
    ~gModel3D() = delete;

    static void ImportFBX(const std::string &path);

    static void ExportGRMX(const std::string &path, const GameObject *gameObject);
    
    static void ImportGRMX(const std::string &path);

private:
    static std::list<std::tuple<std::string, std::string, Component *>> m_componentTemp;

    static std::unordered_map<std::string, GameObject *> m_gameObjectTemp;
    
    static int ReadData(Stream *stream);

    static int ReadGameObject(Stream *stream, Transform *parent, GameObject **gameObject);

    static int ReadScript(Stream *stream);

    static int WriteData(Stream *stream, const GameObject *gameObject);

    static int WriteGameObject(Stream *stream, const GameObject *gameObject);

    static int WriteScript(Stream *stream);

    static void SetupScene(std::shared_ptr<FbxScene> scene);

    static void WriteHeader(const FbxScene *scene, const FbxNode *node);

    static void WriteNode(const FbxScene *scene, const FbxNode *node, Stream *stream);
};



