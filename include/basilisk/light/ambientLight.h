#ifndef BSK_AMBIENT_LIGHT
#define BSK_AMBIENT_LIGHT

#include <basilisk/light/light.h>

namespace bsk::internal {

class AmbientLight : public Light {
    public:
        AmbientLight(glm::vec3 color = {1.0, 1.0, 1.0}, float intensity = 1.0f) : Light(color, intensity) {}
};

}

#endif