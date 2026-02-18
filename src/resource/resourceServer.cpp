#include <basilisk/resource/resourceServer.h>
#include <basilisk/render/skybox.h>


namespace bsk::internal {

// Define static members (must include full type)
Image*    ResourceServer::defaultImage = new Image({1.0f, 1.0f, 1.0f, 1.0f}, 1, 1);
Material* ResourceServer::defaultMaterial = new Material();
Mesh*     ResourceServer::defaultCube = new Mesh("models/cube.obj");
Mesh*     ResourceServer::defaultQuad = new Mesh("models/quad.obj");
Skybox*   ResourceServer::defaultSkybox = new Skybox({
    "textures/skybox/right.jpg",
    "textures/skybox/left.jpg",
    "textures/skybox/top.jpg",
    "textures/skybox/bottom.jpg",
    "textures/skybox/front.jpg",
    "textures/skybox/back.jpg"
});


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