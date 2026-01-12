#ifndef BSK_PRIMATIVE_H
#define BSK_PRIMATIVE_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Rigid;

struct PrimativeInfo {
    glm::vec2 bl;
    glm::vec2 tr;
    int level;
};

class Primative {
    private:
        glm::vec2 bl, tr;
        float area;
        Primative* parent;
        Primative* left;
        Primative* right;
        Rigid* rigid;

        // gravity properties
        float mass;
        float radius;
        glm::vec2 com;

        void updateArea();
        void updateBound();
    
    public: 
        Primative(glm::vec2 bl, glm::vec2 tr, Rigid* rigid);
        Primative(Primative* left, Primative* right);
        ~Primative();
    
        // getters
        glm::vec2 getBL() const { return bl; }
        glm::vec2 getTR() const { return tr; }
        Primative* getParent() const { return parent; }
        Primative* getLeft() const { return left; }
        Primative* getRight() const { return right; }
        Rigid* getRigid() const { return rigid; }
        float getArea() const { return area; }
        Primative* getSibling(const Primative* primative) const;
        Primative* getSibling() const;
    
        // setters
        void setBL(glm::vec2 bl) { this->bl = bl; }
        void setTR(glm::vec2 tr) { this->tr = tr; }
        void setParent(Primative* parent) { this->parent = parent; }
        void setLeft(Primative* left) { this->left = left; }
        void setRight(Primative* right) { this->right = right; }
    
        // Bounding box operations
        bool intersects(const Primative& other) const;
        bool contains(const glm::vec2& point) const;
        std::pair<float, Primative*> findbestSibling(Primative* primative, float inherited);
        void query(const glm::vec2& bl, const glm::vec2& tr, std::vector<Rigid*>& results) const;
        void query(const glm::vec2& point, std::vector<Rigid*>& results) const;
        
        // Internal tree operations (public for BVH access)
        bool isLeaf() const { return rigid != nullptr; } // has rigid <-> leaf
        void swapChild(Primative* child, Primative* newChild);
        void refitUpward();

        // gravity operations
        void computeMassProperties();
        glm::vec2 computeGravity(Rigid* rigid);
        
        // Debug/visualization
        void getAllPrimatives(std::vector<PrimativeInfo>& results, int level = 0) const;
};

}

#endif

