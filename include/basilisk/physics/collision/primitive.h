#ifndef BSK_PRIMATIVE_H
#define BSK_PRIMATIVE_H

#include <basilisk/util/includes.h>

namespace bsk::internal {

class Rigid;

struct PrimitiveInfo {
    glm::vec2 bl;
    glm::vec2 tr;
    int level;
};

class Primitive {
    private:
        glm::vec2 bl, tr;
        float area;
        Primitive* parent;
        Primitive* left;
        Primitive* right;
        Rigid* rigid;

        // gravity properties
        float mass;
        float radius;
        glm::vec2 com;

        void updateArea();
        void updateBound();
    
    public: 
        Primitive(glm::vec2 bl, glm::vec2 tr, Rigid* rigid);
        Primitive(Primitive* left, Primitive* right);
        ~Primitive();
    
        // getters
        glm::vec2 getBL() const { return bl; }
        glm::vec2 getTR() const { return tr; }
        Primitive* getParent() const { return parent; }
        Primitive* getLeft() const { return left; }
        Primitive* getRight() const { return right; }
        Rigid* getRigid() const { return rigid; }
        float getArea() const { return area; }
        Primitive* getSibling(const Primitive* primative) const;
        Primitive* getSibling() const;
    
        // setters
        void setBL(glm::vec2 bl) { this->bl = bl; }
        void setTR(glm::vec2 tr) { this->tr = tr; }
        void setParent(Primitive* parent) { this->parent = parent; }
        void setLeft(Primitive* left) { this->left = left; }
        void setRight(Primitive* right) { this->right = right; }
    
        // Bounding box operations
        bool intersects(const Primitive& other) const;
        bool contains(const glm::vec2& point) const;
        std::pair<float, Primitive*> findbestSibling(Primitive* primative, float inherited);
        void query(const glm::vec2& bl, const glm::vec2& tr, std::vector<Rigid*>& results) const;
        void query(const glm::vec2& point, std::vector<Rigid*>& results) const;
        
        // Internal tree operations (public for BVH access)
        bool isLeaf() const { return rigid != nullptr; } // has rigid <-> leaf
        void swapChild(Primitive* child, Primitive* newChild);
        void refitUpward();

        // gravity operations
        void computeMassProperties();
        glm::vec2 computeGravity(Rigid* rigid);
        
        // Debug/visualization
        void getAllPrimitives(std::vector<PrimitiveInfo>& results, int level = 0) const;
};

}

#endif

