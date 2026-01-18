#ifndef BSK_POINT_LIGHT
#define BSK_POINT_LIGHT

#include <basilisk/light/light.h>

namespace bsk::internal {

class PointLight : public Light {
    private:
        glm::vec3 position;
        float range;

    public:
        PointLight(glm::vec3 color = {1.0, 1.0, 1.0}, float intensity = 1.0f, glm::vec3 position = {0.0, 0.0, 0.0}, float range = 15.0f): 
            Light(color, intensity), position(position), range(range) {}

        inline glm::vec3 getPosition() { return position; }
        inline float getRange() { return range; }

        inline void setPosition(glm::vec3 position) { this->position = position; }
        inline void setRange(float range) { this->range = range; }
};

}

#endif