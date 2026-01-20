#include <basilisk/render/skybox.h>

namespace bsk::internal {

/**
 * @brief Construct a new Skybox object
 * 
 * @param cubemap 
 */
Skybox::Skybox(Cubemap* cubemap, bool ownsCubemap): cubemap(cubemap), ownsCubemap(ownsCubemap) {
    // Create a shader for the skybox
    shader = new Shader(internalPath("shaders/skybox.vert").c_str(), internalPath("shaders/skybox.frag").c_str());
    // Create a VBO and VAO for the skybox
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    vbo = new VBO(skyboxVertices, sizeof(skyboxVertices));
    vao = new VAO(shader, vbo);
}

Skybox::~Skybox() {
    if (ownsCubemap) {
        delete cubemap;
    }
    delete shader;
    delete vbo;
    delete vao;
}

void Skybox::render(StaticCamera* camera) {
    glDepthMask(GL_FALSE);
    shader->bind("skybox", cubemap, 6);
    shader->setUniform("uView", glm::mat4(glm::mat3(camera->getView())));
    shader->setUniform("uProjection", camera->getProjection());
    vao->render();
    glDepthMask(GL_TRUE);
}

}