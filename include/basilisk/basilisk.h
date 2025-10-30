#ifndef BASILISK_H
#define BASILISK_H

// necessary includes
#include "includes.h"
#include "print.h"

// basilisk types
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


namespace bsk {
    using Engine = ::Engine;
    using Scene = ::Scene;
    using Scene2D = ::Scene2D;
    using Window = ::Window;
    using Mouse = ::Mouse;
    using Keyboard = ::Keyboard;
    using Node = ::Node;
    using Node2D = ::Node2D;
    using Mesh = ::Mesh;
    using Texture = ::Texture;
    using Image = ::Image;
    using Shader = ::Shader;
    using Material = ::Material;
    using VAO = ::VAO;
    using VBO = ::VBO;
    using EBO = ::EBO;
    using TBO = ::TBO;
    using Camera = ::Camera;
    using Camera2D = ::Camera2D;
    using MaterialServer = ::MaterialServer;
}

#endif