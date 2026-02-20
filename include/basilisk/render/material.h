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

        void update();

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
        );

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

        void setColor(const glm::vec3& value) { color = value; update(); }

        void setAlbedo(Image* value) { albedo = value; update(); }
        void setNormal(Image* value) { normal = value; update(); }

        void setRoughness(float value) { roughness = value; update(); }
        void setSubsurface(float value) { subsurface = value; update(); }
        void setSheen(float value) { sheen = value; update(); }
        void setSheenTint(float value) { sheenTint = value; update(); }
        void setAnisotropic(float value) { anisotropic = value; update(); }
        void setSpecular(float value) { specular = value; update(); }
        void setMetallicness(float value) { metallicness = value; update(); }
        void setClearcoat(float value) { clearcoat = value; update(); }
        void setClearcoatGloss(float value) { clearcoatGloss = value; update(); }

        MaterialData getData();
};

}

#endif