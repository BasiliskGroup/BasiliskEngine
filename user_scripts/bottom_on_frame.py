import sys

# movement
velocity = 20 * delta_time
keys = self.node_handler.scene.engine.keys
self.jump_time += delta_time

dpos = glm.vec3(self.nodes[0].camera.forward.x, 0, self.nodes[0].camera.forward.z) * (keys[pg.K_w] - keys[pg.K_s]) + self.nodes[0].camera.right * (keys[pg.K_d] - keys[pg.K_a])
if glm.length(dpos) > 0: dpos = glm.normalize(dpos)
self.position += dpos * velocity

# jump mechanic
if keys[pg.K_SPACE] and self.jump_time > self.jump_max and abs(self.physics_body.velocity[1] < 1):
    can_jump = False
    found = None
    for normal in self.collider.collision_normals.values():
        found = normal
        if glm.dot(glm.normalize(normal), (0, 1, 0)) > 0.1:
            can_jump = True
            break
    if can_jump: 
        self.position += glm.normalize(found) * 0.25
        self.physics_body.velocity[1] = 16
        self.jump_time = 0

if keys[pg.K_w] or keys[pg.K_s] or keys[pg.K_a] or keys[pg.K_d] or keys[pg.K_SPACE]:
    self.physics_body.rotation = glm.angleAxis(glm.radians(self.nodes[0].camera.yaw - 90), self.nodes[0].camera.UP)
    self.physics_body.rotational_velocity = 0
    self.saved_rotation = self.physics_body.rotation
else: 
    self.physics_body.rotation = glm.quat(self.saved_rotation)
    
# level switching
for node, collider in self.collider.collision_normals.items():
    if node.tags != 'exit': continue
    self.node_handler.scene.level += 1
    self.node_handler.scene.audio_handler.play_sound_group('step')
    load_scene(self.node_handler.scene, f'room{self.node_handler.scene.level}')
    self.node_handler.scene.add_john()
    
if keys[pg.K_r]: 
    load_scene(self.node_handler.scene, f'room{self.node_handler.scene.level}')
    self.node_handler.scene.add_john()
    
# level 5
if self.node_handler.scene.level == 5:
    for node in self.collider.collision_normals.keys(): # lava check
        if node.tags != 'lava': continue
        load_scene(self.node_handler.scene, f'room{self.node_handler.scene.level}')
        self.node_handler.scene.add_john()
        
    for node in self.collider.collision_normals.keys(): # DOI check
        if node.tags != 'declaration': continue
        self.node_handler.scene.cutscene_handler.play_cutscene("outro")
        sys.exit()