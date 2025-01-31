import glm
from ..nodes.node import Node

def calculate_collisions(normal:glm.vec3, node1: Node, node2: Node, contact_points:list[glm.vec3], inv_inertia1:glm.mat3x3, inv_inertia2:glm.mat3x3, center1:glm.vec3, center2:glm.vec3) -> None:
    """
    Resolve the collisions between two objects with multiple contact points
    """
    physics_body1 = node1.physics_body
    physics_body2 = node2.physics_body
    collider1 = node1.collider
    collider2 = node2.collider
    
    # determine whether or not the colliders have physics
    has_physics1, has_physics2 = physics_body1 is not None, physics_body2 is not None
    
    # get physics data from valid bodies
    if has_physics1: inv_mass1 = 1 / physics_body1.mass
    if has_physics2: inv_mass2 = 1 / physics_body2.mass
    
    # gets coefficients
    elasticity = max(collider1.elasticity, collider2.elasticity)
    kinetic    = min(collider1.kinetic_friction, collider2.kinetic_friction)
    static     = min(collider1.static_friction, collider2.static_friction)
    
    # calculate impulses from contact points
    if has_physics1 and has_physics2:
        for contact_point in contact_points:
            
            # apply impulse based reduced by total points
            radius1, radius2 = contact_point - center1, contact_point - center2
            impulse = calculate_impulse2(node1, node2, inv_mass1, inv_mass2, node1.rotational_velocity, node2.rotational_velocity, radius1, radius2, inv_inertia1, inv_inertia2, elasticity, kinetic, static, normal)
            
            # apply impulses
            apply_impulse(radius1, impulse, inv_inertia1, inv_mass1, node1)
            apply_impulse(radius2, -impulse, inv_inertia2, inv_mass2, node2)
            
    elif has_physics1:
        for contact_point in contact_points:
            radius = contact_point - center1
            impulse = calculate_impluse1(node1, inv_mass1, node1.rotational_velocity, radius, inv_inertia1, elasticity, kinetic, static, normal)
            
            # apply impulses
            apply_impulse(radius, impulse, inv_inertia1, inv_mass1, node1)
            
    else: # only physics body 2
        for contact_point in contact_points:
            radius = contact_point - center2
            impulse = calculate_impluse1(node2, inv_mass2, node2.rotational_velocity, radius, inv_inertia2, elasticity, kinetic, static, normal)
            
            # apply impulse
            apply_impulse(radius, impulse, inv_inertia2, inv_mass2, node2)
    
def calculate_impluse1(node: Node, inv_mass, omega, radius, inv_inertia, elasticity, kinetic, static, normal) -> glm.vec3:
    """
    Calculates the impulse from a collision including friction from the impulse
    """
    # determine if mass needs to be calculated TODO determine if this is a good check
    if glm.dot(radius, node.velocity) < 0: return glm.vec3(0, 0, 0)
    
    # normal impulse
    relative_velocity        = node.velocity + glm.cross(omega, radius)
    relative_normal_velocity = glm.dot(relative_velocity, normal)
    
    # calculate denominator
    denominator = inv_mass + glm.dot(normal, glm.cross(inv_inertia * glm.cross(radius, normal), radius))
    
    # calculate normal impulse
    normal_impulse_magnitude = -(1 + elasticity) * relative_normal_velocity / denominator
    normal_impulse           = normal_impulse_magnitude * normal
    
    # friction impulse
    rel_tan_vel     = relative_velocity - glm.dot(relative_velocity, normal) * normal
    rel_tan_vel_len = glm.length(rel_tan_vel)
    
    if rel_tan_vel_len < 1e-7:   friction_impulse = glm.vec3(0, 0, 0)                                                   # no friction
    elif rel_tan_vel_len < 1e-2: friction_impulse = -static * glm.length(normal_impulse) * glm.normalize(rel_tan_vel)   # static friction
    else:                        friction_impulse = -kinetic * glm.length(normal_impulse) * glm.normalize(rel_tan_vel)  # kinetic friction
    
    # return total impulse
    return normal_impulse + friction_impulse
    
def calculate_impulse2(node1: Node, node2: Node, inv_mass1, inv_mass2, omega1, omega2, radius1, radius2, inv_inertia1, inv_inertia2, elasticity, kinetic, static, normal) -> glm.vec3:
    """
    Calculates the impulse from a collision including friction from the impulse
    """
    # normal impulse
    relative_velocity = node1.velocity + glm.cross(omega1, radius1) - (node2.velocity + glm.cross(omega2, radius2))
    relative_normal_velocity = glm.dot(relative_velocity, normal)
    # calculate denominator
    term1 = inv_mass1 + inv_mass2
    term2 = glm.dot(normal, glm.cross(inv_inertia1 * glm.cross(radius1, normal), radius1) + glm.cross(inv_inertia2 * glm.cross(radius2, normal), radius2))
    # calculate normal impulse
    normal_impulse = -(1 + elasticity) * relative_normal_velocity / (term1 + term2) * normal
    
    # friction impulse
    rel_tan_vel = relative_velocity - glm.dot(relative_velocity, normal) * normal
    rel_tan_vel_len = glm.length(rel_tan_vel)
    if rel_tan_vel_len < 1e-7: friction_impulse = glm.vec3(0, 0, 0)
    elif rel_tan_vel_len < 1e-2: friction_impulse = -static * glm.length(normal_impulse) * glm.normalize(rel_tan_vel)
    else: friction_impulse = -kinetic * glm.length(normal_impulse) * glm.normalize(rel_tan_vel)
    # return total impulse
    return normal_impulse + friction_impulse

def apply_impulse(radius, impulse_signed, inv_inertia, inv_mass, node: Node) -> None:
    """
    Applies the given impulse to the physics body, changing translational and rotational velcoity. 
    """
    
    # Update linear velocity
    node.velocity += impulse_signed * inv_mass
    
    # update rotational velcoity
    node.rotational_velocity += inv_inertia * glm.cross(radius, impulse_signed)