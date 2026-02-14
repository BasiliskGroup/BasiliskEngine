#include <basilisk/physics/tables/forceTypeTable.h>
#include <basilisk/physics/tables/forceTable.h>
#include <basilisk/physics/forces/joint.h>
#include <basilisk/physics/forces/manifold.h>
#include <basilisk/physics/forces/motor.h>
#include <basilisk/physics/forces/spring.h>

namespace bsk::internal {

template<typename T>
ForceTypeTable<T>::ForceTypeTable(std::size_t capacity, ForceTable* forceTable) : forceTable(forceTable) {
    resize(capacity);
}

template<typename T>
void ForceTypeTable<T>::markAsDeleted(std::size_t index) {
    toDelete[index] = true;
    forces[index] = nullptr;
}

template<typename T>
void ForceTypeTable<T>::resize(std::size_t newCapacity) {
    if (newCapacity <= capacity) return;
    
    expandTensors(newCapacity,
        forces, toDelete, indexMap, data
    );

    capacity = newCapacity;
}

template<typename T>
void ForceTypeTable<T>::compact() {
    // do a quick check to see if we need to run more complex compact function
    std::size_t active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    compactTensors(toDelete, size,
        forces, indexMap, data
    );

    size = active;

    // TODO check process with remapping
    for (std::size_t i = 0; i < size; i++) {
        toDelete[i] = false;
        forces[i]->setSpecialIndex(i);
    }
}

template<typename T>
void ForceTypeTable<T>::remap() {
    for (std::size_t i = 0; i < size; i++) {
        if (toDelete[i]) {
            continue;
        }

        std::size_t oldForceIndex = indexMap[i];
        if (oldForceIndex >= forceTable->getCapacity()) continue;
        std::size_t newForceIndex = forceTable->getMappedIndex(oldForceIndex);
        if (newForceIndex == SIZE_MAX) continue;  // deleted force (orphaned entry)
        indexMap[i] = newForceIndex;
    }
}

template<typename T>
void ForceTypeTable<T>::insert(Force* force) {
    if (this->size >= this->capacity) {
        resize(this->capacity * 2);
    }

    // set default arguments
    toDelete[size] = false;
    forces[size] = force;
    indexMap[size] = force->getIndex();
    data[size] = T();

    force->setSpecialIndex(size);
    size++;
}

template class ForceTypeTable<ManifoldData>;
template class ForceTypeTable<JointStruct>;
template class ForceTypeTable<SpringStruct>;
template class ForceTypeTable<MotorStruct>;

}