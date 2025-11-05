#ifndef BSK_BASILISK_H
#define BSK_BASILISK_H

// All headers are now in include/basilisk/ after copying
#include "util/includes.h"
#include "util/print.h"
#include "util/constants.h"
#include "engine/engine.h"
#include "scene/scene.h"
#include "scene/scene2d.h"
#include "IO/window.h"
#include "IO/mouse.h"
#include "IO/keyboard.h"
#include "nodes/node.h"
#include "nodes/node2d.h"
#include "render/mesh.h"
#include "render/texture.h"
#include "render/image.h"
#include "render/shader.h"
#include "render/vao.h"
#include "render/vbo.h"
#include "render/ebo.h"
#include "render/tbo.h"
#include "render/material.h"
#include "resource/materialServer.h"
#include "camera/camera.h"
#include "camera/camera2d.h"

#include "shapes/collider.h"
#include "shapes/rigid.h"
#include "solver/solver.h"
#include "forces/force.h"
#include "forces/manifold.h"

namespace bsk {
    // render
    using Engine = internal::Engine;
    using Scene = internal::Scene;
    using Scene2D = internal::Scene2D;

    // io
    using Window = internal::Window;
    using Mouse = internal::Mouse;
    using Keyboard = internal::Keyboard;

    // node
    using Node = internal::Node;
    using Node2D = internal::Node2D;

    // physics
    using Collider = internal::Collider;
    using Rigid = internal::Rigid;
    using Solver = internal::Solver;
    using Force = internal::Force;
    using Manifold = internal::Manifold;

    // render
    using Mesh = internal::Mesh;
    using Texture = internal::Texture;
    using Image = internal::Image;
    using Shader = internal::Shader;
    using Material = internal::Material;
    using VAO = internal::VAO;
    using VBO = internal::VBO;
    using EBO = internal::EBO;
    using TBO = internal::TBO;
    using Camera = internal::Camera;
    using Camera2D = internal::Camera2D;
    using MaterialServer = internal::MaterialServer;
}

#endif // BASILISK_H