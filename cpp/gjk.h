# ifndef GJK_H
# define GJK_H

# include "glm/glm.hpp"

bool get_gjk_collision(
    const std::vector<glm::vec3>& points1,
    const std::vector<glm::vec3>& points2,
    const glm::vec3&              position1,
    const glm::vec3&              position2,
    
    std::vector<std::array<glm::vec3, 3>>& simplex,

    const int                     iterations = 20
);

# endif