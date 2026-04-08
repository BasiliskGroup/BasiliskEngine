#include <basilisk/basilisk.h>

#include <cstdio>
#include <deque>
#include <optional>
#include <random>
#include <unordered_set>

namespace {

constexpr float kArenaHalf = 7.5f;
constexpr float kWallThickness = 0.55f;
constexpr float kPlayerSpeed = 11.0f;
constexpr float kCollectRadius = 0.55f;
constexpr float kSpawnEnemyEverySec = 2.25f;
constexpr int kMaxEnemies = 14;
constexpr unsigned kWindowW = 960;
constexpr unsigned kWindowH = 720;

glm::vec2 randomInArena(std::mt19937& rng) {
    std::uniform_real_distribution<float> dx(-kArenaHalf + 1.8f, kArenaHalf - 1.8f);
    std::uniform_real_distribution<float> dy(-kArenaHalf + 1.8f, kArenaHalf - 1.8f);
    return {dx(rng), dy(rng)};
}

void spawnEnemy(bsk::Scene2D* scene, bsk::Collider* boxCol, bsk::Material* mat, std::mt19937& rng,
                std::deque<bsk::Node2D*>& enemies, std::unordered_set<bsk::Node2D*>& enemySet) {
    if (static_cast<int>(enemies.size()) >= kMaxEnemies) {
        return;
    }
    glm::vec2 p = randomInArena(rng);
    std::uniform_real_distribution<float> ang(0.0f, 6.28318530718f);
    std::uniform_real_distribution<float> spd(3.5f, 7.5f);
    const float a = ang(rng);
    const float s = spd(rng);
    auto* e = new bsk::Node2D(scene, nullptr, mat, p, a * 0.3f, glm::vec2(0.38f, 0.38f),
        glm::vec3(std::cos(a) * s, std::sin(a) * s, 0.0f), boxCol, 1.0f, 0.25f);
    enemies.push_back(e);
    enemySet.insert(e);
}

bool isHunter(bsk::Node2D* n, const std::unordered_set<bsk::Node2D*>& enemySet) {
    return n && enemySet.count(n) != 0;
}

void resetRound(bsk::Engine* engine, bsk::Node2D* player, bsk::Node2D* coin, std::mt19937& rng,
                std::deque<bsk::Node2D*>& enemies, std::unordered_set<bsk::Node2D*>& enemySet,
                int& score, int highScore, bool& alive) {
    for (bsk::Node2D* e : enemies) {
        delete e;
    }
    enemies.clear();
    enemySet.clear();

    player->setPosition({0.0f, 0.0f});
    player->setVelocity({0.0f, 0.0f, 0.0f});
    coin->setPosition(randomInArena(rng));
    score = 0;
    alive = true;
    char buf[160];
    std::snprintf(buf, sizeof(buf), "Basilisk — Neon Vault   score %d   high %d   WASD  R restart", score,
        highScore);
    engine->getWindow()->setTitle(buf);
}

void buildArena(bsk::Scene2D* scene, bsk::Collider* boxCol, bsk::Material* wallMat, float zLayer) {
    const float t = kWallThickness;
    const float span = 2.0f * kArenaHalf + 2.0f * t;

    auto wall = [&](glm::vec2 pos, glm::vec2 scale) {
        auto* w = new bsk::Node2D(scene, nullptr, wallMat, pos, 0.0f, scale, glm::vec3(0.0f), boxCol, -1.0f);
        w->setLayer(zLayer);
    };

    wall({0.0f, -kArenaHalf - t * 0.5f}, {span, t});
    wall({0.0f, kArenaHalf + t * 0.5f}, {span, t});
    wall({-kArenaHalf - t * 0.5f, 0.0f}, {t, span});
    wall({kArenaHalf + t * 0.5f, 0.0f}, {t, span});
}

} // namespace

int main() {
    auto* engine = new bsk::Engine(static_cast<int>(kWindowW), static_cast<int>(kWindowH),
        "Basilisk — Neon Vault", false, false);
    auto* scene = new bsk::Scene2D(engine);

    scene->getSolver()->setGravity(std::make_optional(glm::vec3(0.0f)));

    auto* camera = new bsk::StaticCamera2D(engine, glm::vec2(0.0f), 18.0f);
    scene->setCamera(camera);

    auto* boxCol = new bsk::Collider({
        glm::vec2(0.5f, 0.5f),
        glm::vec2(-0.5f, 0.5f),
        glm::vec2(-0.5f, -0.5f),
        glm::vec2(0.5f, -0.5f),
    });

    auto* wallMat = new bsk::Material(glm::vec3(0.12f, 0.14f, 0.22f));
    auto* playerMat = new bsk::Material(glm::vec3(0.15f, 0.85f, 0.72f));
    auto* hunterMat = new bsk::Material(glm::vec3(0.92f, 0.22f, 0.35f));
    auto* coinMat = new bsk::Material(glm::vec3(0.98f, 0.78f, 0.20f));

    buildArena(scene, boxCol, wallMat, -0.1f);

    auto* player = new bsk::Node2D(scene, nullptr, playerMat, glm::vec2(0.0f), 0.0f, glm::vec2(0.42f, 0.42f),
        glm::vec3(0.0f), boxCol, 1.0f, 0.4f);
    player->setLayer(0.2f);

    std::mt19937 rng(std::random_device{}());
    auto* coin = new bsk::Node2D(scene, nullptr, coinMat, randomInArena(rng), 0.0f, glm::vec2(0.28f, 0.28f));
    coin->setLayer(0.15f);
    std::deque<bsk::Node2D*> enemies;
    std::unordered_set<bsk::Node2D*> enemySet;

    int score = 0;
    int highScore = 0;
    bool alive = true;
    float enemyTimer = 0.0f;

    while (engine->isRunning()) {
        engine->update();
        auto* kb = engine->getKeyboard();
        auto* win = engine->getWindow();

        if (kb->getPressed(bsk::Key::K_ESCAPE)) {
            glfwSetWindowShouldClose(win->getWindow(), GLFW_TRUE);
        }

        if (kb->getPressed(bsk::Key::K_R)) {
            resetRound(engine, player, coin, rng, enemies, enemySet, score, highScore, alive);
        }

        const float dt = static_cast<float>(engine->getDeltaTime());

        if (alive) {
            glm::vec2 dir(0.0f);
            if (kb->getDown(bsk::Key::K_A)) dir.x -= 1.0f;
            if (kb->getDown(bsk::Key::K_D)) dir.x += 1.0f;
            if (kb->getDown(bsk::Key::K_W)) dir.y += 1.0f;
            if (kb->getDown(bsk::Key::K_S)) dir.y -= 1.0f;
            if (glm::dot(dir, dir) > 1e-6f) {
                dir = glm::normalize(dir);
            }
            player->setVelocity(glm::vec3(dir.x * kPlayerSpeed, dir.y * kPlayerSpeed, 0.0f));

            enemyTimer += dt;
            while (enemyTimer >= kSpawnEnemyEverySec) {
                enemyTimer -= kSpawnEnemyEverySec;
                spawnEnemy(scene, boxCol, hunterMat, rng, enemies, enemySet);
            }

            const glm::vec2 pp = player->getPositionRef();
            const glm::vec2 cp = coin->getPositionRef();
            if (glm::distance(pp, cp) < kCollectRadius) {
                ++score;
                if (score > highScore) {
                    highScore = score;
                }
                coin->setPosition(randomInArena(rng));
                char buf[160];
                std::snprintf(buf, sizeof(buf), "Basilisk — Neon Vault   score %d   high %d", score, highScore);
                win->setTitle(buf);
            }

            for (const auto& hit : player->getCollisions()) {
                if (isHunter(hit.other, enemySet)) {
                    alive = false;
                    player->setVelocity({0.0f, 0.0f, 0.0f});
                    char buf[192];
                    std::snprintf(buf, sizeof(buf),
                        "Basilisk — caught! score %d (high %d) — R restart, Esc quit", score, highScore);
                    win->setTitle(buf);
                    break;
                }
            }
        }

        scene->update();
        scene->render();
        engine->render();
    }

    delete scene;
    delete camera;
    delete wallMat;
    delete playerMat;
    delete hunterMat;
    delete coinMat;
    delete boxCol;
    delete engine;
    return 0;
}
