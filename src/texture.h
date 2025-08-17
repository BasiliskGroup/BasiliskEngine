#ifndef TEXTURE_H
#define TEXTURE_H

#include "includes.h"

class Texture {
    private:
        int width, height, nChannels;
        unsigned int ID;
        unsigned char* data;

    public:
        Texture(const std::string& path);
        unsigned int getID() { return ID; }
};

inline GLenum getFormat(int nChannels) {
    switch (nChannels) {
        case 1: return GL_RED;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default: return GL_RGB;
    }
}

#endif