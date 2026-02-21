/**
 * C++ port of test.py: slot machine effect with three reels.
 * Press SPACE to advance the slot animation.
 */
#include <basilisk/basilisk.h>

using namespace bsk;

class Slot {
public:
    Scene2D* scene;
    float width;
    float height;
    Material* top_material;
    Material* bottom_material;
    Material* middle_material;
    Node2D* top_node;
    Node2D* bottom_node;
    Node2D* middle_node;
    Node2D* top_image_node;
    Node2D* bottom_image_node;
    Node2D* middle_image_node;

    int index = 0;
    float offset = 1.0f;
    float speed = 0.0f;
    float max_speed = 10.0f;
    bool running = false;

    Slot(Scene2D* scene, Image* img0, Image* img1, Image* img2, float x, float w, float h)
        : scene(scene), width(w), height(h)
    {
        top_material    = new Material(glm::vec3(1, 0, 0), img0);
        bottom_material = new Material(glm::vec3(0, 1, 0), img1);
        middle_material = new Material(glm::vec3(0, 0, 1), img2);

        float gap = 0.05f;
        glm::vec2 slotSize(w - gap, h - gap);
        top_node    = new Node2D(scene, nullptr, nullptr, glm::vec2(x, offset * h - h), 0.0f, slotSize);
        bottom_node = new Node2D(scene, nullptr, nullptr, glm::vec2(x, offset * h), 0.0f, slotSize);
        middle_node = new Node2D(scene, nullptr, nullptr, glm::vec2(x, offset * h + h), 0.0f, slotSize);

        glm::vec2 scale = (w < h) ? glm::vec2(1.0f, w / h) : glm::vec2(h / w, 1.0f);
        top_image_node    = new Node2D(top_node,    nullptr, top_material,    glm::vec2(0, 0), 0.0f, scale);
        bottom_image_node = new Node2D(bottom_node, nullptr, bottom_material, glm::vec2(0, 0), 0.0f, scale);
        middle_image_node = new Node2D(middle_node, nullptr, middle_material, glm::vec2(0, 0), 0.0f, scale);

        top_image_node->setLayer(0.1f);
        bottom_image_node->setLayer(0.1f);
        middle_image_node->setLayer(0.1f);
    }

    void start() {
        running = true;
        speed = 0.0f;
    }

    void update(float dt) {
        if (running && speed < max_speed)
            speed += dt * 5.0f;
        if (running) {
            offset += dt * speed;
            if (offset > 1.0f) {
                index++;
                offset = 0.0f;
            }
        }

        top_node->getPositionRef().y    = -1.0f * offset * height - height;
        bottom_node->getPositionRef().y  = -1.0f * offset * height;
        middle_node->getPositionRef().y  = -1.0f * offset * height + height;
    }
};

class SlotEffect {
public:
    Engine* engine;
    Scene2D* scene;
    Scene* game_scene;
    Frame* frame;
    float time = 0.0f;
    Slot* left_slot;
    Slot* middle_slot;
    Slot* right_slot;

    SlotEffect(Engine* engine, Scene* game_scene, Node2D* outline_node)
        : engine(engine), game_scene(game_scene)
    {
        scene = new Scene2D(engine);
        glm::vec2 scale = outline_node->getScale();
        scene->getCamera()->setScale(scale);

        frame = new Frame(engine);

        float w = scale.x / 3.0f;
        float h = scale.y;
        Image* t1 = new Image("textures/man.png");
        Image* t2 = new Image("textures/container.jpg");
        Image* t3 = new Image("textures/bricks.jpg");

        left_slot   = new Slot(scene, t1, t2, t3, 0.0f - scale.x / 3.0f, w, h);
        middle_slot = new Slot(scene, t1, t2, t3, 0.0f, w, h);
        right_slot  = new Slot(scene, t1, t2, t3, 0.0f + scale.x / 3.0f, w, h);
    }

    void update() {
        float dt = (float)engine->getDeltaTime();
        time += dt;
        left_slot->update(dt);
        middle_slot->update(dt);
        right_slot->update(dt);

        if (time > 0.1f && !left_slot->running)   left_slot->start();
        if (time > 0.3f && !middle_slot->running) middle_slot->start();
        if (time > 0.5f && !right_slot->running)   right_slot->start();
    }
};

int main() {
    Engine* engine = new Engine();
    Scene* game_scene = new Scene(engine);
    Scene2D* scene = new Scene2D(engine);
    Scene2D* outline_scene = new Scene2D(engine);

    Node2D* outline_node = new Node2D(outline_scene, nullptr, nullptr, glm::vec2(0, 0), 0.0f, glm::vec2(4, 4));
    SlotEffect* slot_effect = new SlotEffect(engine, game_scene, outline_node);

    while (engine->isRunning()) {
        engine->update();
        scene->update();

        game_scene->update();
        game_scene->render();

        if (engine->getKeyboard()->getDown(bsk::internal::KeyCode::K_SPACE))
            std::cout << "SPACE" << std::endl;
            slot_effect->update();

        slot_effect->frame->use();
        slot_effect->frame->clear(0.2f, 0.2f, 0.2f, 1.0f);
        slot_effect->scene->render();

        engine->disableDepthTest();
        engine->getFrame()->use();
        slot_effect->frame->render(400, 400, 300, 300);
        engine->enableDepthTest();

        engine->render();
    }

    delete slot_effect;
    delete outline_node;
    delete outline_scene;
    delete scene;
    delete game_scene;
    delete engine;
    return 0;
}
