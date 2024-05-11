#include "animation.h"

Animation::Animation(const std::string& name) : name(name) {}

/*Bone* Animation::findBone(const std::string& name) {
    auto it = std::find_if(pBones.begin(), pBones.end(),
        [&](const Bone& bone) {
            return bone.name == name;
        }
    );
    if (it == pBones.end()) {
        return nullptr;
    }
    else
        return &(*it);
}
*/


