#ifndef MATERIAL_SERVER_H
#define MATERIAL_SERVER_H

#include "util/includes.h"
#include "render/material.h"

class MaterialServer {
    private:
        std::unordered_map<Material*, unsigned int>

    public:
        MaterialServer();

        unsigned int add(Material* material);
};

#endif