#include <basilisk/resource/lightServer.h>

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

    refresh();
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
    refresh();
}

/**
 * @brief Add a point light to the server
 * 
 * @param light The point light to add
 */
void LightServer::add(PointLight* light) {
    if (pointLights.size() >= MAX_POINT_LIGHTS) {
        std::cout << "Maximum number of point lights reached" << std::endl;
        return;
    }
    pointLights.push_back(light);
    refresh();
}

/**
 * @brief Add an ambient light to the server
 * 
 * @param light The ambient light to add
 */
void LightServer::add(AmbientLight* light) {
    ambientLights.push_back(light);
    refresh();
}

/**
 * @brief Refresh the light server
 * 
 */
void LightServer::refresh() {
    // Get directional lights
    for (unsigned int i = 0; i < directionalLights.size(); i++) {
        directionalLightData[i * 2] = glm::vec4(directionalLights[i]->getColor(), directionalLights[i]->getIntensity());
        directionalLightData[i * 2 + 1] = glm::vec4(directionalLights[i]->getDirection(), 0.0);
    }
    // Get point lights
    for (unsigned int i = 0; i < pointLights.size(); i++) {
        pointLightData[i * 2] = glm::vec4(pointLights[i]->getColor(), pointLights[i]->getIntensity());
        pointLightData[i * 2 + 1] = glm::vec4(pointLights[i]->getPosition(), pointLights[i]->getRange());
    }
    // Calculate total ambient light
    ambientLightData = glm::vec3(0.0, 0.0, 0.0);
    for (unsigned int i = 0; i < ambientLights.size(); i++) {
        ambientLightData += ambientLights[i]->getColor() * ambientLights[i]->getIntensity();
    }

    // Write data to UBO
    ubo->write(directionalLightData.data(), directionalLightData.size() * sizeof(glm::vec4), 0);
    ubo->write(pointLightData.data(), pointLightData.size() * sizeof(glm::vec4), MAX_DIRECTIONAL_LIGHTS * 2 * sizeof(glm::vec4));
    ubo->write(glm::value_ptr(ambientLightData), sizeof(glm::vec3), MAX_DIRECTIONAL_LIGHTS * 2 * sizeof(glm::vec4) + MAX_POINT_LIGHTS * 2 * sizeof(glm::vec4));

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
    shader->setUniform("uDirectionalLightCount", (int)directionalLights.size());
    shader->setUniform("uPointLightCount", (int)pointLights.size());
}

}