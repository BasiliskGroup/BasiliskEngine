import glm
from dataclasses import dataclass

from basilisk.generic.vec3 import Vec3
# from ...nodes.node import Node

# frozen because data does not need to be mutable
# used in creating polytopes for GJK/EPA
@dataclass(frozen=True)
class SupportPoint():
    support_point: glm.vec3
    
    index1: int # index of the vertex in the mesh
    vertex1: glm.vec3 # world space location of the vertex at collision
    
    index2: int
    vertex2: glm.vec3
    
# used for generating contact points for the contact manifold
@dataclass(frozen=True)
class ContactPoint():
    index: int
    vertex: Vec3
    
# contact manifold object used in the contact handler list
@dataclass
class ContactManifold():
    normal: glm.vec3
    contact_points1: dict[int : glm.vec3] # contact point index : collision position
    contact_points2: dict[int : glm.vec3]
    
@dataclass
class Collision():
    node: ...
    normal: glm.vec3