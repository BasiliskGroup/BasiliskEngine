#ifndef SHADER_H
#define SHADER_H

#include "includes.h"
#include "texture.h"

class Shader {
    private:
        unsigned int program;
    
    public:
        Shader(const char* vertexPath, const char* fragmentPath);
        ~Shader();
        
        void use();
        
        void bind(const std::string&  name, Texture* texture, unsigned int slot);
        
        void setUniform(const std::string&  name, int value);
        void setUniform(const std::string&  name, float value);
        void setUniform(const std::string&  name, double value) { setUniform(name, (float)value); }
        void setUniform(const std::string&  name, glm::mat4 value);

        unsigned int getID() { return program; }
};

#endif