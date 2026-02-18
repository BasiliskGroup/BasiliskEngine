#include <basilisk/resource/resourceServer.h>
#include <basilisk/render/skybox.h>
#include <basilisk/util/resolvePath.h>

// internalPath("models/cube.obj").c_str()

namespace bsk::internal {

// Define static members (must include full type)
// Initialize simple ones at global scope, but defer Mesh/Skybox initialization
// to avoid static initialization order issues with Assimp
Image*    ResourceServer::defaultImage = new Image({0.5f, 0.5f, 0.5f, 1.0f}, 1, 1);
Material* ResourceServer::defaultMaterial = new Material({1.0f, 1.0f, 1.0f}, ResourceServer::defaultImage);
Mesh*     ResourceServer::defaultCube = nullptr;
Mesh*     ResourceServer::defaultQuad = nullptr;
Skybox*   ResourceServer::defaultSkybox = nullptr;


ResourceServer::ResourceServer() {
    textureServer = new TextureServer();
    materialServer = new MaterialServer(textureServer);
    
    // Initialize static resources that require Assimp (deferred to avoid static init order issues)
    if (defaultCube == nullptr) {
        defaultCube = new Mesh(internalPath("models/cube.obj").c_str());
    }
    if (defaultQuad == nullptr) {
        defaultQuad = new Mesh(internalPath("models/quad.obj").c_str());
    }
    if (defaultSkybox == nullptr) {
        defaultSkybox = new Skybox({
            internalPath("textures/skybox/right.jpg").c_str(),
            internalPath("textures/skybox/left.jpg").c_str(),
            internalPath("textures/skybox/top.jpg").c_str(),
            internalPath("textures/skybox/bottom.jpg").c_str(),
            internalPath("textures/skybox/front.jpg").c_str(),
            internalPath("textures/skybox/back.jpg").c_str()
        });
    }
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