#pragma once

#include "../types.h"

struct Vertex3D {
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

    Vertex3D(const Vector3& position = Vector3::zero, const Vector3& normal = Vector3::one, const Vector2& uv = Vector2::zero, const Vector3& tangent = Vector3::zero, const Vector3 bitangent = Vector3::zero, u32 materialID = 0) : position(position), normal(normal), uv(uv), tangent(tangent), bitangent(bitangent), materialIndex(materialID) {
        for (int i=0;i<4;i++) {
            weight[i] = 0.0f;
            boneIndex[i] = -1;
        }
    }

    bool operator !=(const Vertex3D& other) const {
        return this != &other;
    }
};
