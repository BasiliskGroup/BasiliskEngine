#ifndef VAO_H
#define VAO_H

#include "includes.h"
#include "shader.h"
#include "buffer.h"


struct Attribute {
    std::string name;
    GLint location;
    GLenum type;
    GLint count;
    GLsizei size;
};

inline std::vector<Attribute> getAttributes(Shader* shader);
inline unsigned int getStride(const std::vector<Attribute>& attributes);


class VAO {
    private:
        unsigned int ID;
        Shader* shader;
        VBO* vbo;
        EBO* ebo;
        GLenum drawMode;
        unsigned int vertexCount;

        void setupVAO(const std::vector<Attribute>& attributes);
        
    public:
        VAO() {};
        VAO(Shader* shader, VBO* vertices);
        VAO(Shader* shader, VBO* vertices, EBO* indices);
        ~VAO();

        void render(GLenum drawMode=GL_TRIANGLES);
        void bindAttributes(const std::vector<Attribute>& attributes);
        void bindAttribute(GLint location, GLint count, unsigned int stride, unsigned int offset, unsigned int dataType=GL_FLOAT, unsigned int divisor=0);

        inline void bind() { glBindVertexArray(ID); }
        inline void unbind() { glBindVertexArray(0); }

        inline unsigned int getID() { return ID; }
        inline Shader* getShader()  { return shader; }
};


// Helpers
inline unsigned int getStride(const std::vector<Attribute>& attributes) {
    unsigned int stride = 0;
    for (const auto& attribute : attributes) { stride += attribute.size; }
    return stride;
}

inline std::vector<Attribute> getAttributes(Shader* shader) {
    GLint numAttributes = 0;
    GLint maxNameLength = 0;
    
    glGetProgramiv(shader->getID(), GL_ACTIVE_ATTRIBUTES, &numAttributes);
    glGetProgramiv(shader->getID(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);

    std::vector<Attribute> attributes(numAttributes);

    for (int i = 0; i < numAttributes; ++i) {
        std::vector<GLchar> nameBuf(maxNameLength);
        GLsizei length;
        GLint size;
        GLenum type;

        glGetActiveAttrib(shader->getID(), i, maxNameLength, &length, &size, &type, nameBuf.data());

        std::string name(nameBuf.data());
        GLint location = glGetAttribLocation(shader->getID(), name.c_str());
        if (location == -1) continue;

        GLint componentCount;
        GLsizei sizeInBytes;
        GLenum glBaseType;

        switch (type) {
            case GL_FLOAT:        componentCount = 1; sizeInBytes = 4; break;
            case GL_FLOAT_VEC2:   componentCount = 2; sizeInBytes = 4 * 2; break;
            case GL_FLOAT_VEC3:   componentCount = 3; sizeInBytes = 4 * 3; break;
            case GL_FLOAT_VEC4:   componentCount = 4; sizeInBytes = 4 * 4; break;
            default: continue;
        }

        attributes.at(location) = { name, location, type, componentCount, sizeInBytes };
    }

    return attributes;
}

#endif