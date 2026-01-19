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

#define MAX_DIRECTIONAL_LIGHTS 5
#define MAX_POINT_LIGHTS 10

namespace bsk::internal {

class Node;  // Forward declaration to avoid circular dependency

class LightServer {

    private:
        std::vector<DirectionalLight*> directionalLights;
        std::vector<PointLight*> pointLights;
        std::vector<AmbientLight*> ambientLights;

        UBO* ubo;
        std::vector<glm::vec4> directionalLightData;
        std::vector<glm::vec4> pointLightData;
        glm::vec3 ambientLightData;

    public:
        LightServer();
        ~LightServer();

        void add(DirectionalLight* light);
        void add(PointLight* light);
        void add(AmbientLight* light);

        void update(StaticCamera* camera, Shader* shader, std::string name = "lights", unsigned int slot = 6);
        void bind(Shader* shader, std::string name = "lights", unsigned int slot = 0);
        void perObjectWrite(Node* node, Shader* shader, std::string name = "lights", unsigned int slot = 6);
};

}

#endif