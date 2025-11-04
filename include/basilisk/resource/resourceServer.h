#ifndef BSK_RESOURCE_SERVER_H
#define BSK_RESOURCE_SERVER_H

#include <basilisk/util/includes.h>
#include <basilisk/render/shader.h>
#include <basilisk/resource/textureServer.h>
#include <basilisk/resource/materialServer.h>

namespace bsk::internal {

class ResourceServer {
    private:
        TextureServer* textureServer;
        MaterialServer* materialServer;

    public:
        ResourceServer();
        ~ResourceServer();

        void write(Shader* shader, std::string textureUniform, std::string materialUniform, unsigned int startingSlot=8);

        inline TextureServer* getTextureServer() const { return textureServer; }
        inline MaterialServer* getMaterialServer() const { return materialServer; }
};

}

#endif