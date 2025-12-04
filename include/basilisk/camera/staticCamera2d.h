#ifndef BSK_STATIC_CAMERA_2D_H
#define BSK_STATIC_CAMERA_2D_H


#include <basilisk/util/includes.h>
#include <basilisk/camera/virtualCamera.h>
#include <basilisk/render/shader.h>

namespace bsk::internal {

class Engine;

class StaticCamera2D : public VirtualCamera{
    protected:
        Engine* engine;

        glm::vec2 position;
        glm::vec2 viewScale;

        void updateProjection();
        void updateView();

    public:
        StaticCamera2D(Engine* engine, glm::vec2 position = {0.0f, 0.0f}, float scale=10.0f);

        void update();
        void use(Shader* shader);

        void setPosition(glm::vec2 position) { this->position = position; }
        void setX(double x) { position.x = x; }
        void setY(double y) { position.y = y; }
        void setScale(float scale);
        void setScale(float xScale, float yScale) { viewScale = {xScale, yScale}; updateProjection(); }
        void setScale(glm::vec2 viewScale) { this->viewScale = viewScale; updateProjection(); }

        glm::vec2 getPosition() { return position; }
        double getX() { return position.x; }
        double getY() { return position.y; }

        glm::vec2 getViewScale() { return viewScale; }
        double getViewWidth() { return viewScale.x; }
        double getViewHeight() { return viewScale.y; }
};

};


#endif