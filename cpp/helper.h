# ifndef HELPER_H
# define HELPER_H

# include "glm/glm.hpp"

# include <vector>

void get_support_point(
    const std::vector<glm::vec3>& points1, 
    const std::vector<glm::vec3>& points2, 
    const glm::vec3&              direction_vector,

    glm::vec3& minkowski,
    glm::vec3& point1,
    glm::vec3& point2
);

glm::vec3 triple_product(
    const glm::vec3& vec1, 
    const glm::vec3& vec2, 
    const glm::vec3& vec3
);

# endif