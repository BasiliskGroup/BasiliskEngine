#ifndef BSK_TEXTURE_H
#define BSK_TEXTURE_H

#include <basilisk/render/image.h>

namespace bsk::internal {

class Texture {
    private:
        unsigned int id;
        unsigned int width;
        unsigned int height;

        Image* image;

    public:
        Texture(Image* image);
        ~Texture();

        void bind();        
        void setFilter(unsigned int magFilter, unsigned int minFilter);
        void setWrap(unsigned int wrap);

        inline Image* getImage() { return image; }
        inline unsigned int getID() { return id; }
        inline unsigned int getWidth() { return width; }
        inline unsigned int getHeight() { return height; }
};

}

#endif