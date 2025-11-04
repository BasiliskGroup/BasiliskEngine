#ifndef SOLVER_H
#define SOLVER_H

#include <basilisk/util/includes.h>
#include <basilisk/util/maths.h>
#include <basilisk/util/time.h>
#include <basilisk/util/print.h>

#include <basilisk/tables/bodyTable.h>
#include <basilisk/tables/forceRoute.h>
#include <basilisk/tables/colliderFlat.h>

namespace bsk::internal {

class Rigid;
class Force;
class Manifold;
class Collider;

class Solver {
private:
    struct CollisionIndexPair {
        uint bodyA;
        uint bodyB;
        Force* manifold = nullptr;

        CollisionIndexPair(uint bodyA, uint bodyB) : bodyA(bodyA), bodyB(bodyB) {}
        CollisionIndexPair(uint bodyA, uint bodyB, Force* manifold) : bodyA(bodyA), bodyB(bodyB), manifold(nullptr) {} // TODO change this to accepting manifold when we preserve them
    };

    float gravity;      // Gravity
    int iterations;     // Solver iterations

    float alpha;        // Stabilization parameter
    float beta;         // Penalty ramping parameter
    float gamma;        // Warmstarting decay parameter

    // linked lists heads
    Rigid* bodies;
    Force* forces;

    // Tables
    ForceTable* forceTable;
    BodyTable* bodyTable;
    ColliderFlat* colliderFlat;

    // broad collision detection
    std::vector<CollisionIndexPair> collisionPairs;

    // collision struct for hotloop caching
    struct ColliderRow {
        glm::vec2 pos;
        glm::vec2 scale;
        glm::mat2x2 mat;
        glm::mat2x2 imat;
        glm::vec2* start;
        uint length;
        glm::vec2* simplex;
        std::array<float, 4> dots; // TODO this needs to be at least the length of the collider

        ColliderRow() = default;
    };

    struct PolytopeFace {
        glm::vec2 normal;
        float distance;

        // face vertices
        ushort va;
        ushort vb;

        PolytopeFace() = default;
        PolytopeFace(ushort va, ushort vb, glm::vec2 normal, float distance)
            : normal(normal), distance(distance), va(va), vb(vb) {}
    };

    using Simplex = std::array<glm::vec2, 3>;
    using SpSet = std::array<ushort, EPA_ITERATIONS + 3>;
    using SpArray = std::array<glm::vec2, EPA_ITERATIONS + 3>;
    using Polytope = std::array<PolytopeFace, EPA_ITERATIONS + 3>;

    struct CollisionPair {
        uint forceIndex;
        uint manifoldIndex;

        // gjk
        Simplex minks;
        glm::vec2 dir;

        // epa
        // we add 3 since we start with 3 faces
        SpArray sps;
        SpSet spSet;
        Polytope polytope;

        CollisionPair() = default;
    };
    
public:
    Solver();
    ~Solver();

    // getters
    Force*& getForces() { return forces; }
    Rigid*& getBodies() { return bodies; }
    ForceTable* getForceTable() { return forceTable; }
    BodyTable*  getBodyTable()  { return bodyTable; }
    ColliderFlat*  getColliderFlat()  { return colliderFlat; }
    ManifoldTable* getManifoldTable() { return forceTable->getManifoldTable(); } 

    void step(float dt);
    void draw();

    // setters
    void setGravity(float gravity);

    // linked list operations
    void insert(Rigid* rigid);
    void insert(Force* force);
    void remove(Rigid* rigid);
    void remove(Force* force);
    void clear();

private:
    // manage storage functions
    void compactBodies();
    void compactForces();
    void reserveForcesForCollision(uint& forceIndex, uint& manifoldIndex);

    // compute stages
    void warmstartManifolds();
    void warmstartForces();
    void warmstartBodies(float dt);
    void updateVelocities(float dt);
    void mainloopPreload();
    void primalUpdate(float dt);
    void dualUpdate(float dt);

    void computeConstraints(uint start, uint end, ushort type);
    void computeDerivatives(uint start, uint end, ushort type);
    void loadCdX(uint start, uint end);

    // collision functions
    void sphericalCollision();
    void narrowCollision();
    
    bool gjk(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint freeIndex);
    ushort epa(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    void sat(ColliderRow& a, ColliderRow& b, CollisionPair& pair);

    void initColliderRow(uint row, uint manifoldIndex, ColliderRow& colliderRow);

    // gjk methods helper functions
    uint handleSimplex(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint freeIndex);
    uint handle0(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    uint handle1(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    uint handle2(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    uint handle3(ColliderRow& a, ColliderRow& b, CollisionPair& pair);
    void addSupport(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint insertIndex);
    void getFar(const ColliderRow& row, const glm::vec2& dir, glm::vec2& simplexLocal);

    // epa helper methods
    ushort insertHorizon(SpSet& spSet, ushort spIndex, ushort setSize);
    bool discardHorizon(SpSet& spSet, ushort spIndex, ushort setSize);
    ushort polytopeFront(const Polytope& polytope, ushort numFaces);
    void removeFace(Polytope& polytope, ushort index, ushort numFaces);
    void supportSpOnly(ColliderRow& a, ColliderRow& b, CollisionPair& pair, uint insertIndex);
    void buildFace(CollisionPair& pair, ushort indexA, ushort indexB, ushort indexL);

    using Dots = std::array<float, 4>;

    // sat helper functions
    void intersect(ColliderRow& a, ColliderRow& b, CollisionPair& pair, const glm::vec2& mtv);
    void clampToEdge(const glm::vec2& edge, glm::vec2& toClamp);
    void dotEdgeIntersect(const glm::vec2* verts, uint start, Dots dots, float thresh);

    template <typename Compare> // std::greater_equal<int>()) // std::less_equal<int>()) // if (cmp(a, b))
    void findBounds(Dots dots, const float thresh, uint& begin, uint& end, Compare cmp);

    template <typename Compare>
    void findExtremes(Dots dots, uint& begin, uint& end, Compare cmp);
};

}

#endif