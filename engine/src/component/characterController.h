#pragma once

#include "../ecs/entity.h"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>


class CharacterController : public Component {
public:
    void OnInit() override;

private:
    // btPairCachingGhostObject* m_pairCachingGhostObject;
};