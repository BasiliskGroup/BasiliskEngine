# include "epa.h"
# include "gjk.h"

# include <pybind11/stl.h>
# include <pybind11/stl_bind.h>
# include <pybind11/pybind11.h>

# include <vector>
# include <array>
# include <iostream>

namespace pybind11 { namespace detail { // TODO review this

    template <> struct type_caster<glm::vec3> {
    public:
        // This is the Python type we expect (PyGLM's vec3)
        PYBIND11_TYPE_CASTER(glm::vec3, _("glm::vec3"));

        // Conversion from Python to C++
        bool load(handle src, bool) {
            if (!src) return false;

            // Attempt to convert Python object to PyGLM's vec3
            try {
                auto pyVec3 = src.cast<pybind11::tuple>();
                value.x = pyVec3[0].cast<float>();
                value.y = pyVec3[1].cast<float>();
                value.z = pyVec3[2].cast<float>();
                return true;
            } catch (const std::exception& e) {
                return false;
            }
        }

        // Conversion from C++ to Python (optional)
        static handle cast(const glm::vec3& src, return_value_policy /* policy */, handle /* parent */) {
            return pybind11::make_tuple(src.x, src.y, src.z).release();
        }
    };

}} // namespace pybind11::detail

// definitions
void convert_points(
    const pybind11::list& np_points,

    std::vector<glm::vec3>& points
) {
    /*
    Converts numpy points from pybind11 to glm::vec3
    */
    points.reserve(pybind11::len(np_points) / 3);
    for (int i = 0; i < pybind11::len(np_points); i += 3) {
        points.push_back({
            np_points[i    ].cast<float>(), 
            np_points[i + 1].cast<float>(), 
            np_points[i + 2].cast<float>()
        });
    }
}

void convert_point(
    const pybind11::list& py_point,

    glm::vec3& point
) {
    /*
    Converts a single pybind11 point to glm::vec3
    */
    point = {
        py_point[0].cast<float>(),
        py_point[1].cast<float>(),
        py_point[2].cast<float>()
    };
}

pybind11::tuple format_collision_data(
    const glm::vec3&                            normal,
    const float&                                distance,
    const std::vector<std::array<glm::vec3, 3>> polytope,
    const std::array<int, 3>                    face
) {
    /*
    Converts the inputs to a pybind tuple to be returned
    */
    return pybind11::make_tuple(normal, distance, polytope, face);
}

pybind11::tuple get_narrow_collision(
    const pybind11::list& np_points1, // list of floats from np, /3 for each point
    const pybind11::list& np_points2,
    const pybind11::list& py_position1,
    const pybind11::list& py_position2
) {
    /*
    Computes gjk and epa, provides python the output from epa if successful
    */
    std::cout << "starting" << std::endl;

    // convert list points to usable structure
    std::vector<glm::vec3> points1, points2;
    convert_points(np_points1, points1);
    convert_points(np_points2, points2);

    glm::vec3 position1, position2;
    convert_point(py_position1, position1);
    convert_point(py_position2, position2);

    std::cout << "converted" << std::endl;

    // get gjk collision
    std::vector<std::array<glm::vec3, 3>> simplex;
    bool have_collided = get_gjk_collision(points1, points2, position1, position2, simplex);

    std::cout << "gjk done" << std::endl;

    std::cout << simplex.size() << std::endl;

    if (!have_collided) {
        std::cout << "failed" << std::endl;
        return format_collision_data({0, 0, 0}, 0, simplex, {0, 0, 0});
    }

    // get epa collision
    glm::vec3          normal;
    float              distance;
    std::array<int, 3> face;
    get_epa_from_gjk(points1, points2, simplex, normal, distance, face);

    std::cout << "epa done" << std::endl;

    return format_collision_data(normal, distance, simplex, face);
}

void bind_glm_vec3(pybind11::module& m) {
    pybind11::class_<glm::vec3>(m, "vec3")
        .def(pybind11::init<>())
        .def(pybind11::init<float, float, float>()) // overload
        .def_readwrite("x", &glm::vec3::x)
        .def_readwrite("y", &glm::vec3::y)
        .def_readwrite("z", &glm::vec3::z)
        .def("__repr__",
             [](const glm::vec3 &v) {
                 return "<vec3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")>";
             }
        );
}

PYBIND11_MODULE(narrow_collisions, handle) {
    handle.def("get_narrow_collision", &get_narrow_collision, "Gets both GJK and EPA. Distance for the collision is 0 if there is no collision", pybind11::arg("points1"), pybind11::arg("points2"), pybind11::arg("position1"), pybind11::arg("position2"));

    bind_glm_vec3(handle);
}

// g++ -std=c++17 -shared -undefined dynamic_lookup -I/pybind11/include/ `python3.12 -m pybind11 --includes` epa.cpp helper.cpp gjk.cpp narrow_collisions.cpp -o narrow_collisions.so `python3.12-config --ldflags`