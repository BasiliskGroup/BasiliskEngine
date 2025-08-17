#ifndef BUFFER_H
#define BUFFER_H

#include "includes.h"

class Buffer {
    protected:
        unsigned int ID;
        unsigned int size;
        unsigned int bufferType;

    public:
        template<typename T>
        Buffer(const std::vector<T>& data, unsigned int bufferType, bool dynamic=false) : Buffer(data.data(), data.size() * sizeof(T), bufferType=GL_ARRAY_BUFFER, dynamic=false) {}
        Buffer(const void* data, unsigned int size, unsigned int bufferType=GL_ARRAY_BUFFER, bool dynamic=false);
        ~Buffer();

        template<typename T>
        inline void write(const std::vector<T>& data, unsigned int offset = 0) { write(data.data(), data.size() * sizeof(T), offset); }
        void write(const void* data, unsigned int size, unsigned int offset = 0);

        inline void bind()   { glBindBuffer(bufferType, ID); }
        inline void unbind() { glBindBuffer(bufferType, 0); }
        inline unsigned int getSize() { return size; }
};



class VBO : public Buffer {
public:
    template<typename T>
    VBO(const std::vector<T>& data, bool dynamic=false)
        : VBO(data.data(), data.size() * sizeof(T), dynamic) {}
    
    VBO(const void* data, unsigned int size, bool dynamic=false)
        : Buffer(data, size, GL_ARRAY_BUFFER, dynamic) {}
};

class EBO : public Buffer {
public:
    template<typename T>
    EBO(const std::vector<T>& data, bool dynamic=false)
        : EBO(data.data(), data.size() * sizeof(T), dynamic) {}
    
    EBO(const void* data, unsigned int size, bool dynamic=false)
        : Buffer(data, size, GL_ELEMENT_ARRAY_BUFFER, dynamic) {}
};

#endif