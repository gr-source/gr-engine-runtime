#pragma once

#include "default.hpp"

#include "ecs/entity.hpp"

class Camera : public Component {
public:
    static std::vector<Camera*> s_camera;
    
    static Camera *Main();

    static Camera *Current();

    Camera();

    void Init() override;

    void Destroy() override;

    void setEnable();

    Vector3 getPosition() const;

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






