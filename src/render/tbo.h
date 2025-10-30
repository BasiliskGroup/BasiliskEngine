#ifndef TBO_H
#define TBO_H

#include "util/includes.h"


class TBO {
    private:
        unsigned int ID;
        unsigned int size;
        unsigned int capacity;
        unsigned int textureID;

    public:
        TBO(const void* data, unsigned int size, unsigned int reserve = 0);
        template<typename T>
        TBO(const std::vector<T>& data, unsigned int reserve = 0) : TBO(data.data(), data.size() * sizeof(T), reserve) {}

        ~TBO();

        void bind();
        void unbind();

        inline unsigned int getSize() { return size; }
        inline unsigned int getID() { return ID; }
        inline unsigned int getTextureID() { return textureID; }

        void write(const void* data, unsigned int size, unsigned int offset=0);
        template<typename T>
        inline void write(const std::vector<T>& data, unsigned int offset) {
            write(data.data(), data.size() * sizeof(T), offset);
        }

    private:
        void resize();
};

#endif