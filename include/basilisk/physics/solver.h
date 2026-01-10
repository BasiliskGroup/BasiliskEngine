/*
* Copyright (c) 2025 Chris Giles
*
* Permission to use, copy, modify, distribute and sell this software
* and its documentation for any purpose is hereby granted without fee,
* provided that the above copyright notice appear in all copies.
* Chris Giles makes no representations about the suitability
* of this software for any purpose.
* It is provided "as is" without express or implied warranty.
*/

#pragma once

#include "basilisk/physics/threading/scratch.h"
#include <basilisk/util/includes.h>
#include <optional>
#include <basilisk/physics/coloring/color_queue.h>
#include <thread>
#include <barrier>
#include <semaphore>
#include <atomic>


#define PENALTY_MIN 1.0f              // Minimum penalty parameter
#define PENALTY_MAX 1000000000.0f     // Maximum penalty parameter
#define COLLISION_MARGIN 0.0005f      // Margin for collision detection to avoid flickering contacts
#define STICK_THRESH 0.01f            // Position threshold for sticking contacts (ie static friction)

namespace bsk::internal {

// Forward declarations
class Rigid;
class Force;
class ColliderTable;
class BodyTable;
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

    ColliderTable* colliderTable;
    BodyTable* bodyTable;

    // Coloring
    ColorQueue colorQueue;
    std::vector<std::vector<Rigid*>> colorGroups;

    // Threading
    std::barrier<> stageBarrier;
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
    ColliderTable* getColliderTable() const { return colliderTable; }
    BodyTable* getBodyTable() const { return bodyTable; }
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

    // Coloring
    void resetColoring();
    void dsatur();

    // Threading
    void workerLoop(unsigned int threadID);

    // Stages
    void primalStage(ThreadScratch& scratch, int threadID, int activeColor);
    void primalUpdateSingle(PrimalScratch& scratch, Rigid* body);
    void dualStage(ThreadScratch& scratch, int threadID);
    void dualUpdateSingle(Force* force);

    // Picking
    Rigid* pick(glm::vec2 at, glm::vec2& local);
};

}
