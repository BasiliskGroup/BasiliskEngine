#ifndef BSK_MATERIAL_SERVER_H
#define BSK_MATERIAL_SERVER_H

#include <basilisk/util/includes.h>
#include <basilisk/render/material.h>
#include <basilisk/render/shader.h>
#include <basilisk/render/tbo.h>
#include <basilisk/resource/textureServer.h>

namespace bsk::internal {

class MaterialServer {
    private:
        TextureServer* textureServer;
        TBO* tbo;

        std::unordered_map<Material*, unsigned int> materialMapping;

    public:
        MaterialServer(TextureServer* textureServer);
        ~MaterialServer();

        unsigned int add(Material* material);
        unsigned int get(Material* material);

        void write(Shader* shader, std::string name, unsigned int startSlot = 0);
};

}

#endif