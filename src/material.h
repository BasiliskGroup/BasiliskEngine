#ifndef MATERIAL_H
#define MATERIAL_H

#include "texture.h"

class Material {
    private:
        Texture texture;
        vec4 color;
    public:
        Material();
};

#endif