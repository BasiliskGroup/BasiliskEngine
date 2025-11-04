#ifndef BSK_IMAGE_H
#define BSK_IMAGE_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Image {
    private:
        unsigned char* data;
        int width;
        int height;
        int nChannels;
    
    public:
        Image(std::string file);
        ~Image();
        
        unsigned char* getData() { return data; }
        int getWidth() { return width; }
        int getHeight() {return height; }
};

}

#endif