#include <basilisk/resource/materialServer.h>

namespace bsk::internal {

MaterialServer::MaterialServer(TextureServer* textureServer): textureServer(textureServer) {
    tbo = new TBO(nullptr, 0, 20000);
}

MaterialServer::~MaterialServer() {
    delete tbo;
}

/**
 * @brief Get the location of the material in the tbo. 
 *        Starting byte location will be the index returned by this function * sizeof(MaterialData)
 * 
 * @param material The material to get
 * @return unsigned int 
 */
unsigned int MaterialServer::get(Material* material) {
    return materialMapping.at(material);
}

/**
 * @brief Add a new material to the server. 
 * 
 * @param material The material to add
 * @return unsigned int Location of the material in the tbo. 
 */
unsigned int MaterialServer::add(Material* material) {
    // Do not add if the material is ready on the tbo
    if (materialMapping.count(material)) {
        return get(material);
    }

    // Get the material data
    MaterialData data = material->getData();
    
    // Get the location of the albedo and noraml maps in theif texture server
    std::pair<unsigned int, unsigned int> albedo = textureServer->add(material->getAlbedo());
    std::pair<unsigned int, unsigned int> normal = textureServer->add(material->getNormal());
    
    // Update material data to have correct texture array locations
    data.albedoArray = albedo.first;
    data.albedoIndex = albedo.second;
    data.normalArray = normal.first;
    data.normalIndex = normal.second;

    // Write the material data to the next open slot of the tbo
    tbo->write(&data, sizeof(data), tbo->getSize());

    // Add the material to the mapping
    unsigned int index = materialMapping.size();
    materialMapping[material] = index;

    return index;
}

void MaterialServer::write(Shader* shader, std::string name, unsigned int slot) {
    shader->bind(name.c_str(), tbo, slot);
}

}