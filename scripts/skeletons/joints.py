import glm
from scripts.nodes.node import Node

# child free to move and rotate within radius
class Joint(): 
    def __init__(self, child_bone, parent_offset:glm.vec3, child_offset:glm.vec3, spring_constant:float=1e4): # parent and child not saved for splitting
        # child bone
        self.child_bone = child_bone
        
        # offsets from node position
        self.parent_offset          = glm.vec3(parent_offset)
        self.original_parent_offset = glm.vec3(parent_offset)
        self.child_offset           = glm.vec3(child_offset)
        self.original_child_offset  = glm.vec3(child_offset)
        
        # spring 
        self.spring_constant = spring_constant
        
    def restrict(self, parent:Node, child:Node, delta_time:float):
        """
        Restricts the child to the parent using rk4
        """
        # calculate offset information
        origin       = parent.position + self.parent_offset
        child_point  = child.position + self.child_offset
        displacement = child_point - origin
        
        magnitude = glm.length(displacement)
        if magnitude < 1e-7: return # no movement needed
        
        direction = glm.normalize(displacement)
        
        # if the node has a physics body do this TODO fix rk4 for springs
        if child.physics_body:
               
            # calculate spring force
            force_spring = -self.spring_constant * displacement
            
            # get relative velocity along the spring direction
            relative_velocity = child.physics_body.velocity - (parent.physics_body.velocity if parent.physics_body else glm.vec3(0.0))
            relative_velocity = glm.dot(direction, relative_velocity)
            
            # get the dampening force of the spring
            mu     = (child.physics_body.mass * parent.physics_body.mass / (child.physics_body.mass + parent.physics_body.mass)) if parent.physics_body else child.physics_body.mass
            c      = 2 * glm.sqrt(self.spring_constant * mu)
            dampen = -c * relative_velocity * direction
            
            # applies the spring force
            force_total = (force_spring + dampen) * (0.5 if child.physics_body and parent.physics_body else 1)
            
            if child.physics_body:
                acceleration                 = force_total / child.physics_body.mass
                child.physics_body.velocity += acceleration * delta_time
                
            if parent.physics_body:
                acceleration                  = -force_total / parent.physics_body.mass
                parent.physics_body.velocity += acceleration * delta_time
            
        # snap to position if it does not
        else: 
            child.position = origin + self.child_offset
    
    def rotate_parent_offset(self, rotation:glm.quat):
        """
        Rotate the original parent offset point by the given rotation
        """
        rotated_quat = glm.inverse(rotation) * glm.quat(0, *self.original_parent_offset) * rotation
        self.parent_offset = glm.vec3(rotated_quat.x, rotated_quat.y, rotated_quat.z)
        
    def rotate_child_offset(self, rotation:glm.quat):
        """
        Rotate the original child offset point by the given rotation
        """
        rotated_quat = glm.inverse(rotation) * glm.quat(0, *self.original_child_offset) * rotation
        self.child_offset = glm.vec3(rotated_quat.x, rotated_quat.y, rotated_quat.z)
        
# child free to move within radius, child must point at offset
class BallJoint(Joint):
    def __init__(self, child_bone, parent_offset:glm.vec3, child_offset:glm.vec3, spring_constant:float=1e4):
        super().__init__(child_bone, parent_offset, child_offset, spring_constant)
        self.original_child_rotation = glm.inverse(glm.quat(self.child_bone.node.rotation))
        
    def restrict(self, parent, child, delta_time:float, view_axis:glm.vec3=None):
        super().restrict(parent, child, delta_time)
        
        # set child's rotation to face parent offset
        current_offset = view_axis if view_axis else parent.position + self.parent_offset - child.position
        
        if glm.length(current_offset) < 1e-6: return # if there is no offset from the origin, do nothing
        
        # gets the difference quaternion to get point quaterion from axis angle
        current_forward = self.original_child_rotation * -self.original_parent_offset
        current_offset = glm.normalize(current_offset)
        
        # calculates the axis angle to turn into quaternion
        axis = glm.cross(current_forward, current_offset)
        angle = glm.acos(glm.dot(glm.normalize(current_forward), current_offset))
        
        if glm.length(axis) < 1e-6: axis = glm.vec3(0, 1, 0)
        
        # get new quaternion rotation
        rot = glm.inverse(glm.angleAxis(angle, glm.normalize(axis))) * self.original_child_rotation
        
        # updating rotation at node level with auto update rotation at physics body level
        self.child_bone.node.rotation = glm.eulerAngles(rot)
        if self.child_bone.node.physics_body: self.child_bone.node.physics_body.rotation = rot
    
# child is locked in place but can rotate on given axis TODO change params
class RotatorJoint(BallJoint):
    def __init__(self, child_bone, parent_offset:glm.vec3, child_offset:glm.vec3, spring_constant:float=1e4):
        super().__init__(child_bone, parent_offset, child_offset, spring_constant)
        self.axis = glm.normalize(self.parent_offset)
        self.child_mag = glm.length(self.child_offset)
        
    def rotate_parent_offset(self, rotation:glm.vec3):
        """
        Rotate parent as normal then update axis with parent offset
        """
        super().rotate_parent_offset(rotation)
        self.axis = glm.normalize(self.parent_offset)
        
    def restrict(self, parent, child, delta_time:float):
        """
        Restrict face axis to parent offset then ball joint restrict
        """
        # force restrict child to axis
        self.child_offset = -self.child_mag * self.axis
        
        # apply spring forces 
        super().restrict(parent, child, delta_time, self.child_offset)    
        
# child free to move within radius but can only rotate on given axis TODO change params
class HingeJoint(BallJoint):
    def __init__(self, child_bone, parent_offset:glm.vec3, child_offset:glm.vec3, spring_constant:float=1e4, axis:glm.vec3=None):
        super().__init__(child_bone, parent_offset, child_offset, spring_constant)
        

# child cannot move or be rotated. ex pistons
class PistonJoint(BallJoint):
    def __init__(self, child_bone, parent_offset:glm.vec3, child_offset:glm.vec3, spring_constant:float=1e4):
        super().__init__(child_bone, parent_offset, child_offset, spring_constant)