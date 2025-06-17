// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// LIBRARY FOR GEOMETRIC OPERATIONS ON IMAGES
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#include "colors.hpp"
#include <cmath> // For copysignf function
#include <numbers>

// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ------------------------------------------------------------------------------------------------------------
class Point;
class Normal;
class Vec;
class Vec2d;
class Transformation;
class HomMatrix;

Vec operator+(const Vec &a, const Vec &b);
Vec operator*(const HomMatrix &a, const Transformation &b);
Vec operator*(const HomMatrix &a, const Vec &b);

// ------------------------------------------------------------------------------------------------------------
// ------------------ VECTOR CLASS (2d)
// ------------------------------------------------------------------------------------------------------------

class Vec2d {
public:
  //-------Properties--------
  float u, v;

  //-----------Constructors-----------

  /// @brief Default constructor initializes to (0, 0)
  Vec2d() : u(0), v(0) {}

  /// @brief Constructor with parameters
  Vec2d(float u, float v) : u(u), v(v) {}

  //--------------------Methods----------------------

  /// @brief Convert 2D vector to string
  std::string to_string() const {
    std::ostringstream oss;
    oss << "(" << u << ", " << v << ")";
    return oss.str();
  }

  /// @brief Print vector to screen
  void print() const { std::cout << to_string() << std::endl; }

  /// @brief Check if two 2D vectors are close
  bool is_close(const Vec2d &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
    return are_close(u, other.u, error_tolerance) && are_close(v, other.v, error_tolerance);
  }
};

// ------------------------------------------------------------------------------------------------------------
// ------------------ VECTOR CLASS (3d)
// ------------------------------------------------------------------------------------------------------------

class Vec {
public:
  //-------Properties--------
  float x, y, z;

  //-----------Constructors-----------

  /// @brief Default constructor initializes to (0, 0, 0)
  Vec() : x(0.f), y(0.f), z(0.f) {}

  /// @brief Constructor accepting cartesian coordinates
  Vec(float x, float y, float z) : x(x), y(y), z(z) {}

  /// @brief Constructor for a normalized Vec accepting polar coordinates
  Vec(float theta, float phi) : x(std::sin(theta) * std::cos(phi)), y(std::sin(theta) * std::sin(phi)), z(std::cos(theta)) {}

  //--------------------Methods----------------------

  /// @brief Convert vector to string
  std::string to_string() const {
    std::ostringstream oss;
    oss << "(" << x << ", " << y << ", " << z << ")";
    return oss.str();
  }

  /// @brief Print vector to screen
  void print() const { std::cout << to_string() << std::endl; }

  /// @brief Check if two vectors are close
  bool is_close(const Vec &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
    return are_close(x, other.x, error_tolerance) && are_close(y, other.y, error_tolerance) &&
           are_close(z, other.z, error_tolerance);
  }

  /// @brief Return negative vector
  Vec operator-() const { return Vec(-x, -y, -z); }

  /// @brief Return squared norm of vector
  float squared_norm() const { return x * x + y * y + z * z; }

  /// @brief Return norm of vector
  float norm() const { return std::sqrt(squared_norm()); }

  /// @brief Normalize vector
  [[nodiscard]]
  Vec normalize() const {
    float n = norm();
    if (n == 0) {
      throw std::runtime_error("Cannot normalize a zero vector");
    }
    return Vec(x / n, y / n, z / n);
  }

  /// @brief Convert vector to a normal
  Normal to_normal() const;

  /// @brief Convert vector to a point
  Point to_point() const;
};

// ------------------------------------------------------------------------------------------------------------
// POINT CLASS
// ------------------------------------------------------------------------------------------------------------

class Point {
public:
  //-------Properties--------

  float x, y, z;

  //-----------Constructors-----------

  /// @brief Default constructor: initialize to (0, 0, 0)
  Point() : x(0.f), y(0.f), z(0.f) {}

  /// @brief Constructor with parameters
  Point(float x, float y, float z) : x(x), y(y), z(z) {}

  //------------Methods-----------

  /// @brief Convert point to string
  std::string to_string() const {
    std::ostringstream oss;
    oss << "(" << x << ", " << y << ", " << z << ")";
    return oss.str();
  }

  /// @brief Print point to screen
  void print() const { std::cout << to_string() << std::endl; }

  /// @brief Check if two points are close
  bool is_close(const Point &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
    return are_close(x, other.x, error_tolerance) && are_close(y, other.y, error_tolerance) &&
           are_close(z, other.z, error_tolerance);
  }

  /// @brief Convert point to a vector
  Vec to_vector() const;
};

// ------------------------------------------------------------------------------------------------------------
// -------------NORMAL CLASS ----------------------------
// ------------------------------------------------------------------------------------------------------------

class Normal {
public:
  //-------Properties--------

  float x, y, z;

  //-----------Constructors-----------

  /// @brief Default constructor: initialize to (0, 0, 0)
  Normal() : x(0.f), y(0.f), z(0.f) {}

  /// @brief Constructor with parameters
  Normal(float x, float y, float z) : x(x), y(y), z(z) {}

  //------------Methods-----------

  /// @brief Convert normal to string
  std::string to_string() const {
    std::ostringstream oss;
    oss << "(" << x << ", " << y << ", " << z << ")";
    return oss.str();
  }

  /// @brief Print normal to screen
  void print() const { std::cout << to_string() << std::endl; }

  /// @brief Check if two normals are close
  bool is_close(const Normal &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
    return are_close(x, other.x, error_tolerance) && are_close(y, other.y, error_tolerance) &&
           are_close(z, other.z, error_tolerance);
  }

  /// @brief Return opposite normal
  Normal operator-() const { return Normal(-x, -y, -z); }

  /// @brief Return squared norm of normal
  float squared_norm() const { return x * x + y * y + z * z; }

  /// @brief Return norm of normal
  float norm() const { return std::sqrt(squared_norm()); }

  /// @brief Normalize normal
  [[nodiscard]]
  Normal normalize() const {
    float n = norm();
    if (n == 0) {
      throw std::runtime_error("Cannot normalize a zero normal");
    }
    return Normal(x / n, y / n, z / n);
  }

  /// @brief Convert normal to a vector
  Vec to_vector() const;
};

//-------------------------------------------------------------------------------------------------------------
//------------------------------- HOM_MATRIX CLASS----------------------
//-------------------------------------------------------------------------------------------------------------
class HomMatrix {
public:
  //-------Properties--------
  std::array<std::array<float, 3>, 3> linear_part; // linear part (3x3 matrix)
  Vec translation_vec;                             // translation vector (3x1 vector)

  //-----------Constructors-----------

  /// @brief copy constructor
  HomMatrix(const HomMatrix &other) : linear_part(other.linear_part), translation_vec(other.translation_vec) {}

  /// @brief default constructor (initializes to identity)
  HomMatrix() {
    // Fill linear part (identity)
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        linear_part[i][j] = (i == j ? 1.f : 0.f);

    // Fill translation vector (0)
    translation_vec.x = translation_vec.y = translation_vec.z = 0.f;
  }

  /// @brief Constructor accepting linear part and translation vector
  HomMatrix(const std::array<std::array<float, 3>, 3> &linear_part, const Vec &translation_vec)
      : linear_part(linear_part), translation_vec(translation_vec) {}

  /// @brief Constructor accepting linear part only
  HomMatrix(const std::array<std::array<float, 3>, 3> &linear_part) : linear_part(linear_part), translation_vec() {}

  /// @brief Constructor accepting translation vector only
  HomMatrix(const Vec &translation_vec) : translation_vec(translation_vec) {
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        linear_part[i][j] = (i == j ? 1.f : 0.f);
  }

  //------------Methods-----------

  ///@brief Check if two matrices are close
  bool is_close(const HomMatrix &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
    // check if the two linear parts are close
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        if (!are_close(linear_part[i][j], other.linear_part[i][j], error_tolerance)) {
          return false;
        }
      }
    }
    // Check if the two translation vectors are close
    if (!translation_vec.is_close(other.translation_vec, error_tolerance)) {
      return false;
    }
    return true;
  }
};

//-------------------------------------------------------------------------------------------------------------
//------------------------------- TRANSFORMATION CLASS----------------------
//-------------------------------------------------------------------------------------------------------------
class Transformation {
public:
  //-------Properties--------

  HomMatrix hom_matrix;         // homogeneous transformation matrix
  HomMatrix inverse_hom_matrix; // inverse homogeneous transformation matrix

  //-----------Constructors-----------

  // The following constructors always build the inverse matrix too

  /// @brief Default constructor: initializes to identity
  Transformation() : hom_matrix(), inverse_hom_matrix() {}

  /// @brief Constructor accepting assigned homogeneous matrix and its inverse
  Transformation(HomMatrix hom_matrix, HomMatrix inverse_hom_matrix)
      : hom_matrix(hom_matrix), inverse_hom_matrix(inverse_hom_matrix) {}

  /// @brief Constructor accepting assigned linear_part, inverse_linear_part and translation
  /// @param linear_part linear part as std::array<std::array<float, 3>, 3>
  /// @param inverse_linear_part inverse linear part as std::array<std::array<float, 3>, 3>
  /// @param translation_vec translation vector as Vec
  Transformation(const std::array<std::array<float, 3>, 3> &linear_part,
                 const std::array<std::array<float, 3>, 3> &inverse_linear_part, const Vec &translation_vec,
                 const Vec &inverse_translation_vec)
      : hom_matrix(linear_part, translation_vec), inverse_hom_matrix(inverse_linear_part, inverse_translation_vec) {}

  /// @brief Constructor for translations
  /// @param translation_vec translation vector as Vec
  Transformation(const Vec &translation_vec) : hom_matrix(translation_vec), inverse_hom_matrix((-translation_vec)) {}

  /// @brief Constructor for rotations
  /// @param rotation_matrix rotation matrix as std::array<std::array<float, 3>, 3>
  Transformation(const std::array<std::array<float, 3>, 3> &rotation_matrix) {
    hom_matrix = HomMatrix(rotation_matrix);
    inverse_hom_matrix = HomMatrix();
    // Invert the rotation matrix
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        inverse_hom_matrix.linear_part[i][j] = rotation_matrix[j][i]; // Transpose for rotation matrix
  }

  /// @brief constructor for scalings and reflections (diagonal only)
  /// @param diagonal diagonal of the linear part as std::array<float, 3>
  Transformation(const std::array<float, 3> &diagonal) {
    hom_matrix = HomMatrix();
    inverse_hom_matrix = HomMatrix();
    for (int i = 0; i < 3; ++i) {
      hom_matrix.linear_part[i][i] = diagonal[i];
      inverse_hom_matrix.linear_part[i][i] = 1.0f / diagonal[i];
    }
  }

  //------------Methods-----------

  /// @brief Check if hom_matrix and inverse_hom_matrix are indeed inverse
  bool is_consistent() const {
    // Check if product of linear parts of hom_matrix and inverse_hom_matrix is the identity
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        float sum = 0.f;
        for (int k = 0; k < 3; ++k) {
          sum += hom_matrix.linear_part[i][k] * inverse_hom_matrix.linear_part[k][j];
        }
        if (i == j && !are_close(sum, 1.f, DEFAULT_ERROR_TOLERANCE)) {
          return false;
        } else if (i != j && !are_close(sum, 0.f, DEFAULT_ERROR_TOLERANCE)) {
          return false;
        }
      }
    }
    // Check if product of translation vectors is the identity
    Vec vec = (hom_matrix * (inverse_hom_matrix.translation_vec)) + hom_matrix.translation_vec;
    if (!are_close(vec.x, 0.f, DEFAULT_ERROR_TOLERANCE) || !are_close(vec.y, 0.f, DEFAULT_ERROR_TOLERANCE) ||
        !are_close(vec.z, 0.f, DEFAULT_ERROR_TOLERANCE)) {
      return false;
    } else {
      return true;
    }
  }

  /// @brief check if transformation is close to another transformation
  bool is_close(const Transformation &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
    return hom_matrix.is_close(other.hom_matrix, error_tolerance) &&
           inverse_hom_matrix.is_close(other.inverse_hom_matrix, error_tolerance);
  }

  /// @brief return the inverse transofrmation
  Transformation inverse() const { return Transformation(inverse_hom_matrix, hom_matrix); }
};

//-------------------------------------------------------------------------------------------------------------
//--- IMPLEMENTATION OF CONVERSION BETWEEN GEOM OBJECTS-------
//-------------------------------------------------------------------------------------------------------------

/// @brief Convert a vector to a point
Point Vec::to_point() const { return Point(x, y, z); }

// @brief Convert a vector to a normal
Normal Vec::to_normal() const { return Normal(x, y, z); }

/// @brief Convert normal to a vector
Vec Normal::to_vector() const { return Vec(x, y, z); }

/// @brief Convert a point to a vector
Vec Point::to_vector() const { return Vec(x, y, z); }

//---------------------------------------------------------------
//--------OPERATIONS with VEC, NORMAL and POINT ----------------------
//---------------------------------------------------------------

/// @brief General sum operation `In1 + In2 → Out`
template <typename In1, typename In2, typename Out> Out _sum(const In1 &a, const In2 &b) {
  return Out{a.x + b.x, a.y + b.y, a.z + b.z};
}

/// @brief Sum of two vectors, returning a vector
Vec operator+(const Vec &a, const Vec &b) { return _sum<Vec, Vec, Vec>(a, b); }

/// @brief Sum of a point and a vector, returning a point
Point operator+(const Point &a, const Vec &b) { return _sum<Point, Vec, Point>(a, b); }

/// @brief General difference operation `In1 - In2 → Out`
template <typename In1, typename In2, typename Out> Out _difference(const In1 &a, const In2 &b) {
  return Out{a.x - b.x, a.y - b.y, a.z - b.z};
}

/// @brief Difference of two vectors, returning a vector
Vec operator-(const Vec &a, const Vec &b) { return _difference<Vec, Vec, Vec>(a, b); }

/// @brief Difference of a point and a vector, returning a point
Point operator-(const Point &a, Vec &b) { return _difference<Point, Vec, Point>(a, b); }

/// @brief Difference of a point and a point, returning a vector
Vec operator-(const Point &a, const Point &b) { return _difference<Point, Point, Vec>(a, b); }

/// @brief General dot product operation `In1 - In2 → Out`
template <typename In1, typename In2, typename Out> Out _dot_product(const In1 &a, const In2 &b) {
  return Out{a.x * b.x + a.y * b.y + a.z * b.z};
}

/// @brief Dot product of two vectors
float operator*(const Vec &a, const Vec &b) { return _dot_product<Vec, Vec, float>(a, b); }

/// @brief Dot product of a vector and a normal
float operator*(const Vec &a, const Normal &b) { return _dot_product<Vec, Normal, float>(a, b); }

/// @brief Dot product of a normal and a vector
float operator*(const Normal &a, const Vec &b) { return _dot_product<Normal, Vec, float>(a, b); }

/// @brief Dot product of two normals
float operator*(const Normal &a, const Normal &b) { return _dot_product<Normal, Normal, float>(a, b); }

/// @brief General cross product operation `In1 x In2 → Out`
template <typename In1, typename In2, typename Out> Out _cross_product(const In1 &a, const In2 &b) {
  return Out{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

/// @brief Cross product of two vectors, returning a vector
Vec operator^(const Vec &a, const Vec &b) { return _cross_product<Vec, Vec, Vec>(a, b); }

/// @brief Cross product of a vector and a normal, returning a vector
Vec operator^(const Vec &a, const Normal &b) { return _cross_product<Vec, Normal, Vec>(a, b); }

/// @brief Cross product of a normal and a vector, returning a vector
Vec operator^(const Normal &a, const Vec &b) { return _cross_product<Normal, Vec, Vec>(a, b); }

/// @brief Cross product of two normals, returning a vector
Vec operator^(const Normal &a, const Normal &b) { return _cross_product<Normal, Normal, Vec>(a, b); }

/// @brief General right scalar multiplication operation `In * scalar → Out`
template <typename In1, typename In2, typename Out> Out right_scalar_multiplication(const In1 &a, const In2 &b) {
  return Out{a.x * b, a.y * b, a.z * b};
}

/// @brief General left scalar multiplication operation `scalar * In → Out`
template <typename In1, typename In2, typename Out> Out left_scalar_multiplication(const In1 &a, const In2 &b) {
  return Out{a * b.x, a * b.y, a * b.z};
}

/// @brief Right scalar multiplication `Vec * scalar → Vec`
Vec operator*(const Vec &a, float b) { return right_scalar_multiplication<Vec, float, Vec>(a, b); }

/// @brief Left scalar multiplication `scalar * Vec → Vec`
Vec operator*(float a, const Vec &b) { return left_scalar_multiplication<float, Vec, Vec>(a, b); }

/// @brief Right scalar multiplication `Point * scalar → Point`
Point operator*(const Point &a, float b) { return right_scalar_multiplication<Point, float, Point>(a, b); }

/// @brief Left scalar multiplication `scalar * Point → Point`
Point operator*(float a, const Point &b) { return left_scalar_multiplication<float, Point, Point>(a, b); }

/// @brief Right scalar multiplication of a normal and a float, returning a normal
Normal operator*(const Normal &a, float b) { return right_scalar_multiplication<Normal, float, Normal>(a, b); }

/// @brief Left scalar multiplication of a float and a normal, returning a normal
Normal operator*(float a, const Normal &b) { return left_scalar_multiplication<float, Normal, Normal>(a, b); }

//----------------------------------------------------------------------
//-------------OPERATIONS with TRANSFORMATIONS and POINT,VEC,NORMAL--------------------
//----------------------------------------------------------------------

/// @brief Product between a HomMatrix and a vector
Vec operator*(const HomMatrix &a, const Vec &b) {
  // NOTE check if implementation is consistent with other operations transformation/hom_matrix * smth
  //  vector transforms with linear part only, translation is not applied
  Vec result;
  result.x = a.linear_part[0][0] * b.x + a.linear_part[0][1] * b.y + a.linear_part[0][2] * b.z;
  result.y = a.linear_part[1][0] * b.x + a.linear_part[1][1] * b.y + a.linear_part[1][2] * b.z;
  result.z = a.linear_part[2][0] * b.x + a.linear_part[2][1] * b.y + a.linear_part[2][2] * b.z;
  return result;
}

/// @brief Product between a transformation and a vector
Vec operator*(const Transformation &a, const Vec &b) {
  // OPT compiler should optimize this wrapping, but maybe check with benchmark in the future
  // vector transforms with linear part only, translation is not applied
  return a.hom_matrix * b;
}

/// @brief Product between a transformation and a point
Point operator*(const Transformation &a, const Point &b) {
  // OPT compiler should optimize this wrapping, but maybe check with benchmark in the future
  // point transforms with linear part + translation
  return (a * b.to_vector() + a.hom_matrix.translation_vec).to_point();
}

/// @brief Product between a transformation and a normal
Normal operator*(const Transformation &a, const Normal &b) {
  // Note that a normal transforms with inverse transpose of the linear part
  Normal result;
  result.x = a.inverse_hom_matrix.linear_part[0][0] * b.x + a.inverse_hom_matrix.linear_part[1][0] * b.y +
             a.inverse_hom_matrix.linear_part[2][0] * b.z;
  result.y = a.inverse_hom_matrix.linear_part[0][1] * b.x + a.inverse_hom_matrix.linear_part[1][1] * b.y +
             a.inverse_hom_matrix.linear_part[2][1] * b.z;
  result.z = a.inverse_hom_matrix.linear_part[0][2] * b.x + a.inverse_hom_matrix.linear_part[1][2] * b.y +
             a.inverse_hom_matrix.linear_part[2][2] * b.z;
  return result;
}

/// @brief Product between two transformations
Transformation operator*(const Transformation &a, const Transformation &b) {
  std::array<std::array<float, 3>, 3> linear_part;
  std::array<std::array<float, 3>, 3> inverse_linear_part;

  // Define translation vector (A.lin*B.trans+A.trans)
  Vec translation_vec = a.hom_matrix * b.hom_matrix.translation_vec + a.hom_matrix.translation_vec;

  // Define inverse translation vector (A^-1*w+v)
  Vec inverse_translation_vec =
      b.inverse_hom_matrix * a.inverse_hom_matrix.translation_vec + b.inverse_hom_matrix.translation_vec;

  // Define linear part A.lin*B.lin and inverse linear part (B.lin^-1)(A.lin^-1)
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      linear_part[i][j] = a.hom_matrix.linear_part[i][0] * b.hom_matrix.linear_part[0][j] +
                          a.hom_matrix.linear_part[i][1] * b.hom_matrix.linear_part[1][j] +
                          a.hom_matrix.linear_part[i][2] * b.hom_matrix.linear_part[2][j];
      inverse_linear_part[i][j] = b.inverse_hom_matrix.linear_part[i][0] * a.inverse_hom_matrix.linear_part[0][j] +
                                  b.inverse_hom_matrix.linear_part[i][1] * a.inverse_hom_matrix.linear_part[1][j] +
                                  b.inverse_hom_matrix.linear_part[i][2] * a.inverse_hom_matrix.linear_part[2][j];
    };
  };
  return Transformation(linear_part, inverse_linear_part, translation_vec,
                        inverse_translation_vec); // Call constructor (which aslo defines translation vector of inverse)
}

//------------------------------------------------------------------------
//-------- SPECIFIC TRANSFORMATIONS, IMPLEMENTED AS DERIVED CLASSES ------
//-----------------------------------------------------------------------

class rotation_x : public Transformation {
public:
  /// @brief Constructor: builds rotation matrix around x-axis (calls Transformation constructor that accepts a rotation matrix)
  /// @param rotation angle (rads)
  rotation_x(const float &theta)
      : Transformation({{{1.f, 0.f, 0.f}, {0.f, std::cos(theta), -std::sin(theta)}, {0.f, std::sin(theta), std::cos(theta)}}}) {}
};

class rotation_y : public Transformation {
public:
  /// @brief Constructor: builds rotation matrix around y-axis (calls Transformation constructor that accepts a rotation matrix)
  /// @param rotation angle (rads)
  rotation_y(const float &theta)
      : Transformation({{{std::cos(theta), 0.f, std::sin(theta)}, {0.f, 1.f, 0.f}, {-std::sin(theta), 0.f, std::cos(theta)}}}) {}
};

class rotation_z : public Transformation {
public:
  /// @brief Constructor builds rotation matrix around z-axis (calls Transformation constructor that accepts a rotation matrix)
  /// @param rotation angle (rads)
  rotation_z(const float &theta)
      : Transformation({{{std::cos(theta), -std::sin(theta), 0.f}, {std::sin(theta), std::cos(theta), 0.f}, {0.f, 0.f, 1.f}}}) {}
};

class translation : public Transformation {
public:
  /// @brief Constructor: calls Transformation constructor that accepts Vec
  /// @param translation Vec
  translation(Vec vec) : Transformation(vec) {}
};

class scaling : public Transformation {
public:
  /// @brief Contructor: calls Transformation constructor that accepts the diagonal of the linear part
  /// @param array of length 3 representing diagonal of linear part
  scaling(std::array<float, 3> diagonal) : Transformation(diagonal) {}
};

//-------------------------------------------------------------------------------------------------------------
//------------------------------- FURTHER GLOBAL CONSTANTS ----------------------
//-------------------------------------------------------------------------------------------------------------

const Vec VEC_X = Vec(1.f, 0.f, 0.f);
const Vec VEC_Y = Vec(0.f, 1.f, 0.f);
const Vec VEC_Z = Vec(0.f, 0.f, 1.f);

//-------------------------------------------------------------------------------------------------------------
//------------------------------- ORTHONORMAL BASIS OBJECT ----------------------
//-------------------------------------------------------------------------------------------------------------

///@brief orthonormal basis of 3d vectors
struct ONB {
  // ------- Properties --------
  Vec e1, e2, e3;

  // ------- Constructors --------
  /// @brief Default constructor: same basis as the world's reference frame
  ONB() : e1(VEC_X), e2(VEC_Y), e3(VEC_Z) {};

  /// @brief Constructor from three Vec
  ONB(Vec e1, Vec e2, Vec e3) : e1(e1), e2(e2), e3(e3) {};

  /// @brief Branchless constructor from a Vec (cast into e_3)
  /// @details Assumes the input Vec to be normalized. Based on the algorithm by Duff et al. (2017)
  /// @param vec normalized Vec = e_3
  ONB(Vec vec) : e3(vec) {
    float sign = std::copysignf(1.f, e3.z); // copysignf returns the absolute value of the first argument with the sign of the
                                            // second one (sign is set negative if e3.z=0)
    const float a = -1.f / (sign + e3.z);
    const float b = e3.x * e3.y * a;

    e1 = Vec(1.f + sign * e3.x * e3.x * a, sign * b, -sign * e3.x);
    e2 = Vec(b, sign + e3.y * e3.y * a, -e3.y);
  }

  // -------------------- Methods ----------------------
  /// @brief Returns true if it's actually a ONB, false otherwise
  bool is_consistent() const {
    if (!are_close(e1 * e2, 0.f) || !are_close(e1 * e3, 0.f) || !are_close(e2 * e3, 0.f)) {
      return false;
    }
    if (!are_close(e1.squared_norm(), 1.f) || !are_close(e2.squared_norm(), 1.f) || !are_close(e1.squared_norm(), 1.f)) {
      return false;
    }
    return true;
  }
};

//-------------------------------------------------------------------------------------------------------------
//------------------------------- DEGREES TO RADIANS CONVERSION ----------------------
//-------------------------------------------------------------------------------------------------------------

float degs_to_rads(const float &angle_degs) { return angle_degs * std::numbers::pi_v<float> / 180.f; };