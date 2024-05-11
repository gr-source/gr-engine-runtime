#pragma once

#include "../ecs/Component.h"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>


class CharacterController : public Component {
public:
    void OnInit() override;

private:
    // btPairCachingGhostObject* m_pairCachingGhostObject;
};