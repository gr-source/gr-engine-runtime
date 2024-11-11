#pragma once

#include "default.hpp"

#include "ecs/entity.hpp"

class Camera : public Component {
public:
    static Camera *Main();

    static Camera *Current();

    static void SetEnable(Camera *camera);

    Camera();
    ~Camera();

    void Init() override;

    void setEnable();

    const Vector3 getPosition() const;

    static std::vector<Camera*> s_camera;

    void Enable() override;

    void Disable() override;
    
private:
    Transform *m_transform;

    Matrix4x4 m_projection;

    float m_nearPlane;
    
    float m_farPlane;
    
    float m_aspect;

    float m_fov;

    static Camera *s_current;

    void updateProjection();
};

REGISTER_COMPONENT(Camera);






