#pragma once

#include "gr-base/script/transform.hpp"
#include <gr-base/core/default.hpp>

class CameraEditorSystem
{
public:
    inline static void Update(Scene* scene)
    {
        GetInstance().update(scene);
    }

private:
    static CameraEditorSystem& GetInstance();

    float mouseSensitivity = 0.1f;

    Vector2 elapsed = Vector2::zero;

    void update(Scene* scene);

    void UpdateCursor(Transform* transform);

    void ProcessMouse(const Vector2 &delta, Transform *transform);
};



