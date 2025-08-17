#include "chunkHandler.h"


std::tuple<int, int, int> getChunkPosition(Node* node) {
    const int CHUNK_SIZE = 40;
    vec3 position = node->getPosition();
    return {floor(position.x / CHUNK_SIZE), floor(position.y / CHUNK_SIZE), floor(position.z / CHUNK_SIZE)};
}


ChunkHandler::ChunkHandler(): chunks({}) {
}


void ChunkHandler::render() {
    for (const auto& pair : chunks) {
        Chunk* chunk = pair.second;
        chunk->render();
    }
}


void ChunkHandler::add(Node* node) {
    std::tuple<int, int, int> position = getChunkPosition(node);

    if (chunks.find(position) == chunks.end()) {
        chunks[position] = new Chunk();
    }

    Chunk* chunk = chunks.at(position);
    chunk->add(node);
}