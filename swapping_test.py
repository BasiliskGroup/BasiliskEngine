import basilisk as bsk
import glm

engine = bsk.Engine()
scene = bsk.Scene()
engine.scene = scene

node = bsk.Node()

platform = bsk.Node(
    position = (0, -5, 0),
    scale = (10, 1, 10),
    collision = True,
    collision_group='group1'
)

scene.add(node, platform)

def actions():
    
    if engine.keys[bsk.pg.K_p] and not engine.previous_keys[bsk.pg.K_p]:
        node.physics = not node.physics
        
    if engine.keys[bsk.pg.K_c] and not engine.previous_keys[bsk.pg.K_c]:
        node.collision = not node.collision
        
    if engine.keys[bsk.pg.K_s] and not engine.previous_keys[bsk.pg.K_s]:
        match node.static:
            case None: node.static = True
            case True: node.static = False
            case False: node.static = None
            
    if engine.keys[bsk.pg.K_g] and not engine.previous_keys[bsk.pg.K_g]:
        node.collision_group = None if node.collision_group else 'group1'
            
    if engine.keys[bsk.pg.K_l] and not engine.previous_keys[bsk.pg.K_l]:
        print('physics:', node.physics)
        print('collision:', node.collision)
        print('static:', node.static)
            
def death_plane():
    
    if node.y < -20:
        node.velocity = glm.vec3(0)
        node.rotational_velocity = glm.vec3(0)
        node.position = glm.vec3(0)

while engine.running:
    
    actions()
    death_plane()
    
    engine.update()