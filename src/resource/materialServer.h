#ifndef MATERIAL_SERVER_H
#define MATERIAL_SERVER_H

#include "util/includes.h"
#include "render/material.h"
#include "render/shader.h"
#include "render/tbo.h"
#include "resource/textureServer.h"

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

#endif