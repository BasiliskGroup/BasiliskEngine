# include "helper.h"

# include <unordered_set>
# include <array>
# include <cfloat>
# include <iostream>

// structs
struct pair_hash { // TODO review this
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& p) const {
        return std::hash<T1>{}(p.first) ^ std::hash<T2>{}(p.second);
    }
};

// definitions

void get_nearest(
    const std::vector<std::array<glm::vec3, 3>>& polytope, 
    const std::vector<std::array<int, 3>>&       faces, 
    const std::vector<glm::vec3>&                normals, 

    glm::vec3&          normal, 
    float&              distance, 
    std::array<int, 3>& nearest_face
) {
    /*
    Gets the nearest face to the origin and its associated normal
    */
    unsigned int near_index = 0;
    float near_distance     = FLT_MAX;

    for (unsigned int i = 0; i < faces.size(); i++) {

        // calculates the absolute dot product
        float dot = abs(glm::dot(polytope[faces[i][0]][0], normals[i]));

        if (dot < near_distance) {
            near_index    = i;
            near_distance = dot;
        }
    }

    // update normals and faces
    normal       = normals[near_index];
    nearest_face = faces[near_index];
}

glm::vec3 calculate_polytope_normal(
    const std::array<int, 3>&           face, 
    const std::vector<std::array<glm::vec3, 3>>& polytope, 
    const glm::vec3&                    reference_center
) {
    /*
    Gets the normalized normal of a face on the polytope
    */
    glm::vec3 one = polytope[face[0]][0], two = polytope[face[1]][0], three = polytope[face[2]][0], normal = glm::cross(one - two, one - three);

    // takes the average of the three points and points vector towards polytope center
    glm::vec3 average = (one + two + three);
    average /= 3;

    float dot = glm::dot(average - reference_center, normal);

    // if the normal is facing towards the origin, then swap directions
    if (dot < 0)
        normal *= -1;

    return glm::normalize(normal);
}

glm::vec3 get_average_point(
    const std::vector<std::array<glm::vec3, 3>>& polytope
) {
    /*
    Computes the average of every minkowski vector in the polytope
    */
    glm::vec3 total(0.0f, 0.0f, 0.0f);

    // compute the average of the polytope points
    for (const std::array<glm::vec3, 3>& group : polytope) {
        total += group[0];
    }
    total /= polytope.size();

    return total;
}

void update_faces_and_normals(
    const std::vector<std::array<glm::vec3, 3>>& polytope,

    std::vector<std::array<int, 3>>& faces, 
    std::vector<glm::vec3>&          normals
) {
    /*
    Returns new faces and normals of polytope with added point
    Polytope must contain recent support point as last index
    */
    try {
        //std::cout << "starting" << std::endl;

        int sp_index = polytope.size() - 1;
        std::vector<int> visible_indices;

        //std::cout << "visible" << std::endl;

        // gets the indices of the visible points to the sp
        for (int i = normals.size(); i > 0; i--) {

            //std::cout << i << " " << normals.size() << " " << faces.size() << " " << polytope.size() << std::endl;

            // checks if normal is facing in the correct direction
            float point_dot = glm::dot(normals[i], polytope[sp_index][0]);
            if (point_dot < 1e-5)
                continue;

            //std::cout << "normal " << i << std::endl;

            glm::vec3 average = polytope[faces[i][0]][0] + polytope[faces[i][1]][0] + polytope[faces[i][2]][0];
            average /= 3;

            //std::cout << "average " << i << std::endl;

            // checks if the face is pointing in the correct direction
            float face_dot = glm::dot(normals[i], polytope[sp_index][0] - average);
            if (face_dot < 1e-5)
                continue;

            //std::cout << "face " << i << std::endl;

            visible_indices.push_back(i);
        }

        //std::cout << "edges" << std::endl;

        // finds new edges
        std::unordered_set<std::pair<int, int>, pair_hash> edges;
        for (int visible_index: visible_indices) {
            for (int f = 0; f < 3; f++) {
                int low = faces[visible_index][f], high = faces[visible_index][(f + 1) % 3];

                // swap position if incorrect
                if (high < low)
                    std::swap(high, low);

                // generate edges and compare, remove duplicates
                std::pair<int, int> edge = {low, high};
                if (edges.find(edge) != edges.end()) {
                    edges.erase(edge);
                } else {
                    edges.insert(edge);
                }
            }
        }

        //std::cout << "removing" << std::endl;

        // for (int v : visible_indices) 
        //     std::cout << v << std::endl;

        // removes the visible faces and their normals
        for (int visible_index: visible_indices) {
            faces.erase(faces.begin() + visible_index);
            normals.erase(normals.begin() + visible_index);
        }

        // std::cout << "adding" << std::endl;

        // adds new faces
        std::vector<std::array<int, 3>> new_faces;
        for (const std::pair<int, int>& edge : edges) {
            new_faces.push_back({edge.first, edge.second, sp_index});
        }
        faces.insert(faces.end(), new_faces.begin(), new_faces.end());

        // adds new normals
        glm::vec3 average_point = get_average_point(polytope);

        for (const std::array<int, 3>& face: new_faces) {
            normals.push_back(calculate_polytope_normal(face, polytope, average_point));
        }

        // std::cout << "complete" << std::endl;
    } catch (const std::exception& e) {
        std::cout << e.what() << " whoops" << std::endl;
    }
}

void get_epa_from_gjk(
    const std::vector<glm::vec3>& points1, 
    const std::vector<glm::vec3>& points2,

    std::vector<std::array<glm::vec3, 3>>& polytope,
    glm::vec3&                             normal,
    float&                                 distance,
    std::array<int, 3>&                    face
) {

    std::cout << "setting up epa" << std::endl;

    // set up initial faces
    std::vector<std::array<int, 3>> faces = {
        {0, 1, 2},
        {0, 1, 3},
        {0, 2, 3},
        {1, 2, 3}
    };

    // calculate face normals
    glm::vec3 average_point = get_average_point(polytope);
    std::vector<glm::vec3> normals;

    std::cout << "adding normals" << std::endl;

    for (const std::array<int, 3>& face : faces) {
        normals.push_back(calculate_polytope_normal(face, polytope, average_point));
    }

    std::cout << "avergage_point & calculate polytope normal" << std::endl;

    // main loop
    while (true) {

        std::cout << "starting loop" << std::endl;

        // gets the nearest data
        glm::vec3 near_normal;
        float near_distance;
        std::array<int, 3> near_face;

        get_nearest(polytope, faces, normals, near_normal, near_distance, near_face);

        std::cout << "get nearest" << std::endl;

        // gets support point (sp) data
        glm::vec3 minkowski, point1, point2;
        get_support_point(points1, points2, near_normal, minkowski, point1, point2);

        std::cout << "support point" << std::endl;
        
        if (glm::length(minkowski) - near_distance < 0)
            return;


        for (const std::array<glm::vec3, 3>& row : polytope) {
            if (row[0] == minkowski)
                return;
        }

        std::cout << "end of checks" << std::endl;

        // add sp and update polytope geometry
        polytope.push_back({minkowski, point1, point2});
        update_faces_and_normals(polytope, faces, normals);

        std::cout << "faces & normals" << std::endl;
    }
}