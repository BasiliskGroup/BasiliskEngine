#ifndef BSK_FRAME_H
#define BSK_FRAME_H

#include <memory>
#include <basilisk/util/includes.h>
#include <basilisk/render/shader.h>
#include <basilisk/render/vbo.h>
#include <basilisk/render/ebo.h>
#include <basilisk/render/vao.h>
#include <basilisk/render/fbo.h>
#include <basilisk/render/image.h>


namespace bsk::internal {

class Engine;

class Frame {
    private:
        Engine* engine;

        Shader* shader;
        VBO* vbo;
        EBO* ebo;
        VAO* vao;
        FBO* fbo;

        unsigned int width;
        unsigned int height;
        float aspectRatio;
        bool customShader = true;

    public:
        Frame(Engine* engine, unsigned int width = 800, unsigned int height = 800);
        Frame(Engine* engine, Shader* shader, unsigned int width = 800, unsigned int height = 800);
        ~Frame();

        void use();
        void clear(float r=0.0, float g=0.0, float b=0.0, float a=1.0);
        void render();
        void render(int x, int y, int width, int height);
        
        inline Shader* getShader() { return shader; }
        inline VBO* getVBO() { return vbo; }
        inline EBO* getEBO() { return ebo; }
        inline VAO* getVAO() { return vao; }
        inline FBO* getFBO() { return fbo; }
        inline unsigned int getWidth() { return width; }
        inline unsigned int getHeight() { return height; }
        inline float getAspectRatio() { return aspectRatio; }
        unsigned int getRenderWidth();
        unsigned int getRenderHeight();

        void setFilterLinear();
        void setFilterNearest();

        /** Read the current FBO color attachment to CPU and return it as an Image (RGBA, top-left origin). */
        std::shared_ptr<Image> getImage();
};

}

#endif