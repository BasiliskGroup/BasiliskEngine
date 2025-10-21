#include "mesh.h"

/**
 * @brief Construct a new Mesh object from a model
 * 
 * @param modelPath The path to the model to load
 */
Mesh::Mesh(const std::string modelPath) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_GenUVCoords);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { return; }

    unsigned int vertexOffset = 0;

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];

        if (!mesh->HasPositions()) { continue; }

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {

            vertices.push_back({
                {
                    mesh->mVertices[i].x,
                    mesh->mVertices[i].y,
                    mesh->mVertices[i].z,
                },
                {
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y,
                },
                {
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z,
                },
            });
        }

        // Process faces (indices)
        for (unsigned int k = 0; k < mesh->mNumFaces; k++) {
            aiFace face = mesh->mFaces[k];
            for (unsigned int l = 0; l < face.mNumIndices; l++) {
                indices.push_back(face.mIndices[l] + vertexOffset);
            }
        }

        // Update the offset for the next mesh
        vertexOffset += mesh->mNumVertices;
    }

    importer.FreeScene();
}