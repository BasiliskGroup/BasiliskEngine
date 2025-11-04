#ifndef BSK_MATERIAL_H
#define BSK_MATERIAL_H

#include <basilisk/util/includes.h>
#include <basilisk/render/image.h>

namespace bsk::internal {

struct MaterialData {
    glm::vec3 color;

    uint32_t albedoArray;
    uint32_t albedoIndex;
    uint32_t normalArray;
    uint32_t normalIndex;

    float roughness;
    float subsurface;
    float sheen;
    float sheenTint;
    float anisotropic;
    float specular;
    float metallicness;
    float clearcoat;
    float clearcoatGloss;
};

class Material {
    private:
        glm::vec3 color;

        Image* albedo;
        Image* normal;

        float roughness;
        float subsurface;
        float sheen;
        float sheenTint;
        float anisotropic;
        float specular;
        float metallicness;
        float clearcoat;
        float clearcoatGloss;

    public:
        Material(
            const glm::vec3& color = {1.0f, 1.0f, 1.0f},

            Image* albedo = nullptr,
            Image* normal = nullptr,

            float subsurface = 0.0f,
            float sheen = 0.0f,
            float sheenTint = 0.0f,
            float anisotropic = 0.0f,
            float specular = 0.75f,
            float metallicness = 0.0f,
            float clearcoat = 0.0f,
            float clearcoatGloss = 0.0f
        )
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
        {}

        inline const glm::vec3& getColor() const { return color; }
        
        inline Image* getAlbedo() const { return albedo; }
        inline Image* getNormal() const { return normal; }

        inline float getRoughness() const { return roughness; }
        inline float getSubsurface() const { return subsurface; }
        inline float getSheen() const { return sheen; }
        inline float getSheenTint() const { return sheenTint; }
        inline float getAnisotropic() const { return anisotropic; }
        inline float getSpecular() const { return specular; }
        inline float getMetallicness() const { return metallicness; }
        inline float getClearcoat() const { return clearcoat; }
        inline float getClearcoatGloss() const { return clearcoatGloss; }

        void setColor(const glm::vec3& value) { color = value; }

        void setAlbedo(Image* value) { albedo = value; }
        void setNormal(Image* value) { normal = value; }

        void setRoughness(float value) { roughness = value; }
        void setSubsurface(float value) { subsurface = value; }
        void setSheen(float value) { sheen = value; }
        void setSheenTint(float value) { sheenTint = value; }
        void setAnisotropic(float value) { anisotropic = value; }
        void setSpecular(float value) { specular = value; }
        void setMetallicness(float value) { metallicness = value; }
        void setClearcoat(float value) { clearcoat = value; }
        void setClearcoatGloss(float value) { clearcoatGloss = value; }

        MaterialData getData();
};

}

#endif