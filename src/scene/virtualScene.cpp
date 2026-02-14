#include <basilisk/scene/virtualScene.h>

namespace bsk::internal {

template<typename NodeType, typename position_type, typename rotation_type, typename scale_type>
void VirtualScene<NodeType, position_type, rotation_type, scale_type>::add(NodeType* node) {
    childrenPythonMap.emplace(node, std::shared_ptr<NodeType>(node));
}

template<typename NodeType, typename position_type, typename rotation_type, typename scale_type>
void VirtualScene<NodeType, position_type, rotation_type, scale_type>::remove(NodeType* node) {
    childrenPythonMap.erase(node);
}

}