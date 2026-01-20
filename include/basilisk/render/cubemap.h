#ifndef BSK_CUBEMAP_H
#define BSK_CUBEMAP_H

#include <basilisk/util/includes.h>
#include <basilisk/render/image.h>

namespace bsk::internal {

class Cubemap {
    private:
        unsigned int ID;
        void applyImages(const std::vector<Image*>& faces);
    
    public:
        Cubemap(const std::vector<Image*>& faces);
        Cubemap(const std::vector<std::string>& faces);
        ~Cubemap();

        void bind();
        void unbind();

        inline unsigned int getID() { return ID; }
};

}

#endif