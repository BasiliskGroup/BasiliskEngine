#include <basilisk/resource/lightServer.h>

namespace bsk::internal {

/**
 * @brief Construct a new Light Server object
 * 
 */
LightServer::LightServer() {
    unsigned int bufferSize = MAX_DIRECTIONAL_LIGHTS * 2 * sizeof(glm::vec4) + MAX_POINT_LIGHTS * 2 * sizeof(glm::vec4) + sizeof(glm::vec4);
    ubo = new UBO(nullptr, bufferSize, GL_DYNAMIC_DRAW);
    tileTBO = nullptr;
    lightIndicesTBO = nullptr;
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

    // Sort point lights by distance from camera
    std::sort(pointLights.begin(), pointLights.end(), [camera](PointLight* a, PointLight* b) {
        return glm::length(camera->getPosition() - a->getPosition()) < glm::length(camera->getPosition() - b->getPosition());
    });
    // Get point lights data
    size_t lightCount = std::min(pointLights.size(), static_cast<size_t>(MAX_POINT_LIGHTS));
    for (size_t i = 0; i < lightCount; i++) {
        pointLightData[i * 2] = glm::vec4(pointLights[i]->getColor(), pointLights[i]->getIntensity());
        pointLightData[i * 2 + 1] = glm::vec4(pointLights[i]->getPosition(), pointLights[i]->getRange());
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
    // shader->setUniform("uPointLightCount", std::min((int)pointLights.size(), MAX_POINT_LIGHTS));
}

glm::vec3 unproject(const glm::mat4& inverseProjection, float x, float y) {
    glm::vec4 clip(x, y, -1.0f, 1.0f);
    glm::vec4 view = inverseProjection * clip;
    return glm::normalize(glm::vec3(view) / view.w);
}

bool lightIntersectsTile(glm::vec3& lightPositionViewSpace, float lightRadius, Tile& tile) {
    if (lightPositionViewSpace.z - lightRadius > 0.0f)
        return false;
    for (int i = 0; i < 4; i++) {
        float distance = glm::dot(tile.planes[i].normal, lightPositionViewSpace);
        if (distance < -lightRadius) { return false; }
    }
    return true;
}

void LightServer::setTiles(Shader* shader, StaticCamera* camera, unsigned int screenWidth, unsigned int screenHeight) {
    tilesX = (unsigned int)ceil((float)screenWidth  / (float)TILE_SIZE);
    tilesY = (unsigned int)ceil((float)screenHeight / (float)TILE_SIZE);
    unsigned int tileCount = tilesX * tilesY;

    tiles.resize(tileCount);
    tileInfos.resize(tileCount);
    lightIndices.resize(tileCount * MAX_LIGHTS_PER_TILE);

    if (tileTBO) { delete tileTBO; }
    if (lightIndicesTBO) { delete lightIndicesTBO; }

    tileTBO = new TBO(tileInfos);
    lightIndicesTBO = new TBO(lightIndices);

    shader->bind("lightTiles", tileTBO, 7);
    shader->bind("lightIndices", lightIndicesTBO, 14);

    glm::mat4 projection = camera->getProjection();
    glm::mat4 inverseProjection = glm::inverse(projection);

    for (unsigned int ty = 0; ty < tilesY; ++ty) {
        for (unsigned int tx = 0; tx < tilesX; ++tx) {
            unsigned int tileIndex = ty * tilesX + tx;
            Tile& tile = tiles.at(tileIndex);

            // Calculate the corners of the tile ([0, 1] spae)
            float x0 = float(tx * TILE_SIZE) / screenWidth;
            float x1 = float((tx + 1) * TILE_SIZE) / screenWidth;
            float y0 = float(ty * TILE_SIZE) / screenHeight;
            float y1 = float((ty + 1) * TILE_SIZE) / screenHeight;

            // Convert to NDC space ([-1, 1] space)
            float ndcX0 = x0 * 2.0f - 1.0f;
            float ndcX1 = x1 * 2.0f - 1.0f;
            float ndcY0 = y0 * 2.0f - 1.0f;
            float ndcY1 = y1 * 2.0f - 1.0f;

            // Unproject to view space
            glm::vec3 rayBL = unproject(inverseProjection, ndcX0, ndcY0);
            glm::vec3 rayBR = unproject(inverseProjection, ndcX1, ndcY0);
            glm::vec3 rayTR = unproject(inverseProjection, ndcX1, ndcY1);
            glm::vec3 rayTL = unproject(inverseProjection, ndcX0, ndcY1);

            // Calculate the planes of the tile (normals pointing inward)
            tile.planes[0].normal = glm::normalize(glm::cross(rayTL, rayBL));
            tile.planes[1].normal = glm::normalize(glm::cross(rayBR, rayTR));
            tile.planes[2].normal = glm::normalize(glm::cross(rayBL, rayBR));
            tile.planes[3].normal = glm::normalize(glm::cross(rayTR, rayTL));
        }
    }    
}

void LightServer::updateTiles(StaticCamera* camera) {
    lightIndices.clear();
    lightIndices.reserve(tiles.size() * MAX_LIGHTS_PER_TILE);
    
    glm::mat4 view = camera->getView();

    for (unsigned int t = 0; t < tiles.size(); ++t) {
        TileInfo& info = tileInfos.at(t);
        info.offset = (uint32_t)lightIndices.size();
        info.count  = 0;

        Tile& tile = tiles.at(t);

        for (unsigned int i = 0; i < pointLights.size(); ++i) {
            PointLight* light = pointLights.at(i);
            glm::vec3 lightPositionViewSpace = glm::vec3(view * glm::vec4(light->getPosition(), 1.0f));
            float lightRadius = light->getRange();

            if (lightIntersectsTile(lightPositionViewSpace, lightRadius, tile)) {
                lightIndices.push_back(i);
                info.count++;
                if (info.count >= MAX_LIGHTS_PER_TILE) { break; }
            }
        }
    }

    tileTBO->write(tileInfos);
    lightIndicesTBO->write(lightIndices);


    // std::cout << "================================================================================" << std::endl;
    // for (unsigned int y = 0; y < tilesY; ++y) {
    //     for (unsigned int x = 0; x < tilesX; ++x) {
    //         std::cout << "[";

    //         TileInfo& info = tileInfos.at(y * tilesX + x);
    //         for (unsigned int i = info.offset; i < info.offset + info.count; ++i) {
    //             std::cout << lightIndices.at(i) << ",";
    //         }
    //         std::cout << "]";

    //         // unsigned int tileIndex = y * tilesX + x;
    //         // TileInfo& info = tileInfos.at(tileIndex);
    //         // std::cout << info.count << ", ";
    //     }
    //     std::cout << std::endl;
    // }

}

}   