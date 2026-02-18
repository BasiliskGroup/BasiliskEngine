/**
 * Test scene for joints, springs, and motors.
 * Manifolds (contacts) appear when bodies collide.
 */
#include <basilisk/basilisk.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/forces/manifold.h>
#include <basilisk/physics/forces/joint.h>
#include <basilisk/physics/forces/spring.h>
#include <basilisk/physics/forces/motor.h>
#include <basilisk/physics/rigid.h>

int main() {
    bsk::Engine* engine = new bsk::Engine(800, 800, "Basilisk – Joints, Springs, Motors");
    bsk::Scene2D* scene = new bsk::Scene2D(engine);

    // Assets
    bsk::Mesh* quad = new bsk::Mesh("models/quad.obj");
    bsk::Image* metalImg = new bsk::Image("textures/metal.png");
    bsk::Image* ropeImg = new bsk::Image("textures/rope.png");
    bsk::Image* bricksImg = new bsk::Image("textures/bricks.jpg");
    bsk::Material* metalMaterial = new bsk::Material({1, 1, 1}, metalImg);
    bsk::Material* ropeMaterial = new bsk::Material({1, 1, 1}, ropeImg);
    bsk::Material* bricksMaterial = new bsk::Material({1, 1, 1}, bricksImg);

    bsk::Collider* boxCollider = new bsk::Collider({{0.5f, 0.5f}, {-0.5f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}});

    scene->getCamera()->setScale(12.0f);
    if (auto* camera2D = dynamic_cast<bsk::Camera2D*>(scene->getCamera())) {
        camera2D->setSpeed(12.0f);
    }

    const float boxSize = 0.6f;
    const float density = 8.0f;
    const float friction = 0.4f;

    // -------------------------------------------------------------------------
    // 1. Ground (static) – long bar at bottom so things can land and collide
    // -------------------------------------------------------------------------
    bsk::Node2D* ground = new bsk::Node2D(
        quad, metalMaterial,
        glm::vec2(0.0f, -5.0f), 0.0f,
        glm::vec2(20.0f, 0.8f),
        glm::vec3(0.0f), boxCollider, 1.0f, 0.6f
    );
    scene->add(ground);
    ground->getRigid()->setMass(0.0f);  // static

    // -------------------------------------------------------------------------
    // 2. Pendulum – world joint; when it swings it can hit ground (manifold)
    // -------------------------------------------------------------------------
    glm::vec2 pendulumAnchor(0.0f, 4.0f);
    bsk::Node2D* pendulum = new bsk::Node2D(
        quad, bricksMaterial,
        glm::vec2(0.0f, 2.5f), 0.0f,
        glm::vec2(boxSize, boxSize * 1.2f),
        glm::vec3(0.0f), boxCollider, density, friction
    );
    scene->add(pendulum);
    // rB = top of box in local coords (0, 0.5) for half-height 0.6 -> (0, 0.6*0.5) = (0, 0.3) with scale (boxSize, boxSize*1.2) -> local (0, 0.5)
    glm::vec2 pendulumLocalTop(0.0f, 0.5f);
    bsk::Joint* pendulumJoint = new bsk::Joint(
        scene->getSolver(),
        nullptr,
        pendulum->getRigid(),
        pendulumAnchor,
        pendulumLocalTop,
        glm::vec3(1000.0f, 1000.0f, 500.0f),
        INFINITY
    );

    // -------------------------------------------------------------------------
    // 3. Two boxes connected by a spring – can collide with ground and each other
    // -------------------------------------------------------------------------
    bsk::Node2D* springA = new bsk::Node2D(
        quad, ropeMaterial,
        glm::vec2(-3.0f, 1.0f), 0.0f,
        glm::vec2(boxSize, boxSize),
        glm::vec3(0.0f), boxCollider, density, friction
    );
    bsk::Node2D* springB = new bsk::Node2D(
        quad, ropeMaterial,
        glm::vec2(-1.5f, 1.0f), 0.0f,
        glm::vec2(boxSize, boxSize),
        glm::vec3(0.0f), boxCollider, density, friction
    );
    scene->add(springA);
    scene->add(springB);
    // Attach at right of A and left of B; rest length ~1.5
    bsk::Spring* spring = new bsk::Spring(
        scene->getSolver(),
        springA->getRigid(),
        springB->getRigid(),
        glm::vec2(0.5f, 0.0f),
        glm::vec2(-0.5f, 0.0f),
        80.0f,
        1.5f
    );

    // -------------------------------------------------------------------------
    // 4. Motor – two boxes with relative angular speed (e.g. “gears” or spinner)
    // -------------------------------------------------------------------------
    bsk::Node2D* motorA = new bsk::Node2D(
        quad, metalMaterial,
        glm::vec2(3.0f, 0.5f), 0.0f,
        glm::vec2(boxSize, boxSize),
        glm::vec3(0.0f), boxCollider, density, friction
    );
    bsk::Node2D* motorB = new bsk::Node2D(
        quad, metalMaterial,
        glm::vec2(3.0f, -0.5f), 0.0f,
        glm::vec2(boxSize, boxSize),
        glm::vec3(0.0f), boxCollider, density, friction
    );
    scene->add(motorA);
    scene->add(motorB);
    bsk::Motor* motor = new bsk::Motor(
        scene->getSolver(),
        motorA->getRigid(),
        motorB->getRigid(),
        2.0f,
        15.0f
    );

    // -------------------------------------------------------------------------
    // 5. Loose boxes – fall and create manifolds on ground and with others
    // -------------------------------------------------------------------------
    bsk::Node2D* loose1 = new bsk::Node2D(
        quad, bricksMaterial,
        glm::vec2(2.0f, 3.0f), 0.0f,
        glm::vec2(boxSize, boxSize),
        glm::vec3(0.0f), boxCollider, density, friction
    );
    bsk::Node2D* loose2 = new bsk::Node2D(
        quad, bricksMaterial,
        glm::vec2(-2.0f, 4.0f), 0.3f,
        glm::vec2(boxSize * 1.2f, boxSize),
        glm::vec3(0.0f), boxCollider, density, friction
    );
    scene->add(loose1);
    scene->add(loose2);

    // -------------------------------------------------------------------------
    // Main loop
    // -------------------------------------------------------------------------
    while (engine->isRunning()) {
        engine->update();
        scene->update();
        scene->render();
        engine->render();
    }

    // Cleanup
    delete motor;
    delete spring;
    delete pendulumJoint;
    delete metalMaterial;
    delete ropeMaterial;
    delete bricksMaterial;
    delete metalImg;
    delete ropeImg;
    delete bricksImg;
    delete quad;
    delete boxCollider;
    delete scene;
    delete engine;
    return 0;
}
