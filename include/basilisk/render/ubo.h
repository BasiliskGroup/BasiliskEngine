#ifndef BSK_UBO_H
#define BSK_UBO_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class UBO {
    private:
        unsigned int ID;
        unsigned int size;

    public:
        UBO(const void* data, unsigned int size, unsigned int drawType=GL_STATIC_DRAW);
        template<typename T>
        UBO(const std::vector<T>& data, unsigned int drawType=GL_STATIC_DRAW) : UBO(data.data(), data.size() * sizeof(T), drawType) {}

        ~UBO();

        void bind();
        void unbind();
        inline unsigned int getID() { return ID; }
        inline unsigned int getSize() { return size; }

        void write(const void* data, unsigned int size, unsigned int offset=0);
        template<typename T>
        void write(const std::vector<T>& data, unsigned int offset=0);
};

}

#endif