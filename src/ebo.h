#ifndef EBO_H
#define EBO_H

#include "includes.h"

class EBO {
    private:
        unsigned int ID;
        unsigned int size;
    public:
        EBO(const void* data, unsigned int size);
        EBO(const std::vector<unsigned int>& data) : EBO(data.data(), data.size() * sizeof(unsigned int)) {}
        ~EBO();

        void bind();
        void unbind();

        unsigned int getSize() { return size; }
};

#endif