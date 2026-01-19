#include <basilisk/resource/lightServer.h>
#include <basilisk/nodes/node.h>

namespace bsk::internal {

/**
 * @brief Construct a new Light Server object
 * 
 */
LightServer::LightServer() {
    unsigned int bufferSize = MAX_DIRECTIONAL_LIGHTS * 2 * sizeof(glm::vec4) + MAX_POINT_LIGHTS * 2 * sizeof(glm::vec4) + sizeof(glm::vec4);
    ubo = new UBO(nullptr, bufferSize, GL_DYNAMIC_DRAW);

    directionalLightData = std::vector<glm::vec4>(MAX_DIRECTIONAL_LIGHTS * 2);
    pointLightData = std::vector<glm::vec4>(MAX_POINT_LIGHTS * 2);
    ambientLightData = glm::vec3(0.0, 0.0, 0.0);
}

/**
 * @brief Destroy the Light Server object
 * 
 */
LightServer::~LightServer() {
    delete ubo;
}

/**
 * @brief Add a directional light to the server
 * 
 * @param light The directional light to add
 */
void LightServer::add(DirectionalLight* light) {
    if (directionalLights.size() >= MAX_DIRECTIONAL_LIGHTS) {
        std::cout << "Maximum number of directional lights reached" << std::endl;
        return;
    }
    directionalLights.push_back(light);
}

/**
 * @brief Add a point light to the server
 * 
 * @param light The point light to add
 */
void LightServer::add(PointLight* light) {
    pointLights.push_back(light);
}

/**
 * @brief Add an ambient light to the server
 * 
 * @param light The ambient light to add
 */
void LightServer::add(AmbientLight* light) {
    ambientLights.push_back(light);
}

/**
 * @brief Update the light server
 * 
 * @param camera The camera to center the point lights around
 * @param shader The shader to write the light data to
 */
void LightServer::update(StaticCamera* camera, Shader* shader, std::string name, unsigned int slot) {
    // Get directional lights data
    size_t directionalCount = directionalLights.size();
    for (size_t i = 0; i < directionalCount; i++) {
        directionalLightData[i * 2] = glm::vec4(directionalLights[i]->getColor(), directionalLights[i]->getIntensity());
        directionalLightData[i * 2 + 1] = glm::vec4(directionalLights[i]->getDirection(), 0.0);
    }

    // Calculate total ambient light data
    ambientLightData = glm::vec3(0.0, 0.0, 0.0);
    for (AmbientLight* light : ambientLights) {
        ambientLightData += light->getColor() * light->getIntensity();
    }

    // Write data to UBO (only the portion actually used)
    size_t directionalDataSize = directionalCount * 2 * sizeof(glm::vec4);
    ubo->write(directionalLightData.data(), directionalDataSize, 0);
    ubo->write(pointLightData.data(), pointLightData.size() * sizeof(glm::vec4), MAX_DIRECTIONAL_LIGHTS * 2 * sizeof(glm::vec4));
    ubo->write(glm::value_ptr(ambientLightData), sizeof(glm::vec3), MAX_DIRECTIONAL_LIGHTS * 2 * sizeof(glm::vec4) + MAX_POINT_LIGHTS * 2 * sizeof(glm::vec4));
    
    // Bind the light data to the shader
    bind(shader, name, slot);
}

/**
 * @brief Bind the light server to a shader
 * 
 * @param shader The shader to bind the light server to
 * @param name The name of the uniform block on the shader
 * @param slot The slot to bind the light server to
 */
void LightServer::bind(Shader* shader, std::string name, unsigned int slot) {
    shader->bind(name.c_str(), ubo, slot);
    shader->setUniform("uDirectionalLightCount", std::min((int)directionalLights.size(), MAX_DIRECTIONAL_LIGHTS));
    shader->setUniform("uPointLightCount", std::min((int)pointLights.size(), MAX_POINT_LIGHTS));
}

void LightServer::perObjectWrite(Node* node, Shader* shader, std::string name, unsigned int slot) {
    // Cache node position to avoid repeated calls
    const glm::vec3 nodePos = node->getPosition();

    // Get point lights in range of the node
    std::vector<PointLight*> pointLightsInRange;
    pointLightsInRange.reserve(std::min(pointLights.size(), static_cast<size_t>(MAX_POINT_LIGHTS)));
    
    for (PointLight* light : pointLights) {
        float range = light->getRange();
        float distanceSquared = glm::length2(light->getPosition() - nodePos);
        if (distanceSquared <= range * range) {
            pointLightsInRange.push_back(light);
        }
    }
    
    // Sort point lights by squared distance from node (avoid expensive sqrt)
    std::sort(pointLightsInRange.begin(), pointLightsInRange.end(), [nodePos](PointLight* a, PointLight* b) {
        return glm::length2(a->getPosition() - nodePos) < glm::length2(b->getPosition() - nodePos);
    });
    
    // Get point lights data (only up to MAX_POINT_LIGHTS)
    size_t lightCount = std::min(pointLightsInRange.size(), static_cast<size_t>(MAX_POINT_LIGHTS));
    for (size_t i = 0; i < lightCount; i++) {
        pointLightData[i * 2] = glm::vec4(pointLightsInRange[i]->getColor(), pointLightsInRange[i]->getIntensity());
        pointLightData[i * 2 + 1] = glm::vec4(pointLightsInRange[i]->getPosition(), pointLightsInRange[i]->getRange());
    }
    
    // Write point lights data to UBO (only the portion actually used)
    ubo->write(pointLightData.data(), lightCount * 2 * sizeof(glm::vec4), MAX_DIRECTIONAL_LIGHTS * 2 * sizeof(glm::vec4));

    // Bind the light data to the shader
    shader->setUniform("uPointLightCount", static_cast<int>(lightCount));
}

}