#include <basilisk/render/frame.h>
#include <basilisk/engine/engine.h>
#include <basilisk/util/resolvePath.h>


namespace bsk::internal {

/**
 * @brief Construct a new Frame object. Used as a render target and can render contents to screen. 
 * 
 * @param width Width of the FBO in pixels
 * @param height Height of the FBO in pixels
 */
Frame::Frame(Engine* engine, unsigned int width, unsigned int height): engine(engine), width(width), height(height), aspectRatio((float)width / (float)height) {

    // Load simple shader for rendering a quad wuth texture
    shader = new Shader(internalPath("shaders/frame.vert").c_str(), internalPath("shaders/frame.frag").c_str());

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
 * @brief Render the contents of this frame to screen or currently bound FBO
 * 
 */
void Frame::render() {

    // Get the current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Get the screen dimensions from the engine
    int screenWidth  = engine->getWindow()->getWidth() * engine->getWindow()->getWindowScaleX();
    int screenHeight = engine->getWindow()->getHeight() * engine->getWindow()->getWindowScaleY();
    float screenAspectRatio = (float)screenWidth / (float)screenHeight;

    // Set the render rect based on screen size and this frame's aspect ratio
    int x, y, width, height;
    if (aspectRatio > screenAspectRatio) { // frame is wider than screen
        width = screenWidth;
        height = width / aspectRatio;
        x = 0;
        y = (screenHeight - height) / 2;
    }
    else { // screen is wider than frame
        height = screenHeight;
        width = height * aspectRatio;
        x = (screenWidth - width) / 2;
        y = 0;
    }

    // Update the viewport and render
    glViewport(x, y, width, height);
    shader->use();
    shader->bind("uTexture", fbo, 4);
    vao->render();

    // Reset viewport to previous dimensions
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

/**
 * @brief Render the contents of this frame to screen or currently bound FBO at the specified location
 * 
 * @param x 
 * @param y 
 * @param width 
 * @param height 
 */
void Frame::render(int x, int y, int width, int height) {
    // Get the current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Update the viewport and render
    glViewport(x, y, width, height);
    shader->use();
    shader->bind("uTexture", fbo, 4);
    vao->render();

    // Reset viewport to previous dimensions
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void Frame::clear(float r, float g, float b, float a) {
    fbo->clear(r, g, b, a);
}

unsigned int Frame::getRenderWidth() {
    // Get the screen dimensions from the engine
    int screenWidth  = engine->getWindow()->getWidth() * engine->getWindow()->getWindowScaleX();
    int screenHeight = engine->getWindow()->getHeight() * engine->getWindow()->getWindowScaleY();
    float screenAspectRatio = (float)screenWidth / (float)screenHeight;

    // Set the render rect based on screen size and this frame's aspect ratio
    int x, y, width, height;
    if (aspectRatio > screenAspectRatio) { // frame is wider than screen
        return screenWidth;
    }
    else {
        return screenHeight * aspectRatio;
    }
}

unsigned int Frame::getRenderHeight() {
    // Get the screen dimensions from the engine
    int screenWidth  = engine->getWindow()->getWidth() * engine->getWindow()->getWindowScaleX();
    int screenHeight = engine->getWindow()->getHeight() * engine->getWindow()->getWindowScaleY();
    float screenAspectRatio = (float)screenWidth / (float)screenHeight;

    // Set the render rect based on screen size and this frame's aspect ratio
    int x, y, width, height;
    if (aspectRatio > screenAspectRatio) { // frame is wider than screen
        return screenWidth / aspectRatio;
    }
    else { // screen is wider than frame
        return screenHeight;
    }

}

}