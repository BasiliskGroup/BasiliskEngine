#ifndef VBO_H
#define VBO_H

#include "includes.h"

class VBO {
    private:
        unsigned int ID;
        unsigned int size;
    public:
        VBO(const void* data, unsigned int size);
        template<typename T>
        VBO(const std::vector<T>& data) : VBO(data.data(), data.size() * sizeof(T)) {}
        ~VBO();

        // void release();
        void bind();
        void unbind();

        unsigned int getSize() { return size; }
};

#endif