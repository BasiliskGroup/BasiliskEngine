#include <basilisk/resource/resourceServer.h>

namespace bsk::internal {

ResourceServer::ResourceServer() {
    textureServer = new TextureServer();
    materialServer = new MaterialServer(textureServer);
}

ResourceServer::~ResourceServer() {
    delete textureServer;
    delete materialServer;
}

void ResourceServer::write(Shader* shader, std::string textureUniform, std::string materialUniform, unsigned int startingSlot) {
    materialServer->write(shader, materialUniform, startingSlot);
    textureServer->write(shader, textureUniform, startingSlot + 1);
}

}