#include <basilisk/render/frame.h>
#include <basilisk/engine/engine.h>
#include <basilisk/util/resolvePath.h>
#include <cstdlib>


namespace bsk::internal {

Frame::Frame(Engine* engine, Shader* shader, unsigned int width, unsigned int height): shader(shader), engine(engine), width(width), height(height), aspectRatio((float)width / (float)height), customShader(true) {
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
 * @brief Construct a new Frame object. Used as a render target and can render contents to screen. 
 * 
 * @param width Width of the FBO in pixels
 * @param height Height of the FBO in pixels
 */
Frame::Frame(Engine* engine, unsigned int width, unsigned int height): Frame(engine, new Shader(internalPath("shaders/frame.vert").c_str(), internalPath("shaders/frame.frag").c_str()), width, height) {
    customShader = false;
}

/**
 * @brief Destroy the Frame object
 * 
 */
Frame::~Frame() {
    if (!customShader) {
        delete shader;
    }
    delete fbo;
    delete ebo;
    delete vbo;
    delete vao;
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
    int screenWidth  = viewport[2] * engine->getWindow()->getWindowScaleX();
    int screenHeight = viewport[3] * engine->getWindow()->getWindowScaleY();
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
    shader->setUniform("textureSize", glm::vec2(this->width, this->height));
    // Draw frame as fully opaque so we don't blend with window clear color
    // (otherwise semi-transparent black in the frame would become gray)
    GLint blendSrc, blendDst;
    glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrc);
    glGetIntegerv(GL_BLEND_DST_RGB, &blendDst);
    glBlendFunc(GL_ONE, GL_ZERO);
    vao->render();
    glBlendFunc(blendSrc, blendDst);
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
    GLint blendSrc, blendDst;
    glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrc);
    glGetIntegerv(GL_BLEND_DST_RGB, &blendDst);
    glBlendFunc(GL_ONE, GL_ZERO);
    vao->render();
    glBlendFunc(blendSrc, blendDst);
    // Reset viewport to previous dimensions
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void Frame::render(unsigned int textureID) {
    // Get the current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Get the screen dimensions from the engine
    int screenWidth  = viewport[2] * engine->getWindow()->getWindowScaleX();
    int screenHeight = viewport[3] * engine->getWindow()->getWindowScaleY();
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
    // Bind the texture to the shader
    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D, textureID);
    shader->setUniform("uTexture", 4);
    // Set the texture size
    shader->setUniform("textureSize", glm::vec2(this->width, this->height));
    // Draw frame as fully opaque so we don't blend with window clear color
    // (otherwise semi-transparent black in the frame would become gray)
    GLint blendSrc, blendDst;
    glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrc);
    glGetIntegerv(GL_BLEND_DST_RGB, &blendDst);
    glBlendFunc(GL_ONE, GL_ZERO);
    vao->render();
    glBlendFunc(blendSrc, blendDst);
    // Reset viewport to previous dimensions
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void Frame::render(unsigned int textureID, int x, int y, int width, int height) {
     // Get the current viewport
     GLint viewport[4];
     glGetIntegerv(GL_VIEWPORT, viewport);
 
     // Update the viewport and render
     glViewport(x, y, width, height);
     shader->use();
     shader->setUniform("uTexture", 4);
     shader->setUniform("textureSize", glm::vec2(this->width, this->height));
     GLint blendSrc, blendDst;
     glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrc);
     glGetIntegerv(GL_BLEND_DST_RGB, &blendDst);
     glBlendFunc(GL_ONE, GL_ZERO);
     vao->render();
     glBlendFunc(blendSrc, blendDst);
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

void Frame::setFilterLinear() {
    unsigned int texture = fbo->getTextureID();
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Frame::setFilterNearest() {
    unsigned int texture = fbo->getTextureID();
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

std::shared_ptr<Image> Frame::getImage() {
    const size_t rowBytes = width * 4;
    const size_t dataSize = rowBytes * height;
    unsigned char* buffer = (unsigned char*)std::malloc(dataSize);
    if (!buffer)
        return nullptr;

    GLint prevFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
    fbo->bind();

    // glReadPixels has origin at bottom-left; read row-by-row so buffer ends up top-left origin
    for (int y = 0; y < (int)height; ++y) {
        glReadPixels(0, y, width, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer + (height - 1 - y) * rowBytes);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prevFbo);

    Image* img = new Image(buffer, (int)width, (int)height, 4);
    return std::shared_ptr<Image>(img);
}

Texture* Frame::getTexture() {
    const size_t rowBytes = width * 4;
    const size_t dataSize = rowBytes * height;
    std::vector<unsigned char> buffer(dataSize);

    GLint prevFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
    fbo->bind();

    for (int y = 0; y < (int)height; ++y) {
        glReadPixels(0, y, width, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data() + (height - 1 - y) * rowBytes);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prevFbo);

    return new Texture(buffer.data(), width, height, GL_RGBA, GL_UNSIGNED_BYTE);
}

}