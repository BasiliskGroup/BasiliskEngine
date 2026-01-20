#ifndef BSK_LIGHT_SERVER_H
#define BSK_LIGHT_SERVER_H

#include <basilisk/util/includes.h>
#include <basilisk/camera/staticCamera.h>
#include <basilisk/light/light.h>
#include <basilisk/light/ambientLight.h>
#include <basilisk/light/directionalLight.h>
#include <basilisk/light/pointLight.h>
#include <basilisk/render/shader.h>
#include <basilisk/render/ubo.h>
#include <basilisk/render/tbo.h>

#define MAX_DIRECTIONAL_LIGHTS 5
#define MAX_POINT_LIGHTS 1000
#define TILE_SIZE 32
#define MAX_LIGHTS_PER_TILE 64

struct Plane {
    glm::vec3 normal;
};

struct Tile {
    Plane planes[4];
};

struct TileInfo {
    uint32_t offset;
    uint32_t count;
    uint32_t pad0;
    uint32_t pad1;
};

namespace bsk::internal {

class LightServer {

    private:
        UBO* directionalLightsUBO;
        TBO* tileTBO;
        TBO* lightIndicesTBO;
        TBO* pointLightsTBO;

        std::vector<DirectionalLight*> directionalLights;
        std::vector<PointLight*> pointLights;
        std::vector<AmbientLight*> ambientLights;

        std::vector<glm::vec4> directionalLightData;
        std::vector<glm::vec4> pointLightData;
        glm::vec3 ambientLightData;

        unsigned int tilesX;
        unsigned int tilesY;
        std::vector<Tile> tiles;
        std::vector<TileInfo> tileInfos;
        std::vector<uint32_t> lightIndices;

        // Update groups
        void updateDirectional(Shader* shader);
        void updatePoint(Shader* shader, StaticCamera* camera);
        void updateAmbient(Shader* shader);
        void updateTiles(StaticCamera* camera);

        // Helpers
        glm::vec3 unproject(const glm::mat4& inverseProjection, float x, float y);
        bool lightIntersectsTile(glm::vec3& lightPositionViewSpace, float lightRadius, Tile& tile);

    public:
        LightServer();
        ~LightServer();

        void add(DirectionalLight* light);
        void add(PointLight* light);
        void add(AmbientLight* light);

        void update(Shader* shader, StaticCamera* camera);
        void setTiles(Shader* shader, StaticCamera* camera, unsigned int screenWidth, unsigned int screenHeight);
};

}

#endif