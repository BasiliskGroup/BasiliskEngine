#ifndef VAO_H
#define VAO_H

#include "includes.h"

#include "shader.h"
#include "vbo.h"
#include "ebo.h"

class VAO {
    private:
        unsigned int ID;
        Shader* shader;
        VBO* vbo;
        EBO* ebo;
        unsigned int vertexCount;
    public:
        VAO() {};
        VAO(Shader* shader, VBO* vertices);
        VAO(Shader* shader, VBO* vertices, EBO* indices);
        ~VAO();

        void render();
        unsigned int bindAttributes(Shader* shader);
        void bind();
        void unbind();

        unsigned int getID() {return ID;}
        Shader* getShader() {return shader;}
};

#endif