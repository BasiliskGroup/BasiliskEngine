#ifndef BSK_SHADER_H
#define BSK_SHADER_H

#include <basilisk/util/includes.h>
#include <basilisk/render/texture.h>
#include <basilisk/render/textureArray.h>
#include <basilisk/render/tbo.h>
#include <basilisk/render/fbo.h>

namespace bsk::internal {

// Used to store slot binding information
struct BoundTexture {
    GLuint id;
    GLenum target;
};

// GLint location, GLint count, unsigned int dataType, unsigned int stride, unsigned int offset
struct Attribute {
    std::string name;
    GLint location;
    GLint count;
    GLenum dataType;
    unsigned int offset;
};

class Shader {
    private:
        unsigned int ID;
        unsigned int stride;
        std::vector<Attribute> attributes;
        std::unordered_map<unsigned int, BoundTexture> slotBindings;

        void loadAttributes();

        void bindTextureToSlot(const char* name, GLuint texID, GLenum target, unsigned int slot);

    public:
        Shader(const char* vertexPath, const char* fragmentPath);
        ~Shader();

        void use();

        void bind(const char* name, Texture* texture, unsigned int slot);
        void bind(const char* name, TextureArray* textureArray, unsigned int slot);
        void bind(const char* name, TBO* tbo, unsigned int slot);
        void bind(const char* name, FBO* fbo, unsigned int slot);

        int getUniformLocation(const char* name);
        unsigned int getStride() { return stride; }
        std::vector<Attribute>& getAttributes() { return attributes; }

        void setUniform(const char* name, float value);
        void setUniform(const char* name, double value);
        void setUniform(const char* name, int value);
        void setUniform(const char* name, glm::vec3 value);
        void setUniform(const char* name, glm::mat4 value);

        unsigned int getID() { return ID; }
};

}

#endif