#ifndef BSK_FBO_H
#define BSK_FBO_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class FBO {
    private:
        unsigned int ID;
        unsigned int texture;
        unsigned int depth;

        unsigned int width;
        unsigned int height;
    
    public:
        FBO(unsigned int width, unsigned int height, unsigned int components=4);
        ~FBO();

        void bind();
        void unbind();
        void clear(float r=0.0, float g=0.0, float b=0.0, float a=1.0);

        inline unsigned int getID() { return ID; }
        inline unsigned int getTextureID() { return texture; }
        inline unsigned int getDepthID() { return depth; }
        inline unsigned int getWidth() { return width; }
        inline unsigned int getHeight() { return height; }
};

}

#endif