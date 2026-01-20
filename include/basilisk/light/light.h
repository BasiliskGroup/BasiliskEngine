#ifndef BSK_LIGHT_H
#define BSK_LIGHT_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Light {
    protected:
        glm::vec3 color;
        float intensity;
    
    public:
        Light(glm::vec3 color = {1.0, 1.0, 1.0}, float intensity = 1.0f) : color(color), intensity(intensity) {}
        virtual ~Light() = default;
        
        inline glm::vec3 getColor() { return color; }
        inline float getIntensity() { return intensity; }

        inline void setColor(glm::vec3 color) { this->color = color; }
        inline void setIntensity(float intensity) { this->intensity = intensity; }
};

}

#endif