#ifndef RESOURCE_SERVER_H
#define RESOURCE_SERVER_H

#include "util/includes.h"
#include "render/shader.h"
#include "resource/textureServer.h"
#include "resource/materialServer.h"

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

#endif