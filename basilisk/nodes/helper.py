import glm
# from .node import Node
from ..mesh.mesh import Mesh
from ..render.material import Material

def node_is(node, position: glm.vec3=None, scale: glm.vec3=None, rotation: glm.quat=None, forward: glm.vec3=None, mesh: Mesh=None, material: Material=None, velocity: glm.vec3=None, rotational_velocity: glm.quat=None, physics: bool=None, mass: float=None, collisions: bool=None, static_friction: float=None, kinetic_friction: float=None, elasticity: float=None, collision_group: float=None, name: str=None, tags: list[str]=None,static: bool=None) -> bool:
    """
    Determine if a node meets the requirements given by the parameters. If a parameter is None, then the filter is not applied.
    """
    return all([
            position is None or position == node.position,
            scale    is None or scale    == node.scale,
            rotation is None or rotation == node.rotation,
            forward  is None or forward  == node.forward,
            mesh     is None or mesh     == node.mesh,
            material is None or material == node.material,
            velocity is None or velocity == node.velocity,
            rotational_velocity is None or rotational_velocity == node.rotational_velocity,
            physics    is None or bool(node.physics_body) == physics,
            mass       is None or (node.physics_body and mass == node.physics_body.mass),
            collisions is None or bool(node.collider) == collisions,
            static_friction  is None or (node.collider and node.collider.static_friction  == static_friction),
            kinetic_friction is None or (node.collider and node.collider.kinetic_friction == kinetic_friction),
            elasticity       is None or (node.collider and node.collider.elasticity       == elasticity),
            collision_group  is None or (node.collider and node.collider.collision_group  == collision_group),
            name   is None or node.name == name,
            tags   is None or all([tag in node.tags for tag in tags]),
            static is None or node.static == static
        ])