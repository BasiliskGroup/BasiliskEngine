#include "vao.h"


struct Attribute {
    std::string name;
    GLint location;
    GLenum type;
    GLint count;
    GLsizei size;
};


unsigned int getStride(const std::vector<Attribute>& attributes) {
    // Calculate stride based on attributes
    unsigned int stride = 0;
    for (const auto& attribute : attributes) {
        stride += attribute.size;
    }
    return stride;
}

std::vector<Attribute> getAttributes(Shader* shader) {
    GLint numAttributes = 0;
    GLint maxNameLength = 0;
    
    glGetProgramiv(shader->getID(), GL_ACTIVE_ATTRIBUTES, &numAttributes);
    glGetProgramiv(shader->getID(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);

    std::vector<Attribute> attributes;
    attributes.reserve(numAttributes);

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

        attributes.push_back({ name, location, type, componentCount, sizeInBytes });
    }

    return attributes;
}


unsigned int VAO::bindAttributes(Shader* shader) {

    std::vector<Attribute> attributes = getAttributes(shader);
    unsigned int stride = getStride(attributes);

    unsigned int offset = 0;
    for (const Attribute& attribute : attributes) {
        int location = glGetAttribLocation(shader->getID(), attribute.name.c_str());
        if (location == -1) { continue; }
        
        // Set the attribute parameters
        glVertexAttribPointer(location, attribute.count, GL_FLOAT, GL_FALSE, stride, (const void*)(GLintptr)offset);
        glEnableVertexAttribArray(location);
        
        // Jump offset forward
        offset += attribute.size;
    }

    return stride;
}

VAO::VAO(Shader* shader, VBO* vertices): shader(shader), vbo(vertices), ebo(nullptr) {
    glGenVertexArrays(1, &ID);
    
    // Bindings
    bind();
    vbo->bind();
    unsigned int stride = bindAttributes(shader);

    // Get the vertex count for rendering
    vertexCount = vbo->getSize() / stride;

    unbind(); 
}

VAO::VAO(Shader* shader, VBO* vertices, EBO* indices): shader(shader), vbo(vertices), ebo(indices) {
    glGenVertexArrays(1, &ID);
    
    // Bindings
    bind();
    vbo->bind();
    ebo->bind();
    bindAttributes(shader);

    // Get the vertex count for rendering
    vertexCount = ebo->getSize() / sizeof(unsigned int);

    unbind(); 
}

VAO::~VAO() { glDeleteVertexArrays(1, &ID); }
void VAO::bind() { glBindVertexArray(ID); }
void VAO::unbind() { glBindVertexArray(0); }

void VAO::render() {
    shader->use();
    bind();
    if (ebo)
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    unbind();
}