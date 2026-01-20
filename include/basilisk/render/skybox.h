#ifndef BSK_SKYBOX_H
#define BSK_SKYBOX_H

#include <basilisk/util/includes.h>
#include <basilisk/util/resolvePath.h>
#include <basilisk/camera/staticCamera.h>
#include <basilisk/render/image.h>
#include <basilisk/render/cubemap.h>
#include <basilisk/render/shader.h>
#include <basilisk/render/vbo.h>
#include <basilisk/render/vao.h>

namespace bsk::internal {

class Skybox {
    private:        
        Cubemap* cubemap;
        Shader* shader;
        VBO* vbo;
        VAO* vao;

        bool ownsCubemap;

    public:
        Skybox(Cubemap* cubemap, bool ownsCubemap=false);
        Skybox(const std::vector<Image*>& faces): Skybox(new Cubemap(faces), true) {}
        Skybox(const std::vector<std::string>& faces): Skybox(new Cubemap(faces), true) {}
        ~Skybox();

        void render(StaticCamera* camera);

        inline Cubemap* getCubemap() { return cubemap; }
        inline Shader* getShader() { return shader; }
        inline VBO* getVBO() { return vbo; }
        inline VAO* getVAO() { return vao; }
};

}
#endif