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
        Image(std::string file, bool flip_vertically=true);
        Image(void* data, int width, int height, int nChannels=4) : data((unsigned char*)data), width(width), height(height), nChannels(nChannels) {}
        Image(const std::vector<float>& data, int width, int height, int nChannels=4);
        ~Image();
        
        unsigned char* getData() { return data; }
        int getWidth() { return width; }
        int getHeight() {return height; }
};

}

#endif