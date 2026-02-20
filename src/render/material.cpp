#include <basilisk/render/material.h>
#include <basilisk/engine/engine.h>
#include <basilisk/resource/materialServer.h>
#include <basilisk/resource/resourceServer.h>

namespace bsk::internal {

Material::Material(const glm::vec3& color, Image* albedo, Image* normal,
    float subsurface, float sheen, float sheenTint, float anisotropic,
    float specular, float metallicness, float clearcoat, float clearcoatGloss)
    : color(color),
      albedo(albedo),
      normal(normal),
      roughness(0.0f),
      subsurface(subsurface),
      sheen(sheen),
      sheenTint(sheenTint),
      anisotropic(anisotropic),
      specular(specular),
      metallicness(metallicness),
      clearcoat(clearcoat),
      clearcoatGloss(clearcoatGloss)
{
    if (!this->albedo) this->albedo = Engine::getResourceServer()->defaultImage;
    if (!this->normal) this->normal = Engine::getResourceServer()->defaultImage;
}

void Material::update() {
    Engine::getResourceServer()->getMaterialServer()->update(this);
}

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