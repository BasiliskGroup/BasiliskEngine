#ifndef TEXTURE_H
#define TEXTURE_H

#include "includes.h"

GLenum getFormat(int nChannels);

class Texture {
    private:
        int width, height, nChannels;
        unsigned int ID;
        unsigned char* data;
    public:
        Texture(const std::string& path);
        unsigned int getID() { return ID; }
};

#endif