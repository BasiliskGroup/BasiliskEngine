#ifndef BSK_INSTANCE_HANDLER_H
#define BSK_INSTANCE_HANDLER_H

#include <basilisk/instance/instancer.h>
#include <basilisk/render/shader.h>
#include <basilisk/scene/node.h>

namespace bsk::internal {

template <typename T>
class InstanceHandler<T> {
    private:
        vector<Instancer<T>> instancers;

        Shader* shader;

    public:
        InstanceHandler(Shader* shader, std::vector<std::string> modelFormat, std::vector<std::string> instanceFormat);
        ~InstanceHandler();

        void add(Node* node);
}

}

#endif