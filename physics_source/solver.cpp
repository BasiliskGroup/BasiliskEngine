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

#include "solver.h"
#include <iostream>

Solver::Solver()
    : bodies(0), forces(0)
{
    defaultParams();
}

Solver::~Solver()
{
    clear();
}

Rigid* Solver::pick(float2 at, float2& local)
{
    // Find which body is at the given point
    for (Rigid* body = bodies; body != 0; body = body->next)
    {
        float2x2 Rt = rotation(-body->position.z);
        local = Rt * (at - body->position.xy());
        if (local.x >= -body->size.x * 0.5f && local.x <= body->size.x * 0.5f &&
            local.y >= -body->size.y * 0.5f && local.y <= body->size.y * 0.5f)
            return body;
    }
    return 0;
}

void Solver::clear()
{
    while (forces)
        delete forces;

    while (bodies)
        delete bodies;
}

void Solver::defaultParams()
{
    dt = 1.0f / 60.0f;
    gravity = -10.0f;
    iterations = 10;

    // Note: in the paper, beta is suggested to be [1, 1000]. Technically, the best choice will
    // depend on the length, mass, and constraint function scales (ie units) of your simulation,
    // along with your strategy for incrementing the penalty parameters.
    // If the value is not in the right range, you may see slower convergance for complex scenes.
    beta = 100000.0f;

    // Alpha controls how much stabilization is applied. Higher values give slower and smoother
    // error correction, and lower values are more responsive and energetic. Tune this depending
    // on your desired constraint error response.
    alpha = 0.99f;

    // Gamma controls how much the penalty and lambda values are decayed each step during warmstarting.
    // This should always be < 1 so that the penalty values can decrease (unless you use a different
    // penalty parameter strategy which does not require decay).
    gamma = 0.99f;

    // Post stabilization applies an extra iteration to fix positional error.
    // This removes the need for the alpha parameter, which can make tuning a little easier.
    postStabilize = true;
}

void Solver::step()
{
    std::cout << "\n========== STEP START ==========\n";
    std::cout << "dt: " << dt << "\n";
    std::cout << "gravity: " << gravity << "\n";
    std::cout << "iterations: " << iterations << "\n";
    std::cout << "alpha: " << alpha << ", beta: " << beta << ", gamma: " << gamma << "\n";
    std::cout << "postStabilize: " << (postStabilize ? "true" : "false") << "\n";
    
    // Count bodies and forces
    int bodyCount = 0;
    for (Rigid* body = bodies; body != 0; body = body->next) bodyCount++;
    int forceCount = 0;
    for (Force* force = forces; force != 0; force = force->next) forceCount++;
    std::cout << "Bodies: " << bodyCount << ", Forces: " << forceCount << "\n";
    
    // Perform broadphase collision detection
    std::cout << "\n=== BROADPHASE COLLISION ===\n";
    int newManifolds = 0;
    for (Rigid* bodyA = bodies; bodyA != 0; bodyA = bodyA->next)
    {
        for (Rigid* bodyB = bodyA->next; bodyB != 0; bodyB = bodyB->next)
        {
            float2 dp = bodyA->position.xy() - bodyB->position.xy();
            float r = bodyA->radius + bodyB->radius;
            if (dot(dp, dp) <= r * r && !bodyA->constrainedTo(bodyB)) {
                std::cout << "Creating manifold between body " << bodyA << " and " << bodyB << "\n";
                new Manifold(this, bodyA, bodyB);
                newManifolds++;
            }
        }
    }
    std::cout << "Created " << newManifolds << " new manifolds\n";

    // Initialize and warmstart forces
    std::cout << "\n=== WARMSTART FORCES ===\n";
    int deletedForces = 0;
    for (Force* force = forces; force != 0;)
    {
        if (!force->initialize())
        {
            std::cout << "Force " << force << " returned false, deleting\n";
            Force* next = force->next;
            delete force;
            force = next;
            deletedForces++;
        }
        else
        {
            std::cout << "Force " << force << " rows=" << force->rows() << ":\n";
            for (int i = 0; i < force->rows(); i++)
            {
                float oldLambda = force->lambda[i];
                float oldPenalty = force->penalty[i];
                
                if (postStabilize)
                {
                    force->penalty[i] = clamp(force->penalty[i] * gamma, PENALTY_MIN, PENALTY_MAX);
                }
                else
                {
                    force->lambda[i] = force->lambda[i] * alpha * gamma;
                    force->penalty[i] = clamp(force->penalty[i] * gamma, PENALTY_MIN, PENALTY_MAX);
                }

                force->penalty[i] = min(force->penalty[i], force->stiffness[i]);
                
                std::cout << "  Row " << i << ": lambda " << oldLambda << " -> " << force->lambda[i]
                         << ", penalty " << oldPenalty << " -> " << force->penalty[i]
                         << ", stiffness=" << force->stiffness[i] << "\n";
            }

            force = force->next;
        }
    }
    std::cout << "Deleted " << deletedForces << " inactive forces\n";

    // Initialize and warmstart bodies
    std::cout << "\n=== WARMSTART BODIES ===\n";
    for (Rigid* body = bodies; body != 0; body = body->next)
    {
        std::cout << "Body " << body << ":\n";
        std::cout << "  pos: (" << body->position.x << ", " << body->position.y << ", " << body->position.z << ")\n";
        std::cout << "  vel: (" << body->velocity.x << ", " << body->velocity.y << ", " << body->velocity.z << ")\n";
        std::cout << "  mass: " << body->mass << ", moment: " << body->moment << "\n";
        
        body->velocity.z = clamp(body->velocity.z, -50.0f, 50.0f);

        body->inertial = body->position + body->velocity * dt;
        if (body->mass > 0)
            body->inertial += float3{ 0, gravity, 0 } * (dt * dt);

        float3 accel = (body->velocity - body->prevVelocity) / dt;
        float accelExt = accel.y * sign(gravity);
        float accelWeight = clamp(accelExt / abs(gravity), 0.0f, 1.0f);
        if (!isfinite(accelWeight)) accelWeight = 0.0f;

        body->initial = body->position;
        body->position = body->position + body->velocity * dt + float3{ 0, gravity, 0 } * (accelWeight * dt * dt);
        
        std::cout << "  inertial: (" << body->inertial.x << ", " << body->inertial.y << ", " << body->inertial.z << ")\n";
        std::cout << "  initial: (" << body->initial.x << ", " << body->initial.y << ", " << body->initial.z << ")\n";
        std::cout << "  warmstart pos: (" << body->position.x << ", " << body->position.y << ", " << body->position.z << ")\n";
        std::cout << "  accelWeight: " << accelWeight << "\n";
    }

    int totalIterations = iterations + (postStabilize ? 1 : 0);
    std::cout << "\nTotal iterations: " << totalIterations << "\n";

    for (int it = 0; it < totalIterations; it++)
    {
        float currentAlpha = alpha;
        if (postStabilize)
            currentAlpha = it < iterations ? 1.0f : 0.0f;

        std::cout << "\n--- ITERATION " << it << " (alpha=" << currentAlpha << ") ---\n";

        // Primal update
        std::cout << "=== PRIMAL UPDATE ===\n";
        for (Rigid* body = bodies; body != 0; body = body->next)
        {
            if (body->mass <= 0)
                continue;

            std::cout << "Body " << body << " primal:\n";
            std::cout << "  pos before: (" << body->position.x << ", " << body->position.y << ", " << body->position.z << ")\n";

            float3x3 M = diagonal(body->mass, body->mass, body->moment);
            float3x3 lhs = M / (dt * dt);
            float3 rhs = M / (dt * dt) * (body->position - body->inertial);

            std::cout << "  initial rhs: (" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")\n";

            int forceIdx = 0;
            for (Force* force = body->forces; force != 0; force = (force->bodyA == body) ? force->nextA : force->nextB)
            {
                force->computeConstraint(currentAlpha);
                force->computeDerivatives(body);

                std::cout << "  Force " << force << " (#" << forceIdx << ", rows=" << force->rows() << "):\n";

                for (int i = 0; i < force->rows(); i++)
                {
                    float lambda = isinf(force->stiffness[i]) ? force->lambda[i] : 0.0f;
                    float f = clamp(force->penalty[i] * force->C[i] + lambda, force->fmin[i], force->fmax[i]);

                    std::cout << "    Row " << i << ": C=" << force->C[i]
                             << ", penalty=" << force->penalty[i]
                             << ", lambda=" << lambda
                             << ", f=" << f << "\n";
                    std::cout << "    J: (" << force->J[i].x << ", " << force->J[i].y << ", " << force->J[i].z << ")\n";

                    float3x3 G = diagonal(length(force->H[i].col(0)), length(force->H[i].col(1)), length(force->H[i].col(2))) * abs(f);

                    rhs += force->J[i] * f;
                    lhs += outer(force->J[i], force->J[i] * force->penalty[i]) + G;
                }
                forceIdx++;
            }

            std::cout << "  final rhs: (" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")\n";
            float3 correction = solve(lhs, rhs);
            std::cout << "  correction: (" << correction.x << ", " << correction.y << ", " << correction.z << ")\n";
            
            body->position -= correction;
            std::cout << "  pos after: (" << body->position.x << ", " << body->position.y << ", " << body->position.z << ")\n";
        }

        // Dual update
        if (it < iterations)
        {
            std::cout << "\n=== DUAL UPDATE ===\n";
            for (Force* force = forces; force != 0; force = force->next)
            {
                force->computeConstraint(currentAlpha);

                std::cout << "Force " << force << ":\n";

                for (int i = 0; i < force->rows(); i++)
                {
                    float lambda = isinf(force->stiffness[i]) ? force->lambda[i] : 0.0f;
                    float oldLambda = force->lambda[i];
                    float oldPenalty = force->penalty[i];

                    force->lambda[i] = clamp(force->penalty[i] * force->C[i] + lambda, force->fmin[i], force->fmax[i]);

                    if (fabsf(force->lambda[i]) >= force->fracture[i])
                        force->disable();

                    if (force->lambda[i] > force->fmin[i] && force->lambda[i] < force->fmax[i])
                        force->penalty[i] = min(force->penalty[i] + beta * abs(force->C[i]), min(PENALTY_MAX, force->stiffness[i]));
                    
                    std::cout << "  Row " << i << ": C=" << force->C[i]
                             << ", lambda " << oldLambda << " -> " << force->lambda[i]
                             << ", penalty " << oldPenalty << " -> " << force->penalty[i] << "\n";
                }
            }
        }

        if (it == iterations - 1)
        {
            std::cout << "\n=== COMPUTE VELOCITIES (BDF1) ===\n";
            for (Rigid* body = bodies; body != 0; body = body->next)
            {
                body->prevVelocity = body->velocity;
                if (body->mass > 0) {
                    float3 oldVel = body->velocity;
                    body->velocity = (body->position - body->initial) / dt;
                    std::cout << "Body " << body << ": vel " 
                             << "(" << oldVel.x << ", " << oldVel.y << ", " << oldVel.z << ") -> "
                             << "(" << body->velocity.x << ", " << body->velocity.y << ", " << body->velocity.z << ")\n";
                }
            }
        }
    }
    
    std::cout << "========== STEP END ==========\n\n";

    
}

void Solver::draw()
{
    for (Rigid* body = bodies; body != 0; body = body->next)
        body->draw();
    for (Force* force = forces; force != 0; force = force->next)
        force->draw();
}
