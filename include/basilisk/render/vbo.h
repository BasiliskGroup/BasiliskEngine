#ifndef BSK_VBO_H
#define BSK_VBO_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class VBO {
    private:
        unsigned int ID;
        unsigned int size;

    public:
        VBO(const void* data, unsigned int size, unsigned int drawType=GL_STATIC_DRAW);
        template<typename T>
        VBO(const std::vector<T>& data, unsigned int drawType=GL_STATIC_DRAW) : VBO(data.data(), data.size() * sizeof(T), drawType) {}
            
        ~VBO(); 

        void bind() ;
        void unbind();
        unsigned int getSize();

        void write(const void* data, unsigned int size, unsigned int offset=0);
        template<typename T>
        void write(const std::vector<T>& data, unsigned int offset=0);

};

}

#endif