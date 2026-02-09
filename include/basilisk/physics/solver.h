#ifndef BSK_PHYSICS_SOLVER_H
#define BSK_PHYSICS_SOLVER_H

#include "basilisk/physics/threading/scratch.h"
#include <basilisk/util/includes.h>
#include <basilisk/util/constants.h>
#include <optional>
#include <basilisk/physics/coloring/color_queue.h>
#include <basilisk/physics/tables/adjacency.h>
#include <basilisk/physics/tables/forceTable.h>
#include <basilisk/physics/tables/forceTypeTable.h>
#include <basilisk/physics/forces/force.h>
#include <basilisk/physics/forces/joint.h>
#include <basilisk/physics/forces/manifold.h>
#include <basilisk/physics/forces/motor.h>
#include <basilisk/physics/forces/spring.h>
#include <thread>
#include <barrier>
#include <semaphore>
#include <atomic>

namespace bsk::internal {

// Forward declarations
class Rigid;
class Force;
class ColliderTable;
class BodyTable;
class ForceTable;
template<typename T> class ForceTypeTable;
struct ThreadScratch;
struct WorkRange;

// Core solver class which holds all the rigid bodies and forces, and has logic to step the simulation forward in time
class Solver {
private:
    enum class Stage {
        STAGE_NONE,
        STAGE_DUAL,
        STAGE_PRIMAL,
        STAGE_EXIT
    };

    std::optional<glm::vec3> gravity;  // Gravity
    int iterations;     // Solver iterations
    float dt;           // Timestep

    float alpha;        // Stabilization parameter
    float beta;         // Penalty ramping parameter
    float gamma;        // Warmstarting decay parameter

    bool postStabilize; // Whether to apply post-stabilization to the system

    Rigid* bodies;
    Force* forces;

    int numRigids;
    int numForces;

    BodyTable* bodyTable;
    ForceTable* forceTable;
    static std::unique_ptr<ColliderTable> colliderTable;

    // Coloring
    ColorQueue colorQueue;
    std::vector<std::vector<ColoredData>> colorGroups;
    std::vector<std::vector<ForceEdgeIndices>> forceEdgeIndices;

    // Threading
    std::barrier<> stageBarrier;
    std::barrier<> dualPassBarrier;
    std::counting_semaphore<> startSignal;
    std::counting_semaphore<> finishSignal;
    std::atomic<Stage> currentStage;
    std::atomic<float> currentAlpha;
    std::atomic<int> currentColor;
    std::atomic<bool> running;
    std::vector<std::thread> workers;

public:
    Solver();
    ~Solver();

    // Linked list management
    void insert(Rigid* body);
    void remove(Rigid* body);
    void insert(Force* force);
    void remove(Force* force);

    void clear();
    void defaultParams();
    void step(float dt);

    // Getters
    static ColliderTable* getColliderTable() { return colliderTable.get(); }
    BodyTable* getBodyTable() const { return bodyTable; }
    ForceTable* getForceTable() const { return forceTable; }
    Rigid* getBodies() const { return bodies; }
    Force* getForces() const { return forces; }
    int getNumRigids() const { return numRigids; }
    int getNumForces() const { return numForces; }
    std::optional<glm::vec3> getGravity() const { return gravity; }
    int getIterations() const { return iterations; }
    float getDt() const { return dt; }
    float getAlpha() const { return alpha; }
    float getBeta() const { return beta; }
    float getGamma() const { return gamma; }
    bool getPostStabilize() const { return postStabilize; }
    
    // Setters
    void setGravity(std::optional<glm::vec3> value) { gravity = value; }
    void setIterations(int value) { iterations = value; }
    void setDt(float value) { dt = value; }
    void setAlpha(float value) { alpha = value; }
    void setBeta(float value) { beta = value; }
    void setGamma(float value) { gamma = value; }
    void setPostStabilize(bool value) { postStabilize = value; }
    void setBodies(Rigid* value) { bodies = value; }
    void setForces(Force* value) { forces = value; }
    void setForceTable(ForceTable* value) { forceTable = value; }

    // Coloring
    void resetColoring();
    void dsatur();

    // Threading
    void workerLoop(unsigned int threadID);

    // Stages
    void primalStage(ThreadScratch& scratch, int threadID, int activeColor);
    void primalUpdateSingle(PrimalScratch& scratch, int activeColor, std::size_t bodyColorIndex);
    void dualStage(ThreadScratch& scratch, int threadID);

    template<class TForce, typename T>
    void dualUpdatePass(ForceTypeTable<T>* table, int threadID) {
        std::size_t tableSize = table->getSize();
        if (tableSize == 0) return;

        WorkRange range = partition(tableSize, threadID, NUM_THREADS);
        float alpha = currentAlpha.load(std::memory_order_acquire);

        for (std::size_t i = range.start; i < range.end; i++) {
            std::size_t forceIndex = table->getForceIndex(i);
            Force* force = forceTable->getForce(forceIndex);
            if (!force) continue;

            TForce::computeConstraint(forceTable, forceIndex, alpha);

            for (int r = 0; r < force->rows(); r++) {
                ParameterStruct& parameters = forceTable->getParameter(forceIndex, r);

                float stiffness = parameters.stiffness;
                float lambda = glm::isinf(stiffness) ? parameters.lambda : 0.0f;

                float penalty = parameters.penalty;
                float C = parameters.C;
                float fmin = parameters.fmin;
                float fmax = parameters.fmax;
                float newLambda = glm::clamp(penalty * C + lambda, fmin, fmax);
                parameters.lambda = newLambda;

                float fracture = force->getFracture(r);
                if (fabsf(newLambda) >= fracture)
                    force->disable();

                if (newLambda > fmin && newLambda < fmax) {
                    parameters.penalty = glm::min(penalty + beta * glm::abs(C), glm::min(PENALTY_MAX, stiffness));
                }
            }
        }
    }

    template<class TForce>
    inline void processForce(ForceTable* forceTable, std::size_t forceIndex, ForceBodyOffset body, PrimalScratch& scratch, float alpha);

    // Picking
    Rigid* pick(glm::vec2 at, glm::vec2& local);
};

}

#endif