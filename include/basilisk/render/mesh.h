#ifndef BSK_MESH_H
#define BSK_MESH_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Mesh {
    private:
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

    public:
        Mesh(const std::string modelPath, bool generateUV=false, bool generateNormals=false);

        template<typename vertex>
        Mesh(const std::vector<vertex>& vertices): vertices(vertices) {}
        template<typename vertex, typename index>
        Mesh(const std::vector<vertex>& vertices, const std::vector<index>& indices): vertices(vertices), indices(indices) {}

        std::vector<float>& getVertices() { return vertices; }
        std::vector<unsigned int>& getIndices() { return indices; }
};

}

#endif