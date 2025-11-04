#include <basilisk/render/material.h>

namespace bsk::internal {

/**
 * @brief Get the data of this Material as a struct
 * 
 * @return MaterialData
 */
MaterialData Material::getData() {
    MaterialData data {color, 0, 0, 0, 0, roughness, subsurface, sheen, sheenTint, anisotropic, specular, metallicness, clearcoat, clearcoatGloss};

    return data;
}

}