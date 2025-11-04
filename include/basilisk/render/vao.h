#ifndef BSK_VAO_H
#define BSK_VAO_H

#include <basilisk/util/includes.h>
#include <basilisk/render/vbo.h>
#include <basilisk/render/ebo.h>
#include <basilisk/render/shader.h>

namespace bsk::internal {

class VAO {
    private:
        unsigned int ID;
        Shader* shader;
        VBO* vbo;
        EBO* ebo;

    public:
        VAO();
        VAO(Shader* shader, VBO* vertices, EBO* indices=nullptr);
        ~VAO();

        void render(unsigned int instanceCount=0);
        
        void bind();
        void bindAttribute(GLint location, GLint count, unsigned int dataType, unsigned int stride, unsigned int offset, unsigned int divisor=0);
        void bindAttributes(std::vector<std::string> attribs, unsigned int divisor=0);
        void bindBuffer(VBO* buffer, std::vector<std::string> attribs, unsigned int divisor=0);
        void bindBuffer(VBO* buffer, EBO* indices, std::vector<std::string> attribs, unsigned int divisor=0);
};

}

#endif