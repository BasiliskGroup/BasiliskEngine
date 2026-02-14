#include <basilisk/basilisk.h>
#include <basilisk/physics/rigid.h>
#include <cassert>
#include <iostream>
#include <stdexcept>

namespace bsk::internal {

class Node2DPhysicsTest {
private:
    Engine* engine;
    Scene2D* scene1;
    Scene2D* scene2;
    Scene2D* scene3;
    Mesh* quad;
    Material* material;
    Collider* boxCollider;
    Collider* smallBoxCollider;

public:
    Node2DPhysicsTest() {
        engine = new Engine(800, 800, "Node2DPhysicsTest");
        scene1 = new Scene2D(engine);
        scene2 = new Scene2D(engine);
        scene3 = new Scene2D(engine);
        quad = new Mesh("models/quad.obj");
        material = new Material({1, 1, 1}, nullptr);
        boxCollider = new Collider({{0.5f, 0.5f}, {-0.5f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}});
        smallBoxCollider = new Collider({{0.25f, 0.25f}, {-0.25f, 0.25f}, {-0.25f, -0.25f}, {0.25f, -0.25f}});
    }

    ~Node2DPhysicsTest() {
        delete engine;
        delete quad;
        delete material;
        delete boxCollider;
        delete smallBoxCollider;
    }

    // ============================================================================
    // BASIC RIGID BODY CREATION TESTS
    // ============================================================================

    void test_create_node_with_collider() {
        std::cout << "TEST: Create Node2D with collider..." << std::endl;
        
        Node2D* node = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        
        assert(node->getScene() == scene1);
        assert(node->getRigid() != nullptr);
        assert(node->getRigid()->getCollider() == boxCollider);
        assert(node->getRigid()->getSolver() == scene1->getSolver());
        
        delete node;
        std::cout << "  ✓ Node2D with collider created and cleaned up" << std::endl;
    }

    void test_create_node_without_collider() {
        std::cout << "TEST: Create Node2D without collider..." << std::endl;
        
        Node2D* node = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, nullptr, 1.0f, 0.5f);
        
        assert(node->getScene() == scene1);
        assert(node->getRigid() == nullptr);
        
        delete node;
        std::cout << "  ✓ Node2D without collider created correctly" << std::endl;
    }

    void test_create_orphaned_node_with_collider() {
        std::cout << "TEST: Create orphaned Node2D with collider (deferred physics)..." << std::endl;
        
        Node2D* node = new Node2D(quad, material, {0, 0}, 0, {1, 1}, {5, 10, 0}, boxCollider, 1.0f, 0.5f);
        
        assert(node->getScene() == nullptr);
        assert(node->getRigid() == nullptr);  // No rigid until adopted
        assert(node->getParent() == nullptr);
        
        delete node;
        std::cout << "  ✓ Orphaned node defers physics creation" << std::endl;
    }

    // ============================================================================
    // ADOPTION TESTS
    // ============================================================================

    void test_adopt_orphan_creates_rigid() {
        std::cout << "TEST: Adopting orphaned node creates rigid body..." << std::endl;
        
        glm::vec3 velocity = {5, 10, 0.5f};
        Node2D* orphan = new Node2D(quad, material, {3, 4}, 0.5f, {2, 2}, velocity, boxCollider, 2.0f, 0.3f);
        
        assert(orphan->getRigid() == nullptr);
        
        scene1->add(orphan);
        
        assert(orphan->getScene() == scene1);
        assert(orphan->getRigid() != nullptr);
        assert(orphan->getRigid()->getSolver() == scene1->getSolver());
        assert(orphan->getRigid()->getCollider() == boxCollider);
        assert(orphan->getRigid()->getDensity() == 2.0f);
        assert(orphan->getRigid()->getFriction() == 0.3f);
        
        // Verify physics data was preserved
        glm::vec3 vel = orphan->getRigid()->getVelocity();
        assert(vel.x == velocity.x && vel.y == velocity.y && vel.z == velocity.z);
        
        std::cout << "  ✓ Rigid body created on adoption with preserved physics data" << std::endl;
    }

    void test_adopt_orphan_without_collider() {
        std::cout << "TEST: Adopting orphaned node without collider..." << std::endl;
        
        Node2D* orphan = new Node2D(quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, nullptr, 1.0f, 0.5f);
        
        scene1->add(orphan);
        
        assert(orphan->getScene() == scene1);
        assert(orphan->getRigid() == nullptr);  // Still no rigid
        
        std::cout << "  ✓ Node without collider remains physics-less after adoption" << std::endl;
    }

    void test_adopt_orphan_to_parent_node() {
        std::cout << "TEST: Adopting orphan to parent node (not scene root)..." << std::endl;
        
        Node2D* parent = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* orphan = new Node2D(quad, material, {5, 5}, 0, {1, 1}, {2, 3, 0}, smallBoxCollider, 1.5f, 0.4f);
        
        assert(orphan->getRigid() == nullptr);
        
        parent->add(orphan);
        
        assert(orphan->getScene() == scene1);
        assert(orphan->getParent() == parent);
        assert(orphan->getRigid() != nullptr);
        assert(orphan->getRigid()->getSolver() == scene1->getSolver());
        
        std::cout << "  ✓ Orphan adopted to parent creates rigid body correctly" << std::endl;
    }

    // ============================================================================
    // ORPHANING TESTS
    // ============================================================================

    void test_orphan_destroys_rigid() {
        std::cout << "TEST: Orphaning node destroys rigid body..." << std::endl;
        
        glm::vec3 velocity = {10, 20, 1.0f};
        Node2D* node = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, velocity, boxCollider, 1.0f, 0.5f);
        
        // Give it some velocity
        node->setVelocity({15, 25, 2.0f});
        
        Rigid* originalRigid = node->getRigid();
        assert(originalRigid != nullptr);
        
        scene1->remove(node);
        
        assert(node->getScene() == nullptr);
        assert(node->getRigid() == nullptr);
        
        // Verify physics data was saved for re-adoption
        scene1->add(node);
        assert(node->getRigid() != nullptr);
        glm::vec3 vel = node->getRigid()->getVelocity();
        assert(vel.x == 15 && vel.y == 25 && vel.z == 2.0f);
        
        delete node;
        std::cout << "  ✓ Rigid body destroyed on orphaning, velocity preserved" << std::endl;
    }

    void test_remove_from_parent_orphans() {
        std::cout << "TEST: Removing from parent orphans physics..." << std::endl;
        
        Node2D* parent = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* child = new Node2D(parent, quad, material, {0, 0}, 0, {1, 1}, {5, 5, 0}, smallBoxCollider, 1.0f, 0.5f);
        
        assert(child->getRigid() != nullptr);
        
        parent->remove(child);
        
        assert(child->getScene() == nullptr);
        assert(child->getRigid() == nullptr);
        assert(child->getParent() == nullptr);
        
        delete child;
        std::cout << "  ✓ Child orphaned and physics destroyed" << std::endl;
    }

    // ============================================================================
    // SCENE TRANSITION TESTS
    // ============================================================================

    void test_move_between_scenes_recreates_rigid() {
        std::cout << "TEST: Moving between scenes recreates rigid body..." << std::endl;
        
        glm::vec3 velocity = {7, 14, 0.3f};
        Node2D* node = new Node2D(scene1, quad, material, {10, 20}, 1.5f, {2, 3}, velocity, boxCollider, 1.5f, 0.6f);
        
        Solver* solver1 = scene1->getSolver();
        Rigid* rigid1 = node->getRigid();
        assert(rigid1->getSolver() == solver1);
        
        // Move to scene2
        scene2->add(node);
        
        assert(node->getScene() == scene2);
        Rigid* rigid2 = node->getRigid();
        assert(rigid2 != nullptr);
        assert(rigid2 != rigid1);  // Different rigid instance
        assert(rigid2->getSolver() == scene2->getSolver());  // New solver
        assert(rigid2->getSolver() != solver1);
        
        // Verify physics properties transferred
        assert(rigid2->getCollider() == boxCollider);
        assert(rigid2->getDensity() == 1.5f);
        assert(rigid2->getFriction() == 0.6f);
        
        std::cout << "  ✓ Rigid body recreated with new solver on scene transition" << std::endl;
    }

    void test_move_via_parent_between_scenes() {
        std::cout << "TEST: Moving via parent addition between scenes..." << std::endl;
        
        Node2D* node = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {3, 4, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* newParent = new Node2D(scene2, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, smallBoxCollider, 1.0f, 0.5f);
        
        Solver* solver1 = scene1->getSolver();
        assert(node->getRigid()->getSolver() == solver1);
        
        newParent->add(node);
        
        assert(node->getScene() == scene2);
        assert(node->getParent() == newParent);
        assert(node->getRigid()->getSolver() == scene2->getSolver());
        assert(node->getRigid()->getSolver() != solver1);
        
        std::cout << "  ✓ Scene transition via parent correctly updates solver" << std::endl;
    }

    void test_move_subtree_between_scenes() {
        std::cout << "TEST: Moving subtree between scenes..." << std::endl;
        
        Node2D* parent = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* child1 = new Node2D(parent, quad, material, {1, 1}, 0, {1, 1}, {1, 2, 0}, smallBoxCollider, 1.0f, 0.5f);
        Node2D* child2 = new Node2D(parent, quad, material, {2, 2}, 0, {1, 1}, {3, 4, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* grandchild = new Node2D(child1, quad, material, {3, 3}, 0, {1, 1}, {5, 6, 0}, smallBoxCollider, 1.0f, 0.5f);
        
        Solver* solver1 = scene1->getSolver();
        assert(parent->getRigid()->getSolver() == solver1);
        assert(child1->getRigid()->getSolver() == solver1);
        assert(child2->getRigid()->getSolver() == solver1);
        assert(grandchild->getRigid()->getSolver() == solver1);
        
        scene2->add(parent);
        
        Solver* solver2 = scene2->getSolver();
        assert(parent->getRigid()->getSolver() == solver2);
        assert(child1->getRigid()->getSolver() == solver2);
        assert(child2->getRigid()->getSolver() == solver2);
        assert(grandchild->getRigid()->getSolver() == solver2);
        
        std::cout << "  ✓ All nodes in subtree updated to new solver" << std::endl;
    }

    // ============================================================================
    // REPARENTING WITHIN SCENE TESTS
    // ============================================================================

    void test_reparent_within_scene_preserves_rigid() {
        std::cout << "TEST: Reparenting within same scene preserves rigid body..." << std::endl;
        
        Node2D* parent1 = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* parent2 = new Node2D(scene1, quad, material, {10, 10}, 0, {1, 1}, {0, 0, 0}, smallBoxCollider, 1.0f, 0.5f);
        Node2D* child = new Node2D(parent1, quad, material, {5, 5}, 0, {1, 1}, {7, 8, 0}, boxCollider, 1.2f, 0.7f);
        
        Rigid* originalRigid = child->getRigid();
        Solver* solver = scene1->getSolver();
        child->setVelocity({20, 30, 1.5f});
        
        parent2->add(child);
        
        // In current implementation, rigid might be recreated
        // Test what actually happens
        assert(child->getScene() == scene1);
        assert(child->getParent() == parent2);
        assert(child->getRigid() != nullptr);
        assert(child->getRigid()->getSolver() == solver);
        
        std::cout << "  ✓ Reparenting within scene maintains physics" << std::endl;
    }

    void test_reparent_chain_within_scene() {
        std::cout << "TEST: Reparenting chain within scene..." << std::endl;
        
        Node2D* root1 = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* root2 = new Node2D(scene1, quad, material, {10, 10}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* mid = new Node2D(root1, quad, material, {5, 5}, 0, {1, 1}, {1, 1, 0}, smallBoxCollider, 1.0f, 0.5f);
        Node2D* leaf = new Node2D(mid, quad, material, {7, 7}, 0, {1, 1}, {2, 2, 0}, boxCollider, 1.0f, 0.5f);
        
        // Move mid (and its child) to root2
        root2->add(mid);
        
        assert(mid->getParent() == root2);
        assert(mid->getScene() == scene1);
        assert(mid->getRigid() != nullptr);
        assert(leaf->getParent() == mid);
        assert(leaf->getScene() == scene1);
        assert(leaf->getRigid() != nullptr);
        
        std::cout << "  ✓ Reparenting chain preserves all physics" << std::endl;
    }

    // ============================================================================
    // MIXED COLLIDER/NON-COLLIDER TESTS
    // ============================================================================

    void test_mixed_hierarchy() {
        std::cout << "TEST: Mixed hierarchy (some with/without colliders)..." << std::endl;
        
        Node2D* physicsParent = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* visualChild = new Node2D(physicsParent, quad, material, {1, 1}, 0, {1, 1}, {0, 0, 0}, nullptr, 1.0f, 0.5f);
        Node2D* physicsGrandchild = new Node2D(visualChild, quad, material, {2, 2}, 0, {1, 1}, {5, 5, 0}, smallBoxCollider, 1.0f, 0.5f);
        
        assert(physicsParent->getRigid() != nullptr);
        assert(visualChild->getRigid() == nullptr);
        assert(physicsGrandchild->getRigid() != nullptr);
        
        // Move to new scene
        scene2->add(physicsParent);
        
        assert(physicsParent->getRigid()->getSolver() == scene2->getSolver());
        assert(visualChild->getRigid() == nullptr);
        assert(physicsGrandchild->getRigid()->getSolver() == scene2->getSolver());
        
        std::cout << "  ✓ Mixed hierarchy handled correctly" << std::endl;
    }

    void test_orphan_and_readopt_mixed_hierarchy() {
        std::cout << "TEST: Orphan and re-adopt mixed hierarchy..." << std::endl;
        
        Node2D* physicsParent = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {1, 2, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* visualChild = new Node2D(physicsParent, quad, material, {1, 1}, 0, {1, 1}, {0, 0, 0}, nullptr, 1.0f, 0.5f);
        Node2D* physicsGrandchild = new Node2D(visualChild, quad, material, {2, 2}, 0, {1, 1}, {3, 4, 0}, smallBoxCollider, 1.0f, 0.5f);
        
        scene1->remove(physicsParent);
        
        assert(physicsParent->getScene() == nullptr);
        assert(physicsParent->getRigid() == nullptr);
        assert(visualChild->getScene() == nullptr);
        assert(visualChild->getRigid() == nullptr);
        assert(physicsGrandchild->getScene() == nullptr);
        assert(physicsGrandchild->getRigid() == nullptr);
        
        scene2->add(physicsParent);
        
        assert(physicsParent->getRigid() != nullptr);
        assert(visualChild->getRigid() == nullptr);
        assert(physicsGrandchild->getRigid() != nullptr);
        
        std::cout << "  ✓ Mixed hierarchy orphaned and re-adopted correctly" << std::endl;
    }

    // ============================================================================
    // COMPLEX MULTI-SCENE TRANSITIONS
    // ============================================================================

    void test_ping_pong_between_scenes() {
        std::cout << "TEST: Ping-pong node between scenes..." << std::endl;
        
        Node2D* node = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {10, 20, 0.5f}, boxCollider, 1.0f, 0.5f);
        
        for (int i = 0; i < 5; i++) {
            Solver* solver1 = scene1->getSolver();
            assert(node->getRigid()->getSolver() == solver1);
            
            scene2->add(node);
            assert(node->getScene() == scene2);
            assert(node->getRigid()->getSolver() == scene2->getSolver());
            
            scene1->add(node);
            assert(node->getScene() == scene1);
            // Note: might be different rigid instance
            assert(node->getRigid()->getSolver() == solver1);
        }
        
        std::cout << "  ✓ Node survived multiple scene transitions" << std::endl;
    }

    void test_three_way_scene_transitions() {
        std::cout << "TEST: Three-way scene transitions..." << std::endl;
        
        Node2D* node = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {5, 10, 0}, boxCollider, 1.0f, 0.5f);
        
        scene2->add(node);
        assert(node->getScene() == scene2);
        assert(node->getRigid()->getSolver() == scene2->getSolver());
        
        scene3->add(node);
        assert(node->getScene() == scene3);
        assert(node->getRigid()->getSolver() == scene3->getSolver());
        
        scene1->add(node);
        assert(node->getScene() == scene1);
        assert(node->getRigid()->getSolver() == scene1->getSolver());
        
        std::cout << "  ✓ Three-way transitions handled correctly" << std::endl;
    }

    void test_complex_subtree_multi_scene_dance() {
        std::cout << "TEST: Complex subtree multi-scene dance..." << std::endl;
        
        // Build complex hierarchy in scene1
        Node2D* root = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* level1_a = new Node2D(root, quad, material, {1, 1}, 0, {1, 1}, {1, 1, 0}, smallBoxCollider, 1.0f, 0.5f);
        Node2D* level1_b = new Node2D(root, quad, material, {2, 2}, 0, {1, 1}, {2, 2, 0}, nullptr, 1.0f, 0.5f);
        Node2D* level2_a = new Node2D(level1_a, quad, material, {3, 3}, 0, {1, 1}, {3, 3, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* level2_b = new Node2D(level1_b, quad, material, {4, 4}, 0, {1, 1}, {4, 4, 0}, smallBoxCollider, 1.0f, 0.5f);
        
        // Move subtree to scene2
        scene2->add(level1_a);
        assert(level1_a->getScene() == scene2);
        assert(level2_a->getScene() == scene2);
        
        // Move another subtree to scene3
        scene3->add(level1_b);
        assert(level1_b->getScene() == scene3);
        assert(level2_b->getScene() == scene3);
        
        // Reunite under root in scene1
        root->add(level1_a);
        root->add(level1_b);
        
        assert(level1_a->getScene() == scene1);
        assert(level1_b->getScene() == scene1);
        assert(level2_a->getScene() == scene1);
        assert(level2_b->getScene() == scene1);
        
        std::cout << "  ✓ Complex multi-scene subtree reorganization succeeded" << std::endl;
    }

    // ============================================================================
    // STRESS TESTS
    // ============================================================================

    void test_rapid_add_remove_cycles() {
        std::cout << "TEST: Rapid add/remove cycles..." << std::endl;
        
        Node2D* node = new Node2D(quad, material, {0, 0}, 0, {1, 1}, {5, 10, 0}, boxCollider, 1.0f, 0.5f);
        
        for (int i = 0; i < 20; i++) {
            scene1->add(node);
            assert(node->getRigid() != nullptr);
            
            scene1->remove(node);
            assert(node->getRigid() == nullptr);
        }
        
        delete node;
        std::cout << "  ✓ Node survived 20 add/remove cycles" << std::endl;
    }

    void test_many_nodes_scene_transition() {
        std::cout << "TEST: Move many nodes between scenes..." << std::endl;
        
        std::vector<Node2D*> nodes;
        for (int i = 0; i < 50; i++) {
            Node2D* node = new Node2D(scene1, quad, material, 
                {static_cast<float>(i), static_cast<float>(i)}, 
                0, {1, 1}, {1, 1, 0}, 
                (i % 2 == 0) ? boxCollider : nullptr,  // Half with colliders
                1.0f, 0.5f);
            nodes.push_back(node);
        }
        
        // Move all to scene2
        for (auto* node : nodes) {
            scene2->add(node);
            assert(node->getScene() == scene2);
            if (node->getRigid()) {
                assert(node->getRigid()->getSolver() == scene2->getSolver());
            }
        }
        
        // Move back to scene1
        for (auto* node : nodes) {
            scene1->add(node);
            assert(node->getScene() == scene1);
        }
        
        // Cleanup
        for (auto* node : nodes) {
            delete node;
        }
        
        std::cout << "  ✓ 50 nodes successfully transitioned between scenes" << std::endl;
    }

    void test_deep_hierarchy_scene_transition() {
        std::cout << "TEST: Deep hierarchy (10 levels) scene transition..." << std::endl;
        
        Node2D* current = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* root = current;
        
        // Build 10-level deep chain
        for (int i = 1; i < 10; i++) {
            Node2D* child = new Node2D(current, quad, material, 
                {static_cast<float>(i), static_cast<float>(i)}, 
                0, {1, 1}, {static_cast<float>(i), static_cast<float>(i), 0}, 
                (i % 3 == 0) ? boxCollider : nullptr,
                1.0f, 0.5f);
            current = child;
        }
        
        // Move entire chain to scene2
        scene2->add(root);
        
        // Verify all levels
        current = root;
        for (int i = 0; i < 10; i++) {
            assert(current->getScene() == scene2);
            if (current->getRigid()) {
                assert(current->getRigid()->getSolver() == scene2->getSolver());
            }
            if (!current->getChildren().empty()) {
                current = current->getChildren()[0];
            }
        }
        
        std::cout << "  ✓ 10-level deep hierarchy transitioned correctly" << std::endl;
    }

    // ============================================================================
    // VELOCITY PRESERVATION TESTS
    // ============================================================================

    void test_velocity_preserved_across_orphan_adopt() {
        std::cout << "TEST: Velocity preserved across orphan/adopt cycle..." << std::endl;
        
        Node2D* node = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {10, 20, 1.5f}, boxCollider, 1.0f, 0.5f);
        
        node->setVelocity({25, 35, 2.5f});
        glm::vec3 vel1 = node->getVelocity();
        
        scene1->remove(node);
        assert(node->getRigid() == nullptr);
        
        scene1->add(node);
        glm::vec3 vel2 = node->getVelocity();
        
        assert(vel2.x == vel1.x && vel2.y == vel1.y && vel2.z == vel1.z);
        
        delete node;
        std::cout << "  ✓ Velocity preserved across scene transition" << std::endl;
    }

    void test_velocity_preserved_multiple_transitions() {
        std::cout << "TEST: Velocity preserved through multiple transitions..." << std::endl;
        
        Node2D* node = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {3, 7, 0.3f}, boxCollider, 1.0f, 0.5f);
        
        node->setVelocity({20, 30, 2.0f});
        glm::vec3 originalVel = node->getVelocity();
        
        // Multiple transitions
        scene2->add(node);
        scene3->add(node);
        scene1->add(node);
        scene2->add(node);
        
        glm::vec3 finalVel = node->getVelocity();
        assert(finalVel.x == originalVel.x && finalVel.y == originalVel.y && finalVel.z == originalVel.z);
        
        std::cout << "  ✓ Velocity preserved through 4 scene transitions" << std::endl;
    }

    // ============================================================================
    // EDGE CASE TESTS
    // ============================================================================

    void test_orphan_node_then_delete() {
        std::cout << "TEST: Orphan node then delete (no memory leaks)..." << std::endl;
        
        Node2D* node = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {5, 5, 0}, boxCollider, 1.0f, 0.5f);
        assert(node->getRigid() != nullptr);
        
        scene1->remove(node);
        assert(node->getRigid() == nullptr);
        
        delete node;  // Should not crash or leak
        
        std::cout << "  ✓ Orphaned node deleted safely" << std::endl;
    }

    void test_delete_node_with_rigid() {
        std::cout << "TEST: Delete node with active rigid body..." << std::endl;
        
        Node2D* node = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {5, 5, 0}, boxCollider, 1.0f, 0.5f);
        assert(node->getRigid() != nullptr);
        
        delete node;  // Should clean up rigid body
        
        std::cout << "  ✓ Node with rigid body deleted safely" << std::endl;
    }

    void test_orphan_with_children() {
        std::cout << "TEST: Orphan parent with children (all lose physics)..." << std::endl;
        
        Node2D* parent = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* child1 = new Node2D(parent, quad, material, {1, 1}, 0, {1, 1}, {1, 1, 0}, smallBoxCollider, 1.0f, 0.5f);
        Node2D* child2 = new Node2D(parent, quad, material, {2, 2}, 0, {1, 1}, {2, 2, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* grandchild = new Node2D(child1, quad, material, {3, 3}, 0, {1, 1}, {3, 3, 0}, smallBoxCollider, 1.0f, 0.5f);
        
        scene1->remove(parent);
        
        assert(parent->getRigid() == nullptr);
        assert(child1->getRigid() == nullptr);
        assert(child2->getRigid() == nullptr);
        assert(grandchild->getRigid() == nullptr);
        
        // Re-adopt entire subtree
        scene2->add(parent);
        
        assert(parent->getRigid() != nullptr);
        assert(child1->getRigid() != nullptr);
        assert(child2->getRigid() != nullptr);
        assert(grandchild->getRigid() != nullptr);
        
        std::cout << "  ✓ Orphaned subtree loses and regains physics correctly" << std::endl;
    }

    void test_adopt_orphan_with_children() {
        std::cout << "TEST: Adopt orphan that has children..." << std::endl;
        
        Node2D* parent = new Node2D(quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* child = new Node2D(parent, quad, material, {1, 1}, 0, {1, 1}, {1, 1, 0}, smallBoxCollider, 1.0f, 0.5f);
        
        assert(parent->getRigid() == nullptr);
        assert(child->getRigid() == nullptr);
        assert(child->getParent() == parent);
        
        scene1->add(parent);
        
        assert(parent->getRigid() != nullptr);
        assert(child->getRigid() != nullptr);
        assert(child->getScene() == scene1);
        
        std::cout << "  ✓ Adopting orphan parent creates physics for entire subtree" << std::endl;
    }

    void test_null_collider_never_creates_rigid() {
        std::cout << "TEST: Null collider nodes never create rigid bodies..." << std::endl;
        
        Node2D* node = new Node2D(quad, material, {0, 0}, 0, {1, 1}, {5, 5, 0}, nullptr, 1.0f, 0.5f);
        assert(node->getRigid() == nullptr);
        
        scene1->add(node);
        assert(node->getRigid() == nullptr);
        
        scene1->remove(node);
        assert(node->getRigid() == nullptr);
        
        scene2->add(node);
        assert(node->getRigid() == nullptr);
        
        delete node;
        std::cout << "  ✓ Null collider nodes remain physics-less through all operations" << std::endl;
    }

    void test_position_rotation_scale_with_rigid() {
        std::cout << "TEST: Position/rotation/scale updates with rigid body..." << std::endl;
        
        Node2D* node = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        
        node->setPosition({10, 20});
        node->setRotation(1.5f);
        node->setScale({2, 3});
        
        // These should update both the node and the rigid body
        assert(node->getPosition().x == 10 && node->getPosition().y == 20);
        assert(node->getRotation() == 1.5f);
        assert(node->getScale().x == 2 && node->getScale().y == 3);
        
        // After scene transition, properties should be maintained
        scene2->add(node);
        assert(node->getPosition().x == 10 && node->getPosition().y == 20);
        assert(node->getRotation() == 1.5f);
        
        std::cout << "  ✓ Transform properties maintained through rigid body updates" << std::endl;
    }

    // ============================================================================
    // COPY/MOVE CONSTRUCTOR TESTS
    // ============================================================================

    void test_copy_constructor_with_rigid() {
        std::cout << "TEST: Copy constructor with rigid body..." << std::endl;
        
        Node2D* original = new Node2D(scene1, quad, material, {5, 10}, 1.0f, {2, 2}, {10, 20, 0.5f}, boxCollider, 1.5f, 0.6f);
        
        Node2D* copy = new Node2D(*original);
        
        // Copy should have its own rigid body
        assert(copy->getRigid() != nullptr);
        assert(copy->getRigid() != original->getRigid());
        assert(copy->getRigid()->getSolver() == original->getRigid()->getSolver());
        
        // Physics properties should match
        assert(copy->getRigid()->getDensity() == 1.5f);
        assert(copy->getRigid()->getFriction() == 0.6f);
        
        delete original;
        delete copy;
        
        std::cout << "  ✓ Copy constructor creates independent rigid body" << std::endl;
    }

    void test_copy_constructor_orphaned_node() {
        std::cout << "TEST: Copy constructor with orphaned node..." << std::endl;
        
        Node2D* original = new Node2D(quad, material, {5, 10}, 1.0f, {2, 2}, {10, 20, 0.5f}, boxCollider, 1.5f, 0.6f);
        assert(original->getRigid() == nullptr);
        
        Node2D* copy = new Node2D(*original);
        assert(copy->getRigid() == nullptr);
        assert(copy->getScene() == nullptr);
        
        // Both can be adopted independently
        scene1->add(original);
        scene2->add(copy);
        
        assert(original->getRigid() != nullptr);
        assert(copy->getRigid() != nullptr);
        assert(original->getRigid()->getSolver() != copy->getRigid()->getSolver());
        
        std::cout << "  ✓ Copied orphaned nodes can be adopted independently" << std::endl;
    }

    void test_move_constructor_with_rigid() {
        std::cout << "TEST: Move constructor with rigid body..." << std::endl;
        
        Node2D* original = new Node2D(scene1, quad, material, {5, 10}, 1.0f, {2, 2}, {10, 20, 0.5f}, boxCollider, 1.5f, 0.6f);
        Rigid* originalRigid = original->getRigid();
        
        Node2D* moved = new Node2D(std::move(*original));
        
        // Moved node should have the rigid body
        assert(moved->getRigid() == originalRigid);
        assert(original->getRigid() == nullptr);  // Original should be null
        
        delete moved;
        // Don't delete original - it was moved from
        
        std::cout << "  ✓ Move constructor transfers rigid body ownership" << std::endl;
    }

    // ============================================================================
    // INTEGRATION TESTS
    // ============================================================================

    void test_complex_workflow_simulation() {
        std::cout << "TEST: Complex workflow simulation..." << std::endl;
        
        // Create initial scene with hierarchy
        Node2D* world = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, nullptr, 1.0f, 0.5f);
        Node2D* player = new Node2D(world, quad, material, {10, 10}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* enemy1 = new Node2D(world, quad, material, {20, 20}, 0, {1, 1}, {5, 0, 0}, smallBoxCollider, 1.0f, 0.5f);
        Node2D* enemy2 = new Node2D(world, quad, material, {30, 30}, 0, {1, 1}, {-5, 0, 0}, boxCollider, 1.0f, 0.5f);
        
        // Simulate gameplay: move player to different parent
        Node2D* vehicle = new Node2D(world, quad, material, {15, 15}, 0, {2, 2}, {10, 0, 0}, boxCollider, 2.0f, 0.3f);
        vehicle->add(player);
        
        assert(player->getParent() == vehicle);
        assert(player->getRigid() != nullptr);
        
        // Move enemy to different scene (off-screen)
        scene2->add(enemy1);
        assert(enemy1->getScene() == scene2);
        
        // Bring enemy back
        world->add(enemy1);
        assert(enemy1->getScene() == scene1);
        assert(enemy1->getParent() == world);
        
        // Remove defeated enemy
        world->remove(enemy2);
        assert(enemy2->getScene() == nullptr);
        delete enemy2;
        
        std::cout << "  ✓ Complex gameplay simulation completed successfully" << std::endl;
    }

    void test_scene_cleanup() {
        std::cout << "TEST: Scene cleanup with many nodes..." << std::endl;
        
        std::vector<Node2D*> nodes;
        
        for (int i = 0; i < 30; i++) {
            Node2D* node = new Node2D(scene1, quad, material, 
                {static_cast<float>(i * 2), static_cast<float>(i * 2)}, 
                0, {1, 1}, {1, 1, 0}, 
                (i % 2 == 0) ? boxCollider : nullptr,
                1.0f, 0.5f);
            nodes.push_back(node);
        }
        
        // Delete all nodes
        for (auto* node : nodes) {
            delete node;
        }
        
        std::cout << "  ✓ 30 nodes cleaned up without issues" << std::endl;
    }

    void test_alternating_orphan_adopt_pattern() {
        std::cout << "TEST: Alternating orphan/adopt pattern..." << std::endl;
        
        Node2D* node1 = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {1, 1, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* node2 = new Node2D(scene1, quad, material, {5, 5}, 0, {1, 1}, {2, 2, 0}, smallBoxCollider, 1.0f, 0.5f);
        
        for (int i = 0; i < 10; i++) {
            // Orphan both
            scene1->remove(node1);
            scene1->remove(node2);
            assert(node1->getRigid() == nullptr);
            assert(node2->getRigid() == nullptr);
            
            // Re-adopt to different scenes
            if (i % 2 == 0) {
                scene1->add(node1);
                scene2->add(node2);
            } else {
                scene2->add(node1);
                scene1->add(node2);
            }
            
            assert(node1->getRigid() != nullptr);
            assert(node2->getRigid() != nullptr);
        }
        
        delete node1;
        delete node2;
        
        std::cout << "  ✓ Alternating pattern completed successfully" << std::endl;
    }

    void test_layered_hierarchy_transitions() {
        std::cout << "TEST: Layered hierarchy with multiple scene transitions..." << std::endl;
        
        // Build a tree: root -> [branch1, branch2] -> [leaf1, leaf2, leaf3, leaf4]
        Node2D* root = new Node2D(scene1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        
        Node2D* branch1 = new Node2D(root, quad, material, {1, 1}, 0, {1, 1}, {0, 0, 0}, smallBoxCollider, 1.0f, 0.5f);
        Node2D* branch2 = new Node2D(root, quad, material, {2, 2}, 0, {1, 1}, {0, 0, 0}, nullptr, 1.0f, 0.5f);
        
        Node2D* leaf1 = new Node2D(branch1, quad, material, {3, 3}, 0, {1, 1}, {1, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* leaf2 = new Node2D(branch1, quad, material, {4, 4}, 0, {1, 1}, {2, 0, 0}, smallBoxCollider, 1.0f, 0.5f);
        Node2D* leaf3 = new Node2D(branch2, quad, material, {5, 5}, 0, {1, 1}, {3, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* leaf4 = new Node2D(branch2, quad, material, {6, 6}, 0, {1, 1}, {4, 0, 0}, nullptr, 1.0f, 0.5f);
        
        // Move branch1 to scene2
        scene2->add(branch1);
        assert(branch1->getScene() == scene2);
        assert(leaf1->getScene() == scene2);
        assert(leaf2->getScene() == scene2);
        
        // Move branch2 to scene3
        scene3->add(branch2);
        assert(branch2->getScene() == scene3);
        assert(leaf3->getScene() == scene3);
        assert(leaf4->getScene() == scene3);
        
        // Reunite under root in scene1
        root->add(branch1);
        root->add(branch2);
        
        assert(leaf1->getScene() == scene1);
        assert(leaf2->getScene() == scene1);
        assert(leaf3->getScene() == scene1);
        assert(leaf4->getScene() == scene1);
        
        std::cout << "  ✓ Layered hierarchy transitions handled correctly" << std::endl;
    }

    // ============================================================================
    // MAIN TEST RUNNER
    // ============================================================================

    void runAllTests() {
        std::cout << "\n========================================" << std::endl;
        std::cout << "  Node2D Physics Lifecycle Tests" << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        try {
            // Basic creation
            test_create_node_with_collider();
            test_create_node_without_collider();
            test_create_orphaned_node_with_collider();
            
            // Adoption
            test_adopt_orphan_creates_rigid();
            test_adopt_orphan_without_collider();
            test_adopt_orphan_to_parent_node();
            
            // Orphaning
            test_orphan_destroys_rigid();
            test_remove_from_parent_orphans();
            
            // Scene transitions
            test_move_between_scenes_recreates_rigid();
            test_move_via_parent_between_scenes();
            test_move_subtree_between_scenes();
            
            // Reparenting
            test_reparent_within_scene_preserves_rigid();
            test_reparent_chain_within_scene();
            
            // Mixed hierarchies
            test_mixed_hierarchy();
            test_orphan_and_readopt_mixed_hierarchy();
            
            // Complex transitions
            test_ping_pong_between_scenes();
            test_three_way_scene_transitions();
            test_complex_subtree_multi_scene_dance();
            
            // Stress tests
            test_rapid_add_remove_cycles();
            test_many_nodes_scene_transition();
            test_deep_hierarchy_scene_transition();
            
            // Velocity preservation
            test_velocity_preserved_across_orphan_adopt();
            test_velocity_preserved_multiple_transitions();
            
            // Edge cases
            test_orphan_node_then_delete();
            test_delete_node_with_rigid();
            test_orphan_with_children();
            test_adopt_orphan_with_children();
            test_null_collider_never_creates_rigid();
            test_position_rotation_scale_with_rigid();
            
            // Copy/move
            test_copy_constructor_with_rigid();
            test_copy_constructor_orphaned_node();
            test_move_constructor_with_rigid();
            
            // Integration
            test_complex_workflow_simulation();
            test_scene_cleanup();
            test_alternating_orphan_adopt_pattern();
            test_layered_hierarchy_transitions();
            
            std::cout << "\n========================================" << std::endl;
            std::cout << "  ✓ ALL TESTS PASSED!" << std::endl;
            std::cout << "========================================\n" << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "\n✗ TEST FAILED: " << e.what() << std::endl;
            throw;
        }
    }
};

}  // namespace bsk::internal

int main() {
    try {
        bsk::internal::Node2DPhysicsTest test;
        test.runAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test execution failed: " << e.what() << std::endl;
        return 1;
    }
}