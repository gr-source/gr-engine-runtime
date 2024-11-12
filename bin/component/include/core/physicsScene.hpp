#pragma once

#include <bullet/btBulletDynamicsCommon.h>

class PhysicsScene {
public:
    static void (*contactTest)(btCollisionObject *colObj, btCollisionWorld::ContactResultCallback &resultCallback);
};



