#ifndef BSK_RESOURCE_SERVER_H
#define BSK_RESOURCE_SERVER_H

#include <basilisk/util/includes.h>
#include <basilisk/render/shader.h>
#include <basilisk/resource/textureServer.h>
#include <basilisk/resource/materialServer.h>
#include <basilisk/render/material.h>
#include <basilisk/render/image.h>
#include <basilisk/render/mesh.h>


namespace bsk::internal {

class Skybox;

class ResourceServer {
    private:
        TextureServer* textureServer;
        MaterialServer* materialServer;

    public:
        ResourceServer(std::vector<unsigned int> textureSizeBuckets = {256, 1024, 2048, 2200}, unsigned int textureFilter = GL_LINEAR);
        ~ResourceServer();

        void write(Shader* shader, std::string textureUniform, std::string materialUniform, unsigned int startingSlot=8);

        inline TextureServer* getTextureServer() const { return textureServer; }
        inline MaterialServer* getMaterialServer() const { return materialServer; }

        static Image*    defaultImage;
        static Image*    logoImage;
        static Material* defaultMaterial;
        static Mesh*     defaultCube;
        static Mesh*     defaultQuad;
        static Skybox*   defaultSkybox;
};

}

#endif