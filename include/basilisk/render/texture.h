#ifndef BSK_TEXTURE_H
#define BSK_TEXTURE_H

#include <basilisk/render/image.h>

namespace bsk::internal {

class Texture {
    private:
        unsigned int id;

    public:
        Texture(Image* image);
        ~Texture();

        void bind();        
        void setFilter(unsigned int magFilter, unsigned int minFilter);
        void setWrap(unsigned int wrap);

        unsigned int getID() { return id; }
};

}

#endif