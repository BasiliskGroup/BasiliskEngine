import glm
from .helper import is_ccw_turn
from .line_intersections import line_line_intersect

def sutherland_hodgman(subject:list[glm.vec2], clip:list[glm.vec2]) -> list[glm.vec2]:
    """determines the clipped polygon vertices from ccw oriented polygons"""
    output_poly = subject
    
    for i in range(len(clip)):
        input_poly = output_poly
        output_poly = []
        
        edge_start, edge_end = clip[i], clip[(i + 1) % len(clip)]
        for j in range(len(input_poly)):
            prev_point, curr_point = input_poly[j - 1], input_poly[j]

            if is_ccw_turn(curr_point, edge_start, edge_end):
                if not is_ccw_turn(prev_point, edge_start, edge_end):
                    output_poly += line_line_intersect([edge_end, edge_start], [prev_point, curr_point])
                output_poly.append(curr_point)
            elif is_ccw_turn(prev_point, edge_start, edge_end):
                output_poly += line_line_intersect([edge_end, edge_start], [prev_point, curr_point])
                
    return output_poly

# def get_intersect(one: glm.vec2, two: glm.vec2, thr: glm.vec2, fou: glm.vec2) -> glm.vec2:
#     """
#     Gets the intersection point between two lines
#     """
#     deno = (one.x - two.x) * (thr.y - fou.y) - (one.y - two.y) * (thr.x - fou.x)
#     if deno == 0: # TODO determine if this happens
#         print('sutherland-hodgman line intersection had zero denominator')
#         return None
#     x_num = (one.x * two.y - one.y * two.x) * (thr.x - fou.x) - (one.x - two.x) * (thr.x * fou.y - thr.y * fou.x)
#     y_num = (one.x * two.y - one.y * two.x) * (thr.y - fou.y) - (one.y - two.y) * (thr.x * fou.y - thr.y * fou.x)
#     return glm.vec2(x_num / deno, y_num / deno)

# def clip(poly: list[glm.vec2], one: glm.vec2, two: glm.vec2) -> list[glm.vec2]:
#     """
#     Clip all edges of polygon with one of the clipping edges
#     """
#     num_points = len(poly)
#     new_points = []
    
#     for i in range(num_points):
#         k    = (i + 1) % num_points
#         veci = poly[i]
#         veck = poly[k]
        
#         posi = (two.x - one.x) * (veci.y - one.y) - (two.y - one.y) * (veci.x - one.x)
#         posk = (two.x - one.x) * (veck.y - one.y) - (two.y - one.y) * (veck.x - one.x)
        
#         if posi < 0 and posk < 0: new_points.append(veck)
#         elif posi >= 0 and posk < 0: 
            
#             new_points.append(get_intersect(one, two, veci, veck))
#             new_points.append(veck)
            
#         elif posi < 0 and posk >= 0:
            
#             new_points.append(get_intersect(one, two, veci, veck))
        
#     return new_points

# def sutherland_hodgman(subj_poly:list[glm.vec2], clip_poly:list[glm.vec2]) -> list[glm.vec2]:
#     """
#     Determines the clipped polygon vertices from ccw oriented polygons.
#     """
#     num_clip = len(clip_poly)
    
#     for i in range(num_clip):
#         k = (i + 1) % num_clip
        
#         subj_poly = clip(subj_poly, clip_poly[i], clip_poly[k])
        
#     return subj_poly