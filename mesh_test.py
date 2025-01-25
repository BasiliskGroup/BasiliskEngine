import basilisk as bsk
import numpy as np

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

scene.camera.position = (5, 3, 20)

floor_albedo = bsk.Image('tests/floor_albedo.png')
floor_normal = bsk.Image('tests/floor_normal.png')
floor_mtl = bsk.Material(texture=floor_albedo, normal=floor_normal, roughness=.3, specular_tint=.5, specular=.2)

def quad_simple() -> bsk.Mesh:
    """Simple quad from positions"""
    quad = np.array([[0, 0, 0], [0, 0, 10], [10, 0, 10],
                    [0, 0, 0], [10, 0, 10], [10, 0, 0]])
    
    quad = np.array([[0, 0, 0], [0, 0, 10], [10, 0, 10],
                    [0, 0, 0], [10, 0, 10], [10, 0, 0]])
    mesh = bsk.Mesh(quad)
    return mesh

def quad_grouped() -> bsk.Mesh:
    """Simple quad from positions, but with grouped triangle data"""
    quad = np.array([[[0, 0, 0], [0, 0, 10], [10, 0, 10]],
                    [[0, 0, 0], [10, 0, 10], [10, 0, 0]]])
    mesh = bsk.Mesh(quad)
    return mesh

def quad_uv() -> bsk.Mesh:
    """Quad with uv mapping"""
    quad = np.array([[[0, 0, 0, 0, 0], [0, 0, 10, 0, 1], [10, 0, 10, 1, 1]],
                    [[0, 0, 0, 0, 0], [10, 0, 10, 1, 1], [10, 0, 0, 1, 0]]])
    mesh = bsk.Mesh(quad)
    return mesh

def quad_normal() -> bsk.Mesh:
    """Quad with normals"""
    quad = np.array([[[0, 0, 0, 0, 1, 0], [0, 0, 10, 0, 1, 0], [10, 0, 10, 0, 1, 0]],
                    [[0, 0, 0, 0, 1, 0], [10, 0, 10, 0, 1, 0], [10, 0, 0, 0, 1, 0]]])
    mesh = bsk.Mesh(quad)
    return mesh

def quad_uv_normal() -> bsk.Mesh:
    """Quad with normals"""
    quad = np.array([[[0, 0, 0, 0, 0, 0, 1, 0], [0, 0, 10, 0, 1, 0, 1, 0], [10, 0, 10, 1, 1, 0, 1, 0]],
                    [[0, 0, 0, 0, 0, 0, 1, 0], [10, 0, 10, 1, 1, 0, 1, 0], [10, 0, 0, 1, 0, 0, 1, 0]]])
    mesh = bsk.Mesh(quad)
    return mesh

def test_physics() -> None:
    scene.add_node(position=(5, 5, 5), collisions=True, physics=True)

scene.add_node(mesh=quad_uv_normal(), material=floor_mtl, collisions=True)
# test_physics()

while engine.running:
    engine.update()