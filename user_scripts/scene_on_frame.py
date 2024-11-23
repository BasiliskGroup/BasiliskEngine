from math import sqrt
import random


# level 5
if self.level == 5:
    for node in self.node_handler.nodes:
        if not (node.tags == 'cuttable' and node.collider.vbo == 'cylinder'): continue
        if abs(node.position[0]) < 3:
            target = glm.vec3(node.position)
            target[0] = 0
            x = node.position - target
            v = node.physics_body.velocity

            k = 1.5
            damping = .25
            force = -x * k - v * damping

            node.physics_body.velocity[0] += force[0]

# grabbing
min_grab_distance = 35

if self.project.engine.mouse_buttons[2] and not self.grabbed: # getting object
    point = glm.vec2(self.project.engine.win_size) / 2
    node = self.get_model_node_at(*point, has_collider = True, has_physics_body = True, tags = 'cuttable')
    if node:
        inv_proj, inv_view = glm.inverse(self.camera.m_proj), glm.inverse(self.camera.m_view)
        ndc   = glm.vec4(2 * point[0] / self.project.engine.win_size[0] - 1, 1 - 2 * point[1] / self.project.engine.win_size[1], 1, 1)
        point = inv_proj * ndc
        point /= point.w
        
        self.grabbed = node
        center_vec = node.position - self.camera.position
        self.grab_distance = glm.length(center_vec)

elif self.project.engine.mouse_buttons[2]: # moving object
    point = glm.vec2(self.project.engine.win_size) / 2
    node = self.get_model_node_at(*point, has_collider = True, has_physics_body = True, tags = 'cuttable')

else: # releasing object
    self.grabbed = None
    self.grab_distance = 0

if self.grabbed:
    self.grab_distance = max(self.grab_distance, min_grab_distance)
    target_position = self.camera.position + glm.normalize(self.camera.forward) * self.grab_distance
    x = self.grabbed.position - target_position
    v = self.grabbed.physics_body.velocity

    k = 1.5
    damping = .25
    force = -x * k - v * damping

    self.grabbed.physics_body.velocity += force

# cutting
if not self.clicked and self.project.engine.mouse_buttons[0]: # if left click
    window = self.project.engine.win_size
    self.click_anchor = (window[0] / 2, window[1] / 2)
    self.project.engine.mouse_position = self.click_anchor
    pg.mouse.set_pos(self.click_anchor)
    pg.mouse.get_rel()
    self.clicked = True
    
    # prevent the camera from rotating until the mouse has been released
    def rotate(): pass
    self.camera.rotate = rotate

elif self.clicked and not self.project.engine.mouse_buttons[0]:
    # get the dpos of the mouse 
    mouse_position = self.project.engine.mouse_position
    mouse_position = glm.vec2([int(i) for i in mouse_position])
    self.click_position = mouse_position
    self.clicked = False
    
    # reset the camera rotate function
    self.camera.rotate = self.real_rotate
    pg.mouse.get_rel() # flush the dpos of the mouse from pg
    
    if glm.length(self.click_anchor - self.click_position) > 1e-3:
        
        # converts the points on screen to vectors projected from the camera
        inv_proj, inv_view = glm.inverse(self.camera.m_proj), glm.inverse(self.camera.m_view)
        diff = (glm.vec2(self.click_position) - glm.vec2(self.click_anchor)) / 25

        # points = [self_click_anchor + diff * i for i in range(26)]
        points = []
        for i in range(26):
            points.append(self.click_anchor + diff * i)

        vecs = []
        for point in points:
            ndc   = glm.vec4(2 * point[0] / self.project.engine.win_size[0] - 1, 1 - 2 * point[1] / self.project.engine.win_size[1], 1, 1)
            point = inv_proj * ndc
            point /= point.w
            point = inv_view * glm.vec4(point.x, point.y, point.z, 0)
            vecs.append(glm.vec3(point))
            
        plane_normal = glm.normalize(glm.cross(vecs[0], vecs[25]))
            
        # identify what has been clicked
        nodes = set()
        for point in points:
            node = self.get_model_node_at(*point, has_collider = True)
            skeleton = self.skeleton_handler.get_node_skeleton(node)
            if node and node.tags == 'cuttable': nodes.add(node)
        
        # sort triangles
        for node in nodes:
            
            # easy access variables
            vbo   = self.vao_handler.vbo_handler.vbos[node.model.vbo]
            model = node.model
            
            @staticmethod
            def point_is_above(point:glm.vec3, cam_pos:glm.vec3, plane_normal:glm.vec3) -> bool:
                return glm.dot(point - cam_pos, plane_normal) > 0
            
            # get the real location of points
            model_matrix = get_model_matrix(model.position, model.scale, model.rotation)
            world_points = []
            ary = vbo.unique_points
            for unique_point in ary:
                world_point = model_matrix * glm.vec4(*unique_point, 1)
                world_points.append(glm.vec3(world_point))
            
            # sorts triangles on their plane sides
            sorted_triangles = {'below' : [], 'above' : [], 'middle' : []}
            for triangle in vbo.indicies:
                count = 0
                for i in triangle: count += point_is_above(world_points[i], self.camera.position, plane_normal)
                if not count:    sorted_triangles['below'].append(list(triangle))
                elif count == 3: sorted_triangles['above'].append(list(triangle))
                else:            sorted_triangles['middle'].append(list(triangle))
                    
            # cuts middle triangles
            trapezoids = {'below' : [], 'above' : []}
            last_index = len(vbo.unique_points)
            edge_index = last_index # first index of the edge points in the world_points list
            for middle in sorted_triangles['middle']:
                
                # determine edges and points (should always be 2)
                # aboves = [point_is_above(world_points[middle[i]], self.camera.position, plane_normal) for i in range(3)]
                aboves = []
                for i in range(3): aboves.append(point_is_above(world_points[middle[i]], self.camera.position, plane_normal))
                cut_indices = []
                for i in range(3):
                    if aboves[i] == aboves[(i + 1) % 3]: continue # if edge crosses plane
                        
                    # find plane intersection
                    vec = world_points[middle[(i + 1) % 3]] - world_points[middle[i]]
                    den = glm.dot(vec, plane_normal)
                    t   = glm.dot(self.camera.position - world_points[middle[i]], plane_normal) / den
                    intersection = world_points[middle[i]] + t * vec
                    
                    # determine if intersection has already been added
                    for wp_index, wp in enumerate(world_points[edge_index:]):
                        #if all([abs(wp[i] - intersection[i]) < 1e-3 for i in range(3)]):  # if points are close enough
                        if abs(wp[0] - intersection[0]) < 1e-3 and abs(wp[1] - intersection[1]) < 1e-3 and abs(wp[2] - intersection[2]) < 1e-3:
                            cut_indices.append(wp_index + edge_index)
                            break
                    else: 
                        world_points.append(intersection)
                        cut_indices.append(last_index)
                        last_index += 1
                
#     def add_particles(self, type=2, life=1.0, pos=(0, 0, 0), clr=(1.0, 1.0, 1.0), scale=1.0, vel=(0, 3, 0), accel=(0, -10, 0)):
                cut_index_3d = [world_points[w] for w in cut_indices]
                particle_dir = (cut_index_3d[1] - cut_index_3d[0]) / 10
                for w in range(11):
                    self.particle_handler.add_particles(type=1, life=0.3, pos=cut_index_3d[0] + particle_dir * w, clr=(0.9 + random.uniform(-0.1, 0.1), 0.2 + random.uniform(-0.1, 0.1), 0.2 + random.uniform(-0.1, 0.1)), vel=[random.uniform(-2, 2) for _ in range(3)], accel=(0, 0, 0))
                    self.particle_handler.add_particles(type=2, life=0.3, pos=cut_index_3d[0] + particle_dir * w, clr=(0.9 + random.uniform(-0.1, 0.1), 0.2 + random.uniform(-0.1, 0.1), 0.2 + random.uniform(-0.1, 0.1)), vel=[random.uniform(-2, 2) for _ in range(3)], accel=(0, 0, 0))
                    grey_off = random.uniform(-0.1, 0.1)
                    self.particle_handler.add_particles(type=2, life=0.3, pos=cut_index_3d[0] + particle_dir * w, clr=(0.7 + grey_off, 0.7 + grey_off, 0.7 + grey_off), vel=[random.uniform(-2, 2) for _ in range(3)], accel=(0, 0, 0))
                        
                # determine which point is different
                for i in range(3):
                    if aboves[i] == aboves[(i + 1) % 3] or aboves[i] == aboves[(i + 2) % 3]: continue
                    sorted_triangles['above' if aboves[i] else 'below'].append([middle[i], cut_indices[0], cut_indices[1]])
                    trapezoids['below' if aboves[i] else 'above'].append((middle[(i + 1) % 3], middle[(i + 2) % 3], cut_indices[0], cut_indices[1]))
            
            # fill in center
            # gets linearly independent vector and generate basis
            indep_vector = glm.vec3(1, 1, 1)
            if glm.length(glm.cross(plane_normal, indep_vector)) < 1e-6: indep_vector = glm.vec3(1, 1, 0)
            u = glm.normalize(glm.cross(plane_normal, indep_vector))
            v = glm.normalize(glm.cross(plane_normal, u))
            
            # # convert cut points to 2d
            cut_points_2d = []
            for point in world_points[edge_index:]:
                vec    = point - self.camera.position
                u_comp = glm.dot(vec, u)
                v_comp = glm.dot(vec, v)
                cut_points_2d.append(Point(u_comp, v_comp))
            
            # add interior triangles to triangle lists
            triangles = delunay_triangulation(cut_points_2d)
            for i, triangle in enumerate(triangles):
                one = cut_points_2d.index(triangle.p1)
                two = cut_points_2d.index(triangle.p2)
                thr = cut_points_2d.index(triangle.p3)
                
                triangles[i] = [one + edge_index, two + edge_index, thr + edge_index]
                
            for key in ['above', 'below']: 
                
                # add interior triangles
                sorted_triangles[key].extend(triangles) # TODO may cause memory erros
                
                # add trapezoid triangles
                for trapezoid in trapezoids[key]: 
                    t = []
                    for i in trapezoid:
                        t.append(world_points[i])
                    #t = [world_points[i] for i in trapezoid]
                    
                    center = (t[0] + t[1] + t[2] + t[3]) / 4
                    c1 = glm.cross(t[2] - center, t[3] - center)
                    c2 = glm.cross(t[0] - center, t[1] - center)
                    
                    if glm.dot(c1, c2) > 0:
                        sorted_triangles[key].append([trapezoid[0], trapezoid[1], trapezoid[2]])
                        sorted_triangles[key].append([trapezoid[0], trapezoid[3], trapezoid[2]])
                    else:
                        sorted_triangles[key].append([trapezoid[0], trapezoid[1], trapezoid[2]])
                        sorted_triangles[key].append([trapezoid[1], trapezoid[2], trapezoid[3]])
            
            # deep copy above and below triangle lists
            for key in ['above', 'below']:
                for i, triangle in enumerate(sorted_triangles[key]):
                    sorted_triangles[key][i] = [*triangle]
                    
            for key in ['above', 'below']:
                for i, triangle in enumerate(sorted_triangles[key]):
                    for t in triangle:
                        assert t < len(world_points), 't index out of bounds'
            
            # prune unused points from each vbo
            unique_points = {'above' : world_points[:], 'below' : world_points[:]}
            for key in unique_points.keys():
                index = 0
                while index < len(unique_points[key]):
                    exists = False
                    
                    for triangle in sorted_triangles[key]:
                        #if any([index == triangle[i] for i in range(3)]):
                        if index == triangle[0] or index == triangle[1] or index == triangle[2]:
                            exists = True
                            break
                    
                    if not exists:
                        unique_points[key].pop(index)
                        for i, triangle in enumerate(sorted_triangles[key]):
                            for j in range(3):
                                if triangle[j] >= index: sorted_triangles[key][i][j] -= 1
                        index -= 1
                    index += 1
            
            # generate vbos and create new nodes
            inv_model_matrix = glm.inverse(model_matrix)
            for key in unique_points.keys():
                
                # get difference in geometric centers
                maximum = glm.max(unique_points[key])
                minimum = glm.min(unique_points[key])
                center  = (maximum + minimum) / 2
                vbo_center = glm.vec3(inv_model_matrix * glm.vec4(*center, 1))
                
                # convert to vbo space
                for i, point in enumerate(unique_points[key]): unique_points[key][i] = glm.vec3(inv_model_matrix * glm.vec4(*point, 1)) - vbo_center
                
                world_center = glm.vec3(0, 0, 0)
                for point in unique_points[key]: world_center += point
                world_center /= len(unique_points[key])
                
                # orient sides
                for i, triangle in enumerate(sorted_triangles[key]):
                    if glm.dot(glm.cross(unique_points[key][triangle[1]] - world_center, unique_points[key][triangle[2]] - world_center), unique_points[key][triangle[0]] - world_center) < 0: 
                        sorted_triangles[key][i] = [triangle[2], triangle[1], triangle[0]]
                
                # generate node
                uuid = self.vao_handler.vbo_handler.create_vbo(unique_points[key], sorted_triangles[key])
                
                # split node
                self.node_handler.add(
                    position=center,
                    scale=node.scale,
                    rotation=node.rotation,
                    material=node.material,
                    collider=self.collider_handler.add(vbo=uuid, static=False),
                    physics_body=self.physics_body_handler.add(mass=node.physics_body.mass), # TODO get split mass (estimate)
                    model=uuid, 
                )
                self.node_handler.nodes[-1].tags = 'cuttable'
                
            # TODO split skeleton
            
            # remove split node
            self.node_handler.remove(node)
