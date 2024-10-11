mouse_position = glm.vec2([int(self.project.engine.mouse_position[i]) for i in range(2)])

if not self.clicked and self.project.engine.mouse_keys[0]: # if left click
    self.click_anchor = mouse_position
    self.clicked = True
    
    # prevent the camera from rotating until the mouse has been released
    def rotate(): pass
    self.camera.rotate = rotate

elif self.clicked and not self.project.engine.mouse_keys[0]:
    # get the dpos of the mouse 
    self.click_position = mouse_position
    self.clicked = False
    
    # reset the camera rotate function
    self.camera.rotate = self.real_rotate
    pg.mouse.get_rel() # flush the dpos of the mouse from pg
    
    # calculate plane
    print()
    print('screen position: ', self.click_anchor, self.click_position)
    
    # converts the points on screen to vectors projected from the camera
    inv_proj, inv_view = glm.inverse(self.camera.m_proj), glm.inverse(self.camera.m_view)
    vecs = []
    for point in [self.click_anchor, self.click_position]:
        ndc   = glm.vec4(2 * point[0] / self.project.engine.win_size[0] - 1, 1 - 2 * point[1] / self.project.engine.win_size[1], 1, 1)
        point = inv_proj * ndc
        point /= point.w
        point = inv_view * glm.vec4(point.x, point.y, point.z, 0)
        vecs.append(glm.vec3(point))
        
    plane_normal = glm.normalize(glm.cross(vecs[0], vecs[1]))
    print('normal:', plane_normal)
        
    # identify what has been clicked
    node = self.get_model_node_at(*self.click_anchor)
    skeleton = self.skeleton_handler.get_node_skeleton(node)
    
    # sort triangles
    if node is not None:
        
        # easy access variables
        vbo   = self.vao_handler.vbo_handler.vbos[node.model.vbo]
        model = node.model
        
        @staticmethod
        def point_is_above(point:glm.vec3, cam_pos:glm.vec3, plane_normal:glm.vec3) -> bool:
            return glm.dot(point - cam_pos, plane_normal) > 0
        
        @staticmethod
        def orient_triangle(world_points:list[glm.vec3], indices:list[int], origin:glm.vec3) -> list[int]:
            ab, ac = world_points[indices[1]] - world_points[indices[0]], world_points[indices[2]] - world_points[indices[0]]
            if not glm.dot(glm.cross(ab, ac), world_points[indices[0]] - origin): indices[0], indices[2] = indices[2], indices[0]
            return indices
        
        # get the real location of points
        model_matrix = get_model_matrix(model.position, model.scale, model.rotation)
        world_points = []
        for unique_point in vbo.unique_points:
            world_point = model_matrix * ([float(p) for p in unique_point] + [0])
            world_points.append(glm.vec3(world_point[0], world_point[1], world_point[2]))
        
        # sorts triangles on their plane sides
        sorted_triangles = {'below' : [], 'above' : [], 'middle' : []}
        for triangle in vbo.indicies:
            is_above = point_is_above(world_points[triangle[0]], self.camera.position, plane_normal)
            for index in triangle[1:]:
                if point_is_above(world_points[index], self.camera.position, plane_normal) != is_above:
                    sorted_triangles['middle'].append(triangle)
                    break
            else:
                sorted_triangles['above' if is_above else 'below'].append(triangle)
                
        # cuts middle triangles
        trapezoids = {'below' : [], 'above' : []}
        last_index = len(vbo.unique_points) - 1
        edge_index = last_index + 1 # first index of the edge points in the world_points list
        for middle in sorted_triangles['middle']:
            
            # determine edges and points (should always be 2)
            aboves = [point_is_above(world_points[middle[i]], self.camera.position, plane_normal) for i in range(3)]
            for i in range(3):
                if aboves[i] != aboves[(i + 1) % 3]: # if edge crosses plane
                    
                    # find plane intersection
                    vec = (world_points[middle[(i + 1) % 3]] - world_points[middle[i]])
                    t   = glm.dot((self.camera.position - world_points[middle[i]]) / vec, plane_normal)
                    world_points.append(world_points[middle[i]] + t * vec)
                    
            # determine which point is different
            for i in range(3):
                if aboves[i] == aboves[(i + 1) % 3] or aboves[i] == aboves[(i + 2) % 3]: continue
                sorted_triangles['above' if aboves[i] else 'below'].append(orient_triangle(world_points, (middle[i], last_index + 1, last_index + 2), node.position))
                trapezoids['below' if aboves[i] else 'above'].append((middle[(i + 1) % 3], middle[(i + 2) % 3], last_index + 1, last_index + 2))
                
            last_index += 2
        
        # convert edge world points to vbo points
        edge_vbo_points  = []
        inv_model_matrix = glm.inverse(model_matrix)
        for world_point in world_points[edge_index:]:
            vbo_point = inv_model_matrix * ([float(p) for p in world_point] + [0])
            edge_vbo_points.append(glm.vec3(vbo_point[0], vbo_point[1], vbo_point[2]))
        
        print(trapezoids)
    
    # steps
    # sort triangles
    # split middle triangles
    # find cut points
    # convert cut points to vbo
    # reform both sides and save vbos
    
        print('model:', node.model)
        print('node', node)
        print('skeleton:', skeleton)
