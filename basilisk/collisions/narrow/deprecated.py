    # def sat_manifold(self, points1: list[glm.vec3], points2: list[glm.vec3], axis: glm.vec3, plane_point: glm.vec3, digit: int) -> list[glm.vec3]:
    #     """
    #     Returns the contact manifold from an SAT OBB OBB collision
    #     """
    #     def get_test_points(contact_plane_normal:glm.vec3, points:list[glm.vec3], count: int):
    #         test_points = [(glm.dot(contact_plane_normal, p), p) for p in points]
    #         test_points.sort(key=lambda p: p[0])
    #         return [p[1] for p in test_points[:count]]
        
    #     def get_test_points_unknown(contact_plane_normal:glm.vec3, points:list[glm.vec3]):
    #         test_points = [(glm.dot(contact_plane_normal, p), p) for p in points]
    #         test_points.sort(key=lambda p: p[0])
    #         if test_points[2][0] - test_points[0][0] > 1e-3: return [p[1] for p in test_points[:2]]
    #         else:                                            return [p[1] for p in test_points[:4]]        
        
    #     if digit < 6: # there must be at least one face in the collision
    #         reference, incident = (get_test_points(-axis, points1, 4), get_test_points_unknown(axis, points2)) if digit < 3 else (get_test_points(axis, points2, 4), get_test_points_unknown(-axis, points1))
            
    #         # project vertices onto the 2d plane
    #         reference = project_points(plane_point, axis, reference)
    #         incident  = project_points(plane_point, axis, incident)
            
    #         # convert points to 2d for intersection algorithms
    #         reference, u1, v1 = points_to_2d(plane_point, axis, reference)
    #         incident,  u2, v2 = points_to_2d(plane_point, axis, incident, u1, v1)
            
    #         # convert arbitrary points to polygon
    #         reference = graham_scan(reference)
    #         if len(incident) == 4:  incident =  graham_scan(incident)
            
    #         # run clipping algorithms
    #         manifold = []
    #         if len(incident) == 2: manifold = line_poly_intersect(incident, reference)
    #         else:                  manifold = sutherland_hodgman(reference, incident)
                
    #         # # fall back if manifold fails to develope
    #         assert len(manifold), 'sat did not generate points'
            
    #         # # convert inertsection algorithm output to 3d
    #         return points_to_3d(u1, v1, plane_point, manifold)
        
    #     else: # there is an edge edge collision
            
    #         points1 = get_test_points(-axis, points1, 2)
    #         points2 = get_test_points(axis, points2, 2)
            
    #         return closest_two_lines(*points1, *points2)