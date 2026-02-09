#include <basilisk/physics/tables/forceTypeTable.h>
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
}

template<typename T>
void ForceTypeTable<T>::resize(std::size_t newCapacity) {
    if (newCapacity <= capacity) return;
    
    expandTensors(newCapacity,
        toDelete, indexMap, data
    );

    capacity = newCapacity;
}

template<typename T>
void ForceTypeTable<T>::compact() {
    // do a quick check to see if we need to run more complex compact function
    uint active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    compactTensors(toDelete, size,
        indexMap, data
    );

    size = active;

    // TODO check process with remapping
    for (uint i = 0; i < size; i++) {
        toDelete[i] = false;
    }
}

template<typename T>
void ForceTypeTable<T>::remap() {
    // TODO implement remapping
}

template<typename T>
void ForceTypeTable<T>::insert(Force* force) {
    if (this->size >= this->capacity) {
        resize(this->capacity * 2);
    }

    // set default arguments
    toDelete[size] = false;
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