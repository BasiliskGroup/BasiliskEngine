#include <basilisk/basilisk.h>
#include <basilisk/physics/rigid.h>
#include <cassert>
#include <iostream>
#include <stdexcept>

namespace bsk::internal {

class NodeTrackingTest {
private:
    Engine* engine;
    Scene* scene1;
    Scene* scene2;
    Scene2D* scene2d1;
    Scene2D* scene2d2;
    Mesh* quad;
    Material* material;
    Collider* boxCollider;

public:
    NodeTrackingTest() {
        engine = new Engine(800, 800, "NodeTrackingTest");
        scene1 = new Scene(engine);
        scene2 = new Scene(engine);
        scene2d1 = new Scene2D(engine);
        scene2d2 = new Scene2D(engine);
        quad = new Mesh("models/quad.obj");
        material = new Material({1, 1, 1}, nullptr);
        boxCollider = new Collider({{0.5f, 0.5f}, {-0.5f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}});
    }

    ~NodeTrackingTest() {
        // Engine destructor handles all scene cleanup
        delete engine;
        delete quad;
        delete material;
        delete boxCollider;
    }

    void test_add_node_to_scene() {
        std::cout << "TEST: Add node to scene..." << std::endl;
        
        Node* node = new Node(scene1, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        assert(node->getScene() == scene1);
        assert(node->getParent() == scene1->getRoot());
        assert(scene1->getRoot()->getChildren().size() == 1);
        
        std::cout << "  ✓ Node successfully added to scene root" << std::endl;
    }

    void test_add_node_to_node() {
        std::cout << "TEST: Add node to another node..." << std::endl;
        
        Node* parent = new Node(scene1, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        Node* child = new Node(quad, material, {0, 0, 0}, {}, {1, 1, 1});
        
        assert(child->getScene() == nullptr);  // Orphaned
        assert(child->getParent() == nullptr);
        
        parent->add(child);
        
        assert(child->getScene() == scene1);  // Adopted child's scene
        assert(child->getParent() == parent);
        assert(parent->getChildren().size() == 1);
        
        std::cout << "  ✓ Child node successfully added to parent node" << std::endl;
    }

    void test_reparent_within_scene() {
        std::cout << "TEST: Reparent node within same scene..." << std::endl;
        
        Node* parent1 = new Node(scene1, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        Node* parent2 = new Node(scene1, quad, material, {5, 5, 5}, {}, {1, 1, 1});
        Node* child = new Node(parent1, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        
        assert(parent1->getChildren().size() == 1);
        assert(child->getParent() == parent1);
        
        parent2->add(child);
        
        assert(parent1->getChildren().size() == 0);  // Child removed from parent1
        assert(parent2->getChildren().size() == 1);  // Child added to parent2
        assert(child->getParent() == parent2);
        assert(child->getScene() == scene1);  // Scene unchanged
        
        std::cout << "  ✓ Child successfully reparented within scene" << std::endl;
    }

    void test_move_node_between_scenes() {
        std::cout << "TEST: Move node between scenes..." << std::endl;
        
        Node* node = new Node(scene1, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        assert(node->getScene() == scene1);
        
        Node* newParent = new Node(scene2, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        newParent->add(node);
        
        assert(node->getScene() == scene2);  // Scene changed
        assert(node->getParent() == newParent);
        
        std::cout << "  ✓ Node successfully moved to different scene" << std::endl;
    }

    void test_move_subtree_between_scenes() {
        std::cout << "TEST: Move subtree (parent + children) between scenes..." << std::endl;
        
        Node* parent = new Node(scene1, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        Node* child1 = new Node(parent, quad, material, {1, 1, 1}, {}, {1, 1, 1});
        Node* child2 = new Node(parent, quad, material, {2, 2, 2}, {}, {1, 1, 1});
        Node* grandchild = new Node(child1, quad, material, {3, 3, 3}, {}, {1, 1, 1});
        
        assert(parent->getScene() == scene1);
        assert(child1->getScene() == scene1);
        assert(child2->getScene() == scene1);
        assert(grandchild->getScene() == scene1);
        
        Node* newParent = new Node(scene2, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        newParent->add(parent);
        
        // All nodes should now be in scene2
        assert(parent->getScene() == scene2);
        assert(child1->getScene() == scene2);
        assert(child2->getScene() == scene2);
        assert(grandchild->getScene() == scene2);
        
        std::cout << "  ✓ Entire subtree successfully moved between scenes" << std::endl;
    }

    void test_remove_node_orphans_it() {
        std::cout << "TEST: Remove node orphans it..." << std::endl;
        
        Node* parent = new Node(scene1, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        Node* child = new Node(parent, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        
        assert(child->getScene() == scene1);
        assert(child->getParent() == parent);
        
        parent->remove(child);
        
        assert(child->getScene() == nullptr);  // Orphaned
        assert(child->getParent() == nullptr);
        assert(parent->getChildren().size() == 0);
        
        std::cout << "  ✓ Node successfully orphaned (scene and parent set to nullptr)" << std::endl;
    }

    void test_remove_subtree_orphans_all() {
        std::cout << "TEST: Remove subtree orphans all descendants..." << std::endl;
        
        Node* parent = new Node(scene1, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        Node* child1 = new Node(parent, quad, material, {1, 1, 1}, {}, {1, 1, 1});
        Node* child2 = new Node(parent, quad, material, {2, 2, 2}, {}, {1, 1, 1});
        Node* grandchild = new Node(child1, quad, material, {3, 3, 3}, {}, {1, 1, 1});
        
        parent->remove(child1);
        
        // child1 and grandchild should be orphaned
        assert(child1->getScene() == nullptr);
        assert(child1->getParent() == nullptr);
        assert(grandchild->getScene() == nullptr);
        assert(grandchild->getParent() == child1);  // Parent relationship preserved
        
        std::cout << "  ✓ Entire subtree successfully orphaned recursively" << std::endl;
    }

    void test_cycle_detection() {
        std::cout << "TEST: Cycle detection..." << std::endl;
        
        Node* parent = new Node(scene1, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        Node* child = new Node(parent, quad, material, {1, 1, 1}, {}, {1, 1, 1});
        Node* grandchild = new Node(child, quad, material, {2, 2, 2}, {}, {1, 1, 1});
        
        // Try to add parent to grandchild (would create cycle)
        bool caught_exception = false;
        try {
            grandchild->add(parent);
        } catch (const std::runtime_error& e) {
            caught_exception = true;
            std::cout << "  Caught expected exception: " << e.what() << std::endl;
        }
        
        assert(caught_exception);
        assert(parent->getParent() == scene1->getRoot());  // Unchanged
        
        std::cout << "  ✓ Cycle correctly detected and prevented" << std::endl;
    }

    void test_adopt_orphaned_node() {
        std::cout << "TEST: Adopt orphaned node..." << std::endl;
        
        Node* orphan = new Node(quad, material, {0, 0, 0}, {}, {1, 1, 1});
        assert(orphan->getScene() == nullptr);
        assert(orphan->getParent() == nullptr);
        
        Node* parent = new Node(scene1, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        parent->add(orphan);
        
        assert(orphan->getScene() == scene1);
        assert(orphan->getParent() == parent);
        
        std::cout << "  ✓ Orphaned node successfully adopted" << std::endl;
    }

    void test_scene2d_with_collider() {
        std::cout << "TEST: 2D Node with collider..." << std::endl;
        
        Node2D* node = new Node2D(scene2d1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        assert(node->getScene() == scene2d1);
        assert(node->getRigid() != nullptr);
        assert(node->getRigid()->getCollider() == boxCollider);
        
        std::cout << "  ✓ 2D Node with collider correctly initialized" << std::endl;
    }

    void test_scene2d_with_nullptr_collider() {
        std::cout << "TEST: 2D Node with nullptr collider..." << std::endl;
        
        Node2D* node = new Node2D(quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, nullptr, 1.0f, 0.5f);
        assert(node->getScene() == nullptr);
        assert(node->getRigid() == nullptr);
        
        Node2D* parent = new Node2D(scene2d1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        parent->add(node);
        
        assert(node->getScene() == scene2d1);
        assert(node->getParent() == parent);
        // Note: rigid may or may not be created for nodes without colliders, that's OK
        
        std::cout << "  ✓ 2D Node without collider correctly handled" << std::endl;
    }

    void test_scene2d_reparent_physics_nodes() {
        std::cout << "TEST: 2D Reparent nodes with physics..." << std::endl;
        
        Node2D* parent1 = new Node2D(scene2d1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* parent2 = new Node2D(scene2d1, quad, material, {10, 10}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        Node2D* child = new Node2D(parent1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        
        assert(child->getParent() == parent1);
        
        parent2->add(child);
        
        assert(child->getParent() == parent2);
        assert(child->getScene() == scene2d1);
        
        std::cout << "  ✓ Physics nodes successfully reparented" << std::endl;
    }

    void test_scene2d_move_physics_node_between_scenes() {
        std::cout << "TEST: 2D Move physics node between scenes..." << std::endl;
        
        Node2D* node = new Node2D(scene2d1, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        assert(node->getScene() == scene2d1);
        
        Node2D* newParent = new Node2D(scene2d2, quad, material, {0, 0}, 0, {1, 1}, {0, 0, 0}, boxCollider, 1.0f, 0.5f);
        newParent->add(node);
        
        assert(node->getScene() == scene2d2);
        assert(node->getParent() == newParent);
        
        std::cout << "  ✓ Physics node successfully moved between scenes" << std::endl;
    }

    void test_mixed_hierarchy() {
        std::cout << "TEST: Mixed hierarchy (multiple levels)..." << std::endl;
        
        Node* level1_a = new Node(scene1, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        Node* level1_b = new Node(scene1, quad, material, {5, 5, 5}, {}, {1, 1, 1});
        
        Node* level2_a1 = new Node(level1_a, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        Node* level2_a2 = new Node(level1_a, quad, material, {1, 1, 1}, {}, {1, 1, 1});
        Node* level2_b1 = new Node(level1_b, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        
        Node* level3_a1_1 = new Node(level2_a1, quad, material, {0, 0, 0}, {}, {1, 1, 1});
        
        // Move level2_a1 with its child to level1_b
        level1_b->add(level2_a1);
        
        assert(level2_a1->getParent() == level1_b);
        assert(level3_a1_1->getParent() == level2_a1);
        assert(level3_a1_1->getScene() == scene1);
        
        std::cout << "  ✓ Complex mixed hierarchy handled correctly" << std::endl;
    }

    void runAllTests() {
        std::cout << "\n=== Node Tracking Smoke Tests ===" << std::endl;
        
        try {
            test_add_node_to_scene();
            test_add_node_to_node();
            test_reparent_within_scene();
            test_move_node_between_scenes();
            test_move_subtree_between_scenes();
            test_remove_node_orphans_it();
            test_remove_subtree_orphans_all();
            test_cycle_detection();
            test_adopt_orphaned_node();
            test_scene2d_with_collider();
            test_scene2d_with_nullptr_collider();
            test_scene2d_reparent_physics_nodes();
            test_scene2d_move_physics_node_between_scenes();
            test_mixed_hierarchy();
            
            std::cout << "\n=== ✓ All tests passed! ===" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "\n✗ Test failed with exception: " << e.what() << std::endl;
            throw;
        }
    }
};

}  // namespace bsk::internal

int main() {
    try {
        bsk::internal::NodeTrackingTest test;
        test.runAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test execution failed: " << e.what() << std::endl;
        return 1;
    }
}