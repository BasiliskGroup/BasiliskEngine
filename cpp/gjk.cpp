# include "helper.h"

# include <stdexcept>
# include <iostream>

// definitions
bool handle_simplex_line(
    const std::vector<std::array<glm::vec3, 3>>& simplex,
    glm::vec3& direction
) {
    /*
    Determines the direction for simplex generation if a 2 point simplex.
    */
    glm::vec3 vec_ab = simplex[1][0] - simplex[0][0];
    direction = triple_product(vec_ab, -simplex[0][0], vec_ab);

    return false;
}

bool handle_simplex_triangle(
    const std::vector<std::array<glm::vec3, 3>>& simplex,
    glm::vec3& direction
) {
    /*
    Determines the direction for simplex generation if a 3 point simplex.
    */
    direction = glm::cross(simplex[1][0] - simplex[0][0], simplex[2][0] - simplex[0][0]);
    if (glm::dot(direction, -simplex[0][0]))
        direction *= -1;
    
    return false;
}

bool handle_simplex_tetra(
    std::vector<std::array<glm::vec3, 3>>& simplex,
    glm::vec3& direction,
    const float epsilon = 1e4
) {
    /*
    Determines if a collision has occurred, if not return the next direction
    */
    glm::vec3 vec_da = simplex[3][0] - simplex[0][0];
    glm::vec3 vec_db = simplex[3][0] - simplex[1][0];
    glm::vec3 vec_dc = simplex[3][0] - simplex[2][0];
    glm::vec3 vec_do = -simplex[3][0];

    // check all sides of the simplex TODO check order for speed
    glm::vec3 normal = glm::cross(vec_da, vec_db);
    if (glm::dot(normal, vec_do) > epsilon) {
        simplex.erase(simplex.begin() + 2);
        direction = normal;
        return false;
    }

    normal = glm::cross(vec_dc, vec_da);
    if (glm::dot(normal, vec_do) > epsilon) {
        simplex.erase(simplex.begin() + 1);
        direction = normal;
        return false;
    }

    normal = glm::cross(vec_db, vec_dc);
    if (glm::dot(normal, vec_do) > epsilon) {
        simplex.erase(simplex.begin());
        direction = normal;
        return false;
    }

    return true;
}

bool handle_simplex(
    std::vector<std::array<glm::vec3, 3>>& simplex,

    glm::vec3& direction
) {
    /*
    Controls the simplex operations based on the amount of points in the simplex
    */
    switch (simplex.size()) {
        case(2): return handle_simplex_line(simplex, direction);
        case(3): return handle_simplex_triangle(simplex, direction);
        case(4): return handle_simplex_tetra(simplex, direction);
        default: throw std::invalid_argument("simplex recieved an unsupported size");
    }
}

bool get_gjk_collision(
    const std::vector<glm::vec3>& points1,
    const std::vector<glm::vec3>& points2,
    const glm::vec3&              position1,
    const glm::vec3&              position2,
    
    std::vector<std::array<glm::vec3, 3>>& simplex,

    const int iterations = 20
) {
    /*
    Determines if a collision has occured and outputs the simplex
    */
    // gets starting values for the simplex and direction
    glm::vec3 direction = position1 - position2, minkowski, point1, point2;

    get_support_point(points1, points2, direction, minkowski, point1, point2);
    simplex.push_back({minkowski, point1, point2});

    // points direction vector to the origin
    direction = -simplex[0][0];

    for (int _ = 0; _ < iterations; _++) { // Is used as a while loop but with limited uses

        // gets next support point and determines if it has crossed the origin
        get_support_point(points1, points2, direction, minkowski, point1, point2);
        if (glm::dot(minkowski, direction) < 0)
            return false;
        
        // if it is successful handle the simplex
        simplex.push_back({minkowski, point1, point2});

        std::cout << simplex.size() << std::endl;

        bool has_collided = handle_simplex(simplex, direction);

        if (has_collided) {
            std::cout << simplex.size() << std::endl;
            return true;
        }
    }
    return false;
}