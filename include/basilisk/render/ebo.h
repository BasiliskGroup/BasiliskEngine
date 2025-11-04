#ifndef BSK_EBO_H
#define BSK_EBO_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class EBO {
    private:
        unsigned int ID;
        unsigned int size;

    public:
        EBO(const void* data, unsigned int size, unsigned int drawType=GL_STATIC_DRAW);
        template<typename T>
        EBO(const std::vector<T>& data, unsigned int drawType=GL_STATIC_DRAW) : EBO(data.data(), data.size() * sizeof(T), drawType) {}
        
        ~EBO(); 

        void bind() ;
        void unbind();
        unsigned int getSize();

        void write(const void* data, unsigned int size, unsigned int offset=0);
        template<typename T>
        void write(const std::vector<T>& data, unsigned int offset=0);

};

}

#endif