#ifndef TEXTURE_ARRAY_H
#define TEXTURE_ARRAY_H

#include "util/includes.h"
#include "render/image.h"

class TextureArray {
    private:
        unsigned int id;
        
        unsigned int capacity;
        unsigned int width;
        unsigned int height;
        std::vector<Image*> images;

        void generate();
        void uploadImage(Image* image, unsigned int position);
        
    public:
        TextureArray(unsigned int width, unsigned int height, std::vector<Image*> images={}, unsigned int capacity=1);
        ~TextureArray();

        void bind();
        void unbind();
        unsigned int add(Image* image);
        void setFilter(unsigned int magFilter, unsigned int minFilter);
        void setWrap(unsigned int wrap);

        unsigned int getID() { return id; }
        unsigned int getSize() { return images.size(); }

};

#endif