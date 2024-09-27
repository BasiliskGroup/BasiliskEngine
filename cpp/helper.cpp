# include "helper.h"

glm::vec3 get_furthest_point(
    const std::vector<glm::vec3>& points, 
    const glm::vec3&              direction_vector
) {
    /*
    Gets the point with the highest dot product with the given vector
    */
    glm::vec3 far(0.0f, 0.0f, 0.0f);
    float far_dot = -FLT_MIN;

    for (const glm::vec3& point : points) {

        // test if current point has higher dot product
        float test_dot = glm::dot(direction_vector, point);

        if (test_dot > far_dot) {
            far     = point;
            far_dot = test_dot;
        }
    }

    return far;
}

void get_support_point(
    const std::vector<glm::vec3>& points1, 
    const std::vector<glm::vec3>& points2, 
    const glm::vec3&              direction_vector,

    glm::vec3& minkowski,
    glm::vec3& point1,
    glm::vec3& point2
) {
    /*
    Gets the furthest point distance based on the direction vector
    */
    point1 = get_furthest_point(points1, direction_vector);
    point2 = get_furthest_point(points2, -direction_vector);

    minkowski = point1 - point2;
}

glm::vec3 triple_product(
    const glm::vec3& vec1, 
    const glm::vec3& vec2, 
    const glm::vec3& vec3
) {
    return glm::cross(glm::cross(vec1, vec2), vec3);
}