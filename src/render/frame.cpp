#include <basilisk/render/frame.h>


namespace bsk::internal {

/**
 * @brief Construct a new Frame object. Used as a render target and can render contents to screen. 
 * 
 * @param width Width of the FBO in pixels
 * @param height Height of the FBO in pixels
 */
Frame::Frame(unsigned int width, unsigned int height): width(width), height(height) {

    // Load simple shader for rendering a quad wuth texture
    shader = new Shader("shaders/frame.vert", "shaders/frame.frag");

    // Create data needed to render a full-screen quad
    std::vector<float> vertexData = {
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };
    std::vector<unsigned int> indexData = {
        0, 1, 2,
        2, 3, 0
    };

    // Create render data and objects
    vbo = new VBO(vertexData);
    ebo = new EBO(indexData);
    vao = new VAO(shader, vbo, ebo);

    // Create an FBO as a render target and sampler
    fbo = new FBO(width, height, 4);
    shader->bind("uTexture", fbo, 4);
}

/**
 * @brief Destroy the Frame object
 * 
 */
Frame::~Frame() {
    delete fbo;
    delete ebo;
    delete vbo;
    delete vao;
    delete shader;
}

/**
 * @brief Use this frame as a render target
 * 
 */
void Frame::use() {
    fbo->bind();
}

/**
 * @brief Render the contents of this frame to screen or currently bounod FBO
 * 
 */
void Frame::render() {
    shader->use();
    vao->render();
}

void Frame::clear(float r, float g, float b, float a) {
    fbo->clear(r, g, b, a);
}

}