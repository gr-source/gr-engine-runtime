#pragma once

#include "../ecs/Entity.h"

class CameraComponent : public Component {
public:
    static std::vector<CameraComponent*> m_cameras;

    static CameraComponent& MainCamera();

    static u32 CameraID;

    CameraComponent(float fov = 60.0f, float nearPlane = 1.0f, float farPlane = 1.0f, float aspect = 1.0f);

    void OnInit() override;

    const Vector3 getposition();

    const Matrix4x4 getviewmatrix() const;

    const Matrix4x4 getperspesctive() const;

    Transform* gettransform();

    void SetFildOfView(float fov);

    ~CameraComponent();

private:
    float m_fov;

    float m_nearPlane;

    float m_farPlane;

    float m_aspect;

    Transform* m_transform;

    friend class EditorController;

};


