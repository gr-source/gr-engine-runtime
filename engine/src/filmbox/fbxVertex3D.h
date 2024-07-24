#pragma once

#include "../common.h"

class FbxVertex3D {
public:
    /* posiçao de cada pronto do objeto */
    Vector3 position;

    /* normal da vertice */
    Vector3 normal;

    /* uv para imagem */
    Vector2 uv;

    /* tangent */
    Vector3 tangent;

    /* bitangent */
    Vector3 bitangent;
    
    /* peso do osso */
    float weight[4];

    /* id do osso */
    int boneIndex[4];

    /* id do material */
    u32 materialIndex;
};

