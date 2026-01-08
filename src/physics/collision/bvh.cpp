#include <basilisk/physics/collision/bvh.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/maths.h>

namespace bsk::internal {

BVH::BVH() :
    root(nullptr), size(0), rebuildTimer(0)
{}

BVH::~BVH() {
    if (root != nullptr) { delete root; root = nullptr; }
    size = 0;
}

void BVH::update() {
    rebuildTimer--;
    if (rebuildTimer < 0) {
        rebuild();
        rebuildTimer = 100;
    } else {
        refitAll();
    }
}

void BVH::insert(Rigid* rigid) {
    if (rigid == nullptr) return;

    // create primative for the rigid
    glm::vec2 bl, tr;
    rigid->getAABB(bl, tr);
    Primative* primative = new Primative(bl, tr, rigid);
    primatives[rigid] = primative;

    // if the tree is empty, set the root to the primative
    if (root == nullptr) {
        root = primative;
        size = 1;
        return;
    }

    // Find the best sibling to attach the new primative to
    auto [cost, sibling] = root->findbestSibling(primative, 0.0f);
    Primative* oldParent = sibling->getParent();
    
    // Create new parent node containing sibling and the new primative
    // This constructor sets sibling->parent = newParent
    Primative* newParent = new Primative(sibling, primative);
    
    if (sibling == root) {
        // Sibling is root, so newParent becomes the new root
        root = newParent;
        newParent->setParent(nullptr);
    } else {
        // swapChild doesn't check parent pointer, just compares pointers
        oldParent->swapChild(sibling, newParent);
        oldParent->refitUpward();
    }
    
    size++;
}

void BVH::remove(Rigid* rigid) {
    if (rigid == nullptr) return;

    auto it = primatives.find(rigid);
    if (it == primatives.end()) return;
    
    Primative* primative = it->second;
    primatives.erase(it);

    // If root, remove the root
    if (root == primative) {
        root = nullptr;
        size = 0;
        delete primative;
        return;
    }

    Primative* parent = primative->getParent();
    Primative* sibling = primative->getSibling();    
    Primative* grand = parent->getParent();

    // If parent was the root, set the root to the sibling
    if (parent == root) {
        root = sibling;
        sibling->setParent(nullptr);
        parent->setLeft(nullptr);
        parent->setRight(nullptr);
        delete parent;
        delete primative;
        size--;
        return;
    }
    
    grand->swapChild(parent, sibling);
    grand->refitUpward();
    parent->setLeft(nullptr);
    parent->setRight(nullptr);
    parent->setParent(nullptr);  // Clear parent pointer before deletion
    delete parent;
    delete primative;
    size--;
}

void BVH::refit(Rigid* rigid) {
    if (rigid == nullptr) return;
    
    auto it = primatives.find(rigid);
    if (it == primatives.end()) return;
    
    Primative* primative = it->second;
    glm::vec2 bl, tr;
    rigid->getAABB(bl, tr);
    
    // Check if new AABB (without margin) still fits within old fatted AABB
    if (bl.x >= primative->getBL().x && bl.y >= primative->getBL().y &&
        tr.x <= primative->getTR().x && tr.y <= primative->getTR().y) {
        // Still fits, just refit the hierarchy upward
        primative->refitUpward();
        return;
    }
    
    // Doesn't fit, need to remove and reinsert with new margin
    primative->setBL(bl - BVH_MARGIN);
    primative->setTR(tr + BVH_MARGIN);
    primative->refitUpward();
}

void BVH::rebuild() {
    std::vector<Rigid*> allRigids;
    for (auto& [rigid, _] : primatives) {
        allRigids.push_back(rigid);
    }
    
    // Clear tree
    delete root;
    root = nullptr;
    primatives.clear();
    size = 0;
    
    // Reinsert all
    for (Rigid* rigid : allRigids) {
        insert(rigid);
    }
}

void BVH::refitAll() {
    // Collect all rigids first to avoid iterator invalidation
    // (refit() may call remove/insert which modifies the map)
    std::vector<Rigid*> rigidsToRefit;
    rigidsToRefit.reserve(primatives.size());
    for (auto& [rigid, primative] : primatives) {
        rigidsToRefit.push_back(rigid);
    }
    
    // Now refit each rigid (safe to modify map during this iteration)
    for (Rigid* rigid : rigidsToRefit) {
        // Check if still in map (might have been removed by previous refit)
        if (primatives.find(rigid) != primatives.end()) {
            refit(rigid);
        }
    }
}

std::vector<Rigid*> BVH::query(const glm::vec2& bl, const glm::vec2& tr) const {
    if (root == nullptr) return {};
    std::vector<Rigid*> results;
    root->query(bl, tr, results);
    return results;
}

std::vector<Rigid*> BVH::query(const glm::vec2& point) const {
    if (root == nullptr) return {};
    std::vector<Rigid*> results;
    root->query(point, results);
    return results;
}

std::vector<Rigid*> BVH::query(Rigid* rigid) const {
    if (rigid == nullptr) return {};
    glm::vec2 bl, tr;
    rigid->getAABB(bl, tr);
    return query(bl, tr);
}

std::vector<PrimativeInfo> BVH::getAllPrimatives() const {
    std::vector<PrimativeInfo> results;
    if (root != nullptr) {
        root->getAllPrimatives(results, 0);
    }
    return results;
}

void BVH::computeMassProperties() {
    if (root != nullptr) {
        root->computeMassProperties();
    }
}

glm::vec2 BVH::computeGravity(Rigid* rigid) {
    if (root != nullptr) {
        return root->computeGravity(rigid);
    }
    return glm::vec2(0.0f, 0.0f);
}

}