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
#define MAX_POINT_LIGHTS 50
#define TILE_SIZE 32
#define MAX_LIGHTS_PER_TILE 32

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
        std::vector<DirectionalLight*> directionalLights;
        std::vector<PointLight*> pointLights;
        std::vector<AmbientLight*> ambientLights;

        UBO* ubo;
        std::vector<glm::vec4> directionalLightData;
        std::vector<glm::vec4> pointLightData;
        glm::vec3 ambientLightData;

        TBO* tileTBO;
        TBO* lightIndicesTBO;
        unsigned int tilesX;
        unsigned int tilesY;
        std::vector<Tile> tiles;
        std::vector<TileInfo> tileInfos;
        std::vector<uint32_t> lightIndices;

    public:
        LightServer();
        ~LightServer();

        void add(DirectionalLight* light);
        void add(PointLight* light);
        void add(AmbientLight* light);

        void update(StaticCamera* camera, Shader* shader, std::string name = "lights", unsigned int slot = 6);
        void bind(Shader* shader, std::string name = "lights", unsigned int slot = 0);
        
        void setTiles(Shader* shader, StaticCamera* camera, unsigned int screenWidth, unsigned int screenHeight);
        void updateTiles(StaticCamera* camera);
};

}

#endif