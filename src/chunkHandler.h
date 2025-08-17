#ifndef CHUNK_HANDLER_H
#define CHUNK_HANDLER_H

#include "includes.h"
#include "chunk.h"


// A custom hash function for the specific tuple type
struct TupleHash {
    size_t operator()(const std::tuple<int, int, int>& t) const {
        size_t h1 = std::hash<int>()(std::get<0>(t));
        size_t h2 = std::hash<int>()(std::get<1>(t));
        size_t h3 = std::hash<int>()(std::get<2>(t));

        // A simple hash combination function
        // For more robust hashing, consider using a library like Boost.Hash.
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};



class ChunkHandler {
    private:
        std::unordered_map<std::tuple<int, int, int>, Chunk*, TupleHash> chunks;

    public:
        ChunkHandler();

        void render();

        void add(Node* node);
        void remove(Node* node);
};

#endif