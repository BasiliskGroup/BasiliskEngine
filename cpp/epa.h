# ifndef EPA_H
# define EPA_H

# include "glm/glm.hpp"

void get_epa_from_gjk(
    const std::vector<glm::vec3>& points1, 
    const std::vector<glm::vec3>& points2,

    std::vector<std::array<glm::vec3, 3>>& polytope,
    glm::vec3&                             normal,
    float&                                 distance,
    std::array<int, 3>&                    face
);

# endif