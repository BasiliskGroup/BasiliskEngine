#ifndef BSK_INSTANCER_H
#define BSK_INSTANCER_H

#include <basilisk/util/includes.h>
#include <basilisk/render/shader.h>
#include <basilisk/render/vbo.h>
#include <basilisk/render/vao.h>
#include <basilisk/render/ebo.h>
#include <basilisk/render/mesh.h>

namespace bsk::internal {

template <typename T>
class Instancer {
    private:
        Shader* shader;
        Mesh* mesh;
        VBO* vbo;
        VAO* vao;
        EBO* ebo;

        unsigned int instanceVBO;
        unsigned int capacity;
        unsigned int size;
        std::vector<T> instanceData;

        void uploadInstanceData();
        void resize();

    public:
        Instancer(Shader* shader, Mesh* mesh, std::vector<std::string> modelFormat, std::vector<std::string> instanceFormat, unsigned int reserve=1);
        ~Instancer();

        void add(T objectData);
        void remove();

        void render();
};

}

#include <basilisk/instancer.tpp>

#endif
