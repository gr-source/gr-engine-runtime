#pragma once

#include "../Color.h"

class FbxMaterial {
public:
    FbxMaterial(const std::string& name = "default");
private:
    std::string m_name;
    
    Color m_albedo;
    
    float m_roughness;
    
    float m_metallic;

    float m_ao;
};



