#ifndef BSK_COLLIDER_TABLE_H
#define BSK_COLLIDER_TABLE_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/tables/virtualTable.h>

namespace bsk::internal {

class Collider;

/**
 * @brief Table structure for storing collider data in a structure-of-arrays (SoA) format
 * 
 * ColliderTable stores collider geometry and physics properties in separate arrays for better
 * cache locality and vectorization. Each row represents a single collider with its associated
 * vertices, center of mass, half dimensions, area, and moment of inertia.
 */
class ColliderTable : public VirtualTable {
private:
    // columns 
    std::vector<Collider*> colliders;
    std::vector<bool> toDelete;
    std::vector<std::vector<glm::vec2>> vertices;
    std::vector<glm::vec2> com;
    std::vector<glm::vec2> gc;
    std::vector<glm::vec2> halfDim;
    std::vector<float> area;
    std::vector<float> moment;

public:
    /**
     * @brief Constructs a ColliderTable with the specified initial capacity
     * @param capacity Initial capacity for the internal arrays
     */
    ColliderTable(std::size_t capacity);
    
    /**
     * @brief Destructor that deletes all associated colliders
     */
    ~ColliderTable();

    /**
     * @brief Marks a collider at the given index as deleted
     * 
     * The collider is set to nullptr and marked for deletion. The actual removal
     * happens during compact() which should be called periodically.
     * @param index Index of the collider to mark as deleted
     */
    void markAsDeleted(std::size_t index);

    /**
     * @brief Resizes the internal arrays to accommodate more colliders
     * @param new_capacity New capacity (must be greater than current capacity)
     */
    void resize(std::size_t new_capacity);
    
    /**
     * @brief Compacts the table by removing all marked colliders
     * 
     * This is an expensive operation that should only be called once per frame.
     * Removes all colliders marked for deletion and updates indices.
     */
    void compact();
    
    /**
     * @brief Inserts a new collider into the table
     * 
     * Automatically calculates half dimensions and geometric center.
     * Area and moment of inertia are currently set to placeholder values.
     * @param collider Pointer to the collider object to store
     * @param vertices Vector of 2D vertices representing the collider's shape
     */
    void insert(Collider* collider, const std::vector<glm::vec2>& vertices);

    // Getters
    /**
     * @brief Gets the current number of active colliders in the table
     */
    std::size_t getSize() const { return size; }
    
    /**
     * @brief Gets the current capacity of the internal arrays
     */
    std::size_t getCapacity() const { return capacity; } 

    /**
     * @brief Gets the collider pointer at the specified index
     * @param index Index of the collider
     * @return Pointer to the Collider object, or nullptr if deleted
     */
    Collider* getCollider(std::size_t index) const { return colliders[index]; }
    
    /**
     * @brief Gets the vertices for the collider at the specified index
     * @param index Index of the collider
     * @return Reference to the vector of vertices
     */
    std::vector<glm::vec2>& getVertices(std::size_t index) { return vertices[index]; }
    
    /**
     * @brief Gets the center of mass for the collider at the specified index
     * @param index Index of the collider
     * @return Reference to the center of mass position
     */
    glm::vec2& getCOM(std::size_t index) { return com[index]; }
    
    /**
     * @brief Gets the geometric center (AABB center) for the collider at the specified index
     * @param index Index of the collider
     * @return Reference to the geometric center position
     */
    glm::vec2& getGC(std::size_t index) { return gc[index]; }
    
    /**
     * @brief Gets the half dimensions (half-width, half-height) for the collider at the specified index
     * @param index Index of the collider
     * @return Reference to the half dimensions
     */
    glm::vec2& getHalfDim(std::size_t index) { return halfDim[index]; }
    
    /**
     * @brief Gets the area of the collider at the specified index
     * @param index Index of the collider
     * @return Area value
     */
    float getArea(std::size_t index) const { return area[index]; }
    
    /**
     * @brief Gets the moment of inertia of the collider at the specified index
     * @param index Index of the collider
     * @return Moment of inertia value
     */
    float getMoment(std::size_t index) const { return moment[index]; }

    // Setters
    /**
     * @brief Sets the collider pointer at the specified index
     * @param index Index to set
     * @param collider Pointer to the Collider object
     */
    void setCollider(std::size_t index, Collider* collider) { colliders[index] = collider; }
    
    /**
     * @brief Sets the vertices for the collider at the specified index
     * @param index Index to set
     * @param vertices Vector of 2D vertices
     */
    void setVerts(std::size_t index, const std::vector<glm::vec2>& vertices) { this->vertices[index] = vertices; }
    
    /**
     * @brief Sets the center of mass for the collider at the specified index
     * @param index Index to set
     * @param com Center of mass position
     */
    void setCOM(std::size_t index, const glm::vec2& com) { this->com[index] = com; }
    
    /**
     * @brief Sets the geometric center (AABB center) for the collider at the specified index
     * @param index Index to set
     * @param gc Geometric center position
     */
    void setGC(std::size_t index, const glm::vec2& gc) { this->gc[index] = gc; }
    
    /**
     * @brief Sets the half dimensions for the collider at the specified index
     * @param index Index to set
     * @param halfDim Half dimensions (half-width, half-height)
     */
    void setHalfDim(std::size_t index, const glm::vec2& halfDim) { this->halfDim[index] = halfDim; }
    
    /**
     * @brief Sets the area for the collider at the specified index
     * @param index Index to set
     * @param area Area value
     */
    void setArea(std::size_t index, float area) { this->area[index] = area; }
    
    /**
     * @brief Sets the moment of inertia for the collider at the specified index
     * @param index Index to set
     * @param moment Moment of inertia value
     */
    void setMoment(std::size_t index, float moment) { this->moment[index] = moment; }
};

}

#endif