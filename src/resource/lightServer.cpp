#include <basilisk/resource/lightServer.h>

namespace bsk::internal {

/**
 * @brief Construct a new Light Server object
 * 
 */
LightServer::LightServer() {
    directionalLightData = std::vector<glm::vec4>(MAX_DIRECTIONAL_LIGHTS * 2);
    pointLightData = std::vector<glm::vec4>(MAX_POINT_LIGHTS * 2);
    ambientLightData = glm::vec3(0.0, 0.0, 0.0);

    directionalLightsUBO = new UBO(nullptr, MAX_DIRECTIONAL_LIGHTS * 2 * sizeof(glm::vec4), GL_DYNAMIC_DRAW);
    pointLightsTBO = new TBO(pointLightData);
    tileTBO = nullptr;
    lightIndicesTBO = nullptr;
}

/**
 * @brief Destroy the Light Server object
 * 
 */
LightServer::~LightServer() {
    delete directionalLightsUBO;
    delete tileTBO;
    delete lightIndicesTBO;
    delete pointLightsTBO;
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
void LightServer::update(Shader* shader, StaticCamera* camera) {
    updateDirectional(shader);
    updateAmbient(shader);
    updatePoint(shader, camera);
}

/**
 * @brief Update and write directional data
 * 
 * @param shader Shader to write to
 */
void LightServer::updateDirectional(Shader* shader) {
    // Get and write all directional light data
    for (size_t i = 0; i < directionalLights.size(); i++) {
        directionalLightData[i * 2] = glm::vec4(directionalLights[i]->getColor(), directionalLights[i]->getIntensity());
        directionalLightData[i * 2 + 1] = glm::vec4(directionalLights[i]->getDirection(), 0.0);
    }
    directionalLightsUBO->write(directionalLightData);

    // Bind ubo and upload the number of lights
    shader->bind("uDirectionalLights", directionalLightsUBO, 0);
    shader->setUniform("uDirectionalLightCount", (int)directionalLights.size());
}

/**
 * @brief Update and write point light data
 * 
 * @param shader Shader to write to
 * @param camera Camera to use for view and to center lights around
 */
void LightServer::updatePoint(Shader* shader, StaticCamera* camera) {
    // Sort lights by distance from the camera
    std::sort(pointLights.begin(), pointLights.end(), [camera](PointLight* a, PointLight* b) {
        return glm::length(camera->getPosition() - a->getPosition()) < glm::length(camera->getPosition() - b->getPosition());
    });

    // Update tiles
    updateTiles(camera);

    // Get and write point lights data
    size_t lightCount = std::min(pointLights.size(), static_cast<size_t>(MAX_POINT_LIGHTS));
    for (size_t i = 0; i < lightCount; i++) {
        pointLightData[i * 2] = glm::vec4(pointLights[i]->getColor(), pointLights[i]->getIntensity());
        pointLightData[i * 2 + 1] = glm::vec4(pointLights[i]->getPosition(), pointLights[i]->getRange());
    }
    pointLightsTBO->write(pointLightData);

    // Bind point light tbo
    shader->bind("uPointLights", pointLightsTBO, 15);
}

/**
 * @brief Update and write ambient light data
 * 
 * @param shader Shader to write to
 */
void LightServer::updateAmbient(Shader* shader) {
    // Get weighted sum of all ambient lights
    ambientLightData = glm::vec3(0.0, 0.0, 0.0);
    for (AmbientLight* light : ambientLights) {
        ambientLightData += light->getColor() * light->getIntensity();
    }

    // Write the ambient sum to uniform
    shader->setUniform("uAmbientLight", ambientLightData);
}

/**
 * @brief Update the screen tiles with the lights that intersect them
 * 
 * @param camera Camera to use for view
 */
void LightServer::updateTiles(StaticCamera* camera) {
    // Clear and reserve space for light indices
    lightIndices.clear();
    lightIndices.reserve(tiles.size() * MAX_LIGHTS_PER_TILE);
    
    // Cache the view position and range of each light (range for locallity)
    glm::mat4 view = camera->getView();
    std::vector<std::pair<glm::vec3, float>> lightData;
    for (unsigned int i = 0; i < pointLights.size(); ++i) {
        PointLight* light = pointLights.at(i);
        glm::vec3 lightPositionViewSpace = glm::vec3(view * glm::vec4(light->getPosition(), 1.0f));
        lightData.push_back({lightPositionViewSpace, light->getRange()});
    }

    // Get all lights in each tile
    for (unsigned int t = 0; t < tiles.size(); ++t) {
        // Get tile and initialize basic info
        Tile& tile = tiles.at(t);
        TileInfo& info = tileInfos.at(t);
        info.offset = (uint32_t)lightIndices.size();
        info.count  = 0;

        // Add all lights in the tile
        for (unsigned int i = 0; i < pointLights.size(); ++i) {
            // Get light data
            glm::vec3 lightPositionViewSpace = lightData[i].first;
            float lightRadius = lightData[i].second;
            // Check for intersection
            if (lightIntersectsTile(lightPositionViewSpace, lightRadius, tile)) {
                lightIndices.push_back(i);
                info.count++;
                // Limit lights in tile
                if (info.count >= MAX_LIGHTS_PER_TILE) { break; }
            }
        }
    }

    // Write updated tbo data
    tileTBO->write(tileInfos);
    lightIndicesTBO->write(lightIndices);
}

glm::vec3 LightServer::unproject(const glm::mat4& inverseProjection, float x, float y) {
    glm::vec4 clip(x, y, -1.0f, 1.0f);
    glm::vec4 view = inverseProjection * clip;
    return glm::normalize(glm::vec3(view) / view.w);
}

bool LightServer::lightIntersectsTile(glm::vec3& lightPositionViewSpace, float lightRadius, Tile& tile) {
    if (lightPositionViewSpace.z - lightRadius > 0.0f)
        return false;

    for (int i = 0; i < 4; i++) {
        float distance = glm::dot(tile.planes[i].normal, lightPositionViewSpace);
        if (distance < -(lightRadius * 1.05)) { return false; }
    }

    return true;
}

/**
 * @brief Initialize tiles based on screen and camera
 * 
 * @param shader 
 * @param camera 
 * @param screenWidth 
 * @param screenHeight 
 */
void LightServer::setTiles(Shader* shader, StaticCamera* camera, unsigned int screenWidth, unsigned int screenHeight) {
    // Set number of tiles in each direction
    tilesX = (unsigned int)ceil((float)screenWidth  / (float)TILE_SIZE);
    tilesY = (unsigned int)ceil((float)screenHeight / (float)TILE_SIZE);
    unsigned int tileCount = tilesX * tilesY;

    // Resize tile structures 
    tiles.resize(tileCount);
    tileInfos.resize(tileCount);
    lightIndices.resize(tileCount * MAX_LIGHTS_PER_TILE);

    // Get camera projection and inverse projection
    glm::mat4 projection = camera->getProjection();
    glm::mat4 inverseProjection = glm::inverse(projection);

    // Loop through each tile
    for (unsigned int ty = 0; ty < tilesY; ++ty) {
        for (unsigned int tx = 0; tx < tilesX; ++tx) {
            // Get tile
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

    // Delete any existing TBOs
    if (tileTBO) { delete tileTBO; }
    if (lightIndicesTBO) { delete lightIndicesTBO; }

    // Reallocate
    tileTBO = new TBO(tileInfos);
    lightIndicesTBO = new TBO(lightIndices);
    
    // Bind TBOs
    shader->bind("uLightTiles", tileTBO, 7);
    shader->bind("uLightIndices", lightIndicesTBO, 14);
}


}   