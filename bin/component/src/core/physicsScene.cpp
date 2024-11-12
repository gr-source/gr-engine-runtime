#include "core/physicsScene.hpp"
#include "core/function.hpp"

void (*PhysicsScene::contactTest)(btCollisionObject *colObj, btCollisionWorld::ContactResultCallback &resultCallback);
INSERT_FUNCTION(0, PhysicsScene::contactTest);



