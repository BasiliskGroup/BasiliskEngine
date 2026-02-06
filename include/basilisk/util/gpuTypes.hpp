#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace bsk::internal {

// Padded vec3 for WGSL compatibility (16-byte aligned)
// Inherits all glm::vec3 functionality while adding padding
struct alignas(16) vec3 : public glm::vec3 {
    float _padding;
    
    // Default constructor
    vec3() : glm::vec3(0.0f), _padding(0.0f) {}
    
    // Constructor from scalars
    vec3(float x, float y, float z) : glm::vec3(x, y, z), _padding(0.0f) {}
    
    // Constructor from single scalar (sets all components)
    explicit vec3(float scalar) : glm::vec3(scalar), _padding(0.0f) {}
    
    // Copy constructor from glm::vec3
    vec3(const glm::vec3& v) : glm::vec3(v), _padding(0.0f) {}
    
    // Copy constructor
    vec3(const vec3& v) : glm::vec3(v), _padding(0.0f) {}
    
    // Assignment from glm::vec3
    vec3& operator=(const glm::vec3& v) {
        glm::vec3::operator=(v);
        return *this;
    }
    
    // Assignment operator
    vec3& operator=(const vec3& v) {
        glm::vec3::operator=(v);
        return *this;
    }
    
    // Conversion to glm::vec3& (for modification)
    glm::vec3& asGlm() {
        return *static_cast<glm::vec3*>(this);
    }
    
    const glm::vec3& asGlm() const {
        return *static_cast<const glm::vec3*>(this);
    }
};

// Verify alignment at compile time
static_assert(sizeof(vec3) == 16, "vec3 must be 16 bytes for WGSL compatibility");
static_assert(alignof(vec3) == 16, "vec3 must be 16-byte aligned for WGSL compatibility");

// Padded mat3x3 for WGSL compatibility
// WGSL mat3x3 is laid out as 3 vec3f columns, each taking 16 bytes (12 data + 4 padding)
// Total: 48 bytes (vs glm::mat3's 36 bytes)
struct alignas(16) mat3x3 {
    vec3 columns[3];  // Each column is 16 bytes (vec3 with padding)
    
    // Default constructor (identity matrix)
    mat3x3() {
        columns[0] = vec3(1.0f, 0.0f, 0.0f);
        columns[1] = vec3(0.0f, 1.0f, 0.0f);
        columns[2] = vec3(0.0f, 0.0f, 1.0f);
    }
    
    // Constructor from glm::mat3
    mat3x3(const glm::mat3& m) {
        columns[0] = vec3(m[0]);
        columns[1] = vec3(m[1]);
        columns[2] = vec3(m[2]);
    }
    
    // Constructor from column vectors
    mat3x3(const vec3& col0, const vec3& col1, const vec3& col2) {
        columns[0] = col0;
        columns[1] = col1;
        columns[2] = col2;
    }
    
    // Constructor from glm column vectors
    mat3x3(const glm::vec3& col0, const glm::vec3& col1, const glm::vec3& col2) {
        columns[0] = vec3(col0);
        columns[1] = vec3(col1);
        columns[2] = vec3(col2);
    }
    
    // Constructor from scalar (diagonal matrix)
    explicit mat3x3(float diagonal) {
        columns[0] = vec3(diagonal, 0.0f, 0.0f);
        columns[1] = vec3(0.0f, diagonal, 0.0f);
        columns[2] = vec3(0.0f, 0.0f, diagonal);
    }
    
    // Copy constructor
    mat3x3(const mat3x3& m) {
        columns[0] = m.columns[0];
        columns[1] = m.columns[1];
        columns[2] = m.columns[2];
    }
    
    // Assignment operator
    mat3x3& operator=(const mat3x3& m) {
        columns[0] = m.columns[0];
        columns[1] = m.columns[1];
        columns[2] = m.columns[2];
        return *this;
    }
    
    // Assignment from glm::mat3
    mat3x3& operator=(const glm::mat3& m) {
        columns[0] = vec3(m[0]);
        columns[1] = vec3(m[1]);
        columns[2] = vec3(m[2]);
        return *this;
    }
    
    // Column access
    vec3& operator[](int i) {
        return columns[i];
    }
    
    const vec3& operator[](int i) const {
        return columns[i];
    }
    
    // Conversion to glm::mat3
    operator glm::mat3() const {
        return glm::mat3(
            glm::vec3(columns[0]),
            glm::vec3(columns[1]),
            glm::vec3(columns[2])
        );
    }
    
    // Convert to glm::mat3 for operations
    glm::mat3 asGlm() const {
        return glm::mat3(
            glm::vec3(columns[0]),
            glm::vec3(columns[1]),
            glm::vec3(columns[2])
        );
    }
    
    // Matrix-vector multiplication
    vec3 operator*(const vec3& v) const {
        glm::mat3 m = asGlm();
        glm::vec3 result = m * glm::vec3(v);
        return vec3(result);
    }
    
    // Matrix-matrix multiplication
    mat3x3 operator*(const mat3x3& other) const {
        glm::mat3 m1 = asGlm();
        glm::mat3 m2 = other.asGlm();
        return mat3x3(m1 * m2);
    }
    
    // Matrix addition
    mat3x3 operator+(const mat3x3& other) const {
        glm::mat3 m1 = asGlm();
        glm::mat3 m2 = other.asGlm();
        return mat3x3(m1 + m2);
    }
    
    // Matrix subtraction
    mat3x3 operator-(const mat3x3& other) const {
        glm::mat3 m1 = asGlm();
        glm::mat3 m2 = other.asGlm();
        return mat3x3(m1 - m2);
    }
    
    // Scalar multiplication
    mat3x3 operator*(float scalar) const {
        return mat3x3(asGlm() * scalar);
    }
    
    // Transpose
    mat3x3 transpose() const {
        return mat3x3(glm::transpose(asGlm()));
    }
    
    // Inverse
    mat3x3 inverse() const {
        return mat3x3(glm::inverse(asGlm()));
    }
    
    // Determinant
    float determinant() const {
        return glm::determinant(asGlm());
    }
};

// Verify alignment
static_assert(sizeof(mat3x3) == 48, "mat3x3 must be 48 bytes for WGSL compatibility");
static_assert(alignof(mat3x3) == 16, "mat3x3 must be 16-byte aligned for WGSL compatibility");

// Helper functions to match glm API

// Vector operations
inline vec3 normalize(const vec3& v) {
    return vec3(glm::normalize(glm::vec3(v)));
}

inline float length(const vec3& v) {
    return glm::length(glm::vec3(v));
}

inline float length2(const vec3& v) {
    return glm::dot(glm::vec3(v), glm::vec3(v));
}

inline float dot(const vec3& a, const vec3& b) {
    return glm::dot(glm::vec3(a), glm::vec3(b));
}

inline vec3 cross(const vec3& a, const vec3& b) {
    return vec3(glm::cross(glm::vec3(a), glm::vec3(b)));
}

inline vec3 reflect(const vec3& I, const vec3& N) {
    return vec3(glm::reflect(glm::vec3(I), glm::vec3(N)));
}

inline vec3 refract(const vec3& I, const vec3& N, float eta) {
    return vec3(glm::refract(glm::vec3(I), glm::vec3(N), eta));
}

inline vec3 min(const vec3& a, const vec3& b) {
    return vec3(glm::min(glm::vec3(a), glm::vec3(b)));
}

inline vec3 max(const vec3& a, const vec3& b) {
    return vec3(glm::max(glm::vec3(a), glm::vec3(b)));
}

inline vec3 clamp(const vec3& v, const vec3& minVal, const vec3& maxVal) {
    return vec3(glm::clamp(glm::vec3(v), glm::vec3(minVal), glm::vec3(maxVal)));
}

inline vec3 mix(const vec3& a, const vec3& b, float t) {
    return vec3(glm::mix(glm::vec3(a), glm::vec3(b), t));
}

// Matrix operations
inline mat3x3 transpose(const mat3x3& m) {
    return m.transpose();
}

inline mat3x3 inverse(const mat3x3& m) {
    return m.inverse();
}

inline float determinant(const mat3x3& m) {
    return m.determinant();
}

// Scalar * matrix
inline mat3x3 operator*(float scalar, const mat3x3& m) {
    return m * scalar;
}

// Common matrix constructors (as functions to avoid namespace pollution)
inline mat3x3 mat3x3_identity() {
    return mat3x3(1.0f);
}

inline mat3x3 mat3x3_zero() {
    return mat3x3(0.0f);
}

// Rotation matrices (using glm functions)
inline mat3x3 mat3x3_rotate(float angle, const vec3& axis) {
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(axis));
    return mat3x3(glm::mat3(rot));
}

inline mat3x3 mat3x3_scale(const vec3& scale) {
    return mat3x3(
        vec3(scale.x, 0.0f, 0.0f),
        vec3(0.0f, scale.y, 0.0f),
        vec3(0.0f, 0.0f, scale.z)
    );
}

// Outer product
inline mat3x3 outerProduct(const vec3& c, const vec3& r) {
    return mat3x3(glm::outerProduct(glm::vec3(c), glm::vec3(r)));
}

}