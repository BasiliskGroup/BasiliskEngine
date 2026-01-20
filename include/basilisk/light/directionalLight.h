#ifndef BSK_DIRECTIONAL_LIGHT
#define BSK_DIRECTIONAL_LIGHT

#include <basilisk/light/light.h>

namespace bsk::internal {

class DirectionalLight : public Light {
    private:
        glm::vec3 direction;

    public:
        DirectionalLight(glm::vec3 color = {1.0, 1.0, 1.0}, float intensity = 1.0f, glm::vec3 direction = {0.0, -1.0, 0.2}): 
            Light(color, intensity), direction(direction) {}

        inline glm::vec3 getDirection() { return direction; }
        inline void setDirection(glm::vec3 direction) { this->direction = direction; }
};

}

#endif