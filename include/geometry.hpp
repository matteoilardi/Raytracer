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

// ------------------------------------------------------------------------------------------------------------
// ------------------ VECTOR CLASS (2d)
// ------------------------------------------------------------------------------------------------------------

class Vec2d {
public:
  //-------Properties--------
  float u, v;

  //-----------Constructors-----------

  /// @brief Default constructor initializes to (0, 0)
  constexpr Vec2d() noexcept : u(0.f), v(0.f) {}

  /// @brief Constructor with parameters
  constexpr Vec2d(float u, float v) noexcept : u(u), v(v) {}

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
  constexpr bool is_close(const Vec2d &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const noexcept {
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
  constexpr Vec() noexcept : x(0.f), y(0.f), z(0.f) {}

  /// @brief Constructor accepting cartesian coordinates
  constexpr Vec(float x, float y, float z) noexcept : x(x), y(y), z(z) {}

  /// @brief Constructor for a normalized Vec accepting polar coordinates
  constexpr Vec(float theta, float phi) noexcept
      : x(std::sin(theta) * std::cos(phi)), y(std::sin(theta) * std::sin(phi)), z(std::cos(theta)) {}

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
  constexpr bool is_close(const Vec &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const noexcept {
    return are_close(x, other.x, error_tolerance) && are_close(y, other.y, error_tolerance) &&
           are_close(z, other.z, error_tolerance);
  }

  /// @brief Return negative vector
  constexpr Vec operator-() const noexcept { return Vec(-x, -y, -z); }

  /// @brief Return squared norm of vector
  constexpr float squared_norm() const noexcept { return x * x + y * y + z * z; }

  /// @brief Return norm of vector
  constexpr float norm() const noexcept { return std::sqrt(squared_norm()); }

  /// @brief Normalize vector
  [[nodiscard]]
  constexpr Vec normalized() const noexcept {
    float n = norm();
    return Vec(x / n, y / n, z / n);
  }

  /// @brief Convert vector to a normal
  constexpr Normal to_normal() const noexcept;

  /// @brief Convert vector to a point
  constexpr Point to_point() const noexcept;
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
  constexpr Point() noexcept : x(0.f), y(0.f), z(0.f) {}

  /// @brief Constructor with parameters
  constexpr Point(float x, float y, float z) noexcept : x(x), y(y), z(z) {}

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
  constexpr bool is_close(const Point &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const noexcept {
    return are_close(x, other.x, error_tolerance) && are_close(y, other.y, error_tolerance) &&
           are_close(z, other.z, error_tolerance);
  }

  /// @brief Convert point to a vector
  constexpr Vec to_vector() const noexcept;
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
  constexpr Normal() noexcept : x(0.f), y(0.f), z(0.f) {}

  /// @brief Constructor with parameters
  constexpr Normal(float x, float y, float z) noexcept : x(x), y(y), z(z) {}

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
  constexpr bool is_close(const Normal &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const noexcept {
    return are_close(x, other.x, error_tolerance) && are_close(y, other.y, error_tolerance) &&
           are_close(z, other.z, error_tolerance);
  }

  /// @brief Return opposite normal
  constexpr Normal operator-() const noexcept { return Normal(-x, -y, -z); }

  /// @brief Return squared norm of normal
  constexpr float squared_norm() const noexcept { return x * x + y * y + z * z; }

  /// @brief Return norm of normal
  constexpr float norm() const noexcept { return std::sqrt(squared_norm()); }

  /// @brief Normalize normal
  [[nodiscard]]
  constexpr Normal normalized() const noexcept {
    float n = norm();
    return Normal(x / n, y / n, z / n);
  }

  /// @brief Convert normal to a vector
  constexpr Vec to_vector() const noexcept;
};

//-------------------------------------------------------------------------------------------------------------
//--- CONVERSION BETWEEN GEOM OBJECTS -------
//-------------------------------------------------------------------------------------------------------------

/// @brief Convert a vector to a point
constexpr Point Vec::to_point() const noexcept { return Point{x, y, z}; }

// @brief Convert a vector to a normal
constexpr Normal Vec::to_normal() const noexcept { return Normal{x, y, z}; }

/// @brief Convert normal to a vector
constexpr Vec Normal::to_vector() const noexcept { return Vec{x, y, z}; }

/// @brief Convert a point to a vector
constexpr Vec Point::to_vector() const noexcept { return Vec{x, y, z}; }

//---------------------------------------------------------------
//-------- OPERATIONS with VEC, NORMAL and POINT ----------
//---------------------------------------------------------------

// Concepts
template<typename T>
concept VecLike = std::same_as<T, Vec>;

template<typename T>
concept PointLike = std::same_as<T, Point>;

template<typename T>
concept NormalLike = std::same_as<T, Normal>;

// Legal sum combinations
template<typename A, typename B>
concept Sumable = (PointLike<A> && VecLike<B>) || (VecLike<A> && VecLike<B>);

// Low-level addition
template<typename A, typename B, typename Out>
constexpr Out _sum(const A& a, const B& b) noexcept {
    return Out{a.x + b.x, a.y + b.y, a.z + b.z};
}

// Sum result type traits
template<typename A, typename B>
struct SumResult;

template<> struct SumResult<Point, Vec> { using type = Point; };
template<> struct SumResult<Vec, Vec>   { using type = Vec; };

// Public sum operator
template<typename A, typename B>
requires Sumable<A,B>
constexpr auto operator+(const A& a, const B& b) noexcept {
    using Out = typename SumResult<A,B>::type;
    return _sum<A,B,Out>(a,b);
}

// --------

/// @brief General difference operation `In1 - In2 → Out`
template <typename In1, typename In2, typename Out> constexpr Out _difference(const In1 &a, const In2 &b) noexcept {
  return Out{a.x - b.x, a.y - b.y, a.z - b.z};
}

/// @brief Difference of two vectors, returning a vector
constexpr Vec operator-(const Vec &a, const Vec &b) noexcept { return _difference<Vec, Vec, Vec>(a, b); }

/// @brief Difference of a point and a vector, returning a point
constexpr Point operator-(const Point &a, Vec &b) noexcept { return _difference<Point, Vec, Point>(a, b); }

/// @brief Difference of a point and a point, returning a vector
constexpr Vec operator-(const Point &a, const Point &b) noexcept { return _difference<Point, Point, Vec>(a, b); }

/// @brief General dot product operation `In1 - In2 → Out`
template <typename In1, typename In2, typename Out> constexpr Out _dot_product(const In1 &a, const In2 &b) noexcept {
  return Out{a.x * b.x + a.y * b.y + a.z * b.z};
}

/// @brief Dot product of two vectors
constexpr float operator*(const Vec &a, const Vec &b) noexcept { return _dot_product<Vec, Vec, float>(a, b); }

/// @brief Dot product of a vector and a normal
constexpr float operator*(const Vec &a, const Normal &b) noexcept { return _dot_product<Vec, Normal, float>(a, b); }

/// @brief Dot product of a normal and a vector
constexpr float operator*(const Normal &a, const Vec &b) noexcept { return _dot_product<Normal, Vec, float>(a, b); }

/// @brief Dot product of two normals
constexpr float operator*(const Normal &a, const Normal &b) noexcept { return _dot_product<Normal, Normal, float>(a, b); }

/// @brief General cross product operation `In1 x In2 → Out`
template <typename In1, typename In2, typename Out> constexpr Out _cross_product(const In1 &a, const In2 &b) noexcept {
  return Out{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

/// @brief Cross product of two vectors, returning a vector
constexpr Vec operator^(const Vec &a, const Vec &b) noexcept { return _cross_product<Vec, Vec, Vec>(a, b); }

/// @brief Cross product of a vector and a normal, returning a vector
constexpr Vec operator^(const Vec &a, const Normal &b) noexcept { return _cross_product<Vec, Normal, Vec>(a, b); }

/// @brief Cross product of a normal and a vector, returning a vector
constexpr Vec operator^(const Normal &a, const Vec &b) noexcept { return _cross_product<Normal, Vec, Vec>(a, b); }

/// @brief Cross product of two normals, returning a vector
constexpr Vec operator^(const Normal &a, const Normal &b) noexcept { return _cross_product<Normal, Normal, Vec>(a, b); }

/// @brief General right scalar multiplication operation `In * scalar → Out`
template <typename In1, typename In2, typename Out>
constexpr Out right_scalar_multiplication(const In1 &a, const In2 &b) noexcept {
  return Out{a.x * b, a.y * b, a.z * b};
}

/// @brief General left scalar multiplication operation `scalar * In → Out`
template <typename In1, typename In2, typename Out>
constexpr Out left_scalar_multiplication(const In1 &a, const In2 &b) noexcept {
  return Out{a * b.x, a * b.y, a * b.z};
}

/// @brief Right scalar multiplication `Vec * scalar → Vec`
constexpr Vec operator*(const Vec &a, float b) noexcept { return right_scalar_multiplication<Vec, float, Vec>(a, b); }

/// @brief Left scalar multiplication `scalar * Vec → Vec`
constexpr Vec operator*(float a, const Vec &b) noexcept { return left_scalar_multiplication<float, Vec, Vec>(a, b); }

/// @brief Right scalar multiplication `Point * scalar → Point`
constexpr Point operator*(const Point &a, float b) noexcept { return right_scalar_multiplication<Point, float, Point>(a, b); }

/// @brief Left scalar multiplication `scalar * Point → Point`
constexpr Point operator*(float a, const Point &b) noexcept { return left_scalar_multiplication<float, Point, Point>(a, b); }

/// @brief Right scalar multiplication of a normal and a float, returning a normal
constexpr Normal operator*(const Normal &a, float b) noexcept { return right_scalar_multiplication<Normal, float, Normal>(a, b); }

/// @brief Left scalar multiplication of a float and a normal, returning a normal
constexpr Normal operator*(float a, const Normal &b) noexcept { return left_scalar_multiplication<float, Normal, Normal>(a, b); }

//-------------------------------------------------------------------------------------------------------------
//------------------------------- HOM_MATRIX CLASS----------------------
//-------------------------------------------------------------------------------------------------------------
class HomMatrix {
public:
  //-------Properties--------
  std::array<std::array<float, 3>, 3> linear_part; // linear part (3x3 matrix)
  Vec translation_vec;                             // translation vector

  //-----------Constructors-----------

  /// @brief copy constructor
  constexpr HomMatrix(const HomMatrix &other) = default;

  /// @brief default constructor (initializes to identity)
  constexpr HomMatrix() noexcept
      : linear_part{{{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}}}, translation_vec{0.f, 0.f, 0.f} {}

  /// @brief Constructor accepting linear part and translation vector
  constexpr HomMatrix(const std::array<std::array<float, 3>, 3> &linear_part, const Vec &translation_vec) noexcept
      : linear_part(linear_part), translation_vec(translation_vec) {}

  /// @brief Constructor accepting linear part only
  constexpr HomMatrix(const std::array<std::array<float, 3>, 3> &linear_part) noexcept
      : linear_part(linear_part), translation_vec() {}

  /// @brief Constructor accepting translation vector only
  HomMatrix(const Vec &translation_vec) noexcept
      : linear_part{{{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}}}, translation_vec(translation_vec) {}

  //------------Methods-----------

  ///@brief Check if two matrices are close
  constexpr bool is_close(const HomMatrix &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const noexcept {
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

  friend class Transformation;

private:
  /// @brief Product between a transformation and a vector
  constexpr Vec apply(const Vec &v) const noexcept {
    // A vector transforms with linear part only: translation is not applied
    const auto &L = linear_part;
    return Vec{L[0][0] * v.x + L[0][1] * v.y + L[0][2] * v.z, L[1][0] * v.x + L[1][1] * v.y + L[1][2] * v.z,
               L[2][0] * v.x + L[2][1] * v.y + L[2][2] * v.z};
  }

  /// @brief Product between a transformation and a point
  constexpr Point apply(const Point &p) const noexcept {
    // A point transforms with linear part + translation
    const auto &L = linear_part;
    const auto &t = translation_vec;
    return Point{L[0][0] * p.x + L[0][1] * p.y + L[0][2] * p.z + t.x, L[1][0] * p.x + L[1][1] * p.y + L[1][2] * p.z + t.y,
                 L[2][0] * p.x + L[2][1] * p.y + L[2][2] * p.z + t.z};
  }

  /// @brief Product between a transformation and a normal
  constexpr Normal apply_transposed(const Normal &n) const noexcept {
    const auto &L = linear_part;
    return Normal{L[0][0] * n.x + L[1][0] * n.y + L[2][0] * n.z, L[0][1] * n.x + L[1][1] * n.y + L[2][1] * n.z,
                  L[0][2] * n.x + L[1][2] * n.y + L[2][2] * n.z};
  }

  /// @brief Product between two transformations
  constexpr HomMatrix compose(const HomMatrix &other) const noexcept {
    // Given two homogeneous matrices:
    //     T_A = [ A  t_A ],   T_B = [ B  t_B ],
    //           [ 0   1  ]          [ 0   1  ]
    // where A,B ∈ R^{3×3} and t_A,t_B ∈ R^3, their product is:
    //     T_A * T_B = [ A·B    A·t_B + t_A ]
    //                 [  0           1     ].

    const auto &A = linear_part;
    const auto &B = other.linear_part;
    const auto &t_A = translation_vec;
    const auto &t_B = other.translation_vec;

    Vec translation_vec = this->apply(t_B) + t_A;

    std::array<std::array<float, 3>, 3> linear_part{};
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        linear_part[i][j] = A[i][0] * B[0][j] + A[i][1] * B[1][j] + A[i][2] * B[2][j];
      }
    }

    return HomMatrix(linear_part, translation_vec);
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
  constexpr Transformation() noexcept : hom_matrix(), inverse_hom_matrix() {}

  /// @brief Constructor accepting assigned homogeneous matrix and its inverse
  constexpr Transformation(HomMatrix hom_matrix, HomMatrix inverse_hom_matrix) noexcept
      : hom_matrix(hom_matrix), inverse_hom_matrix(inverse_hom_matrix) {}

  /// @brief Constructor accepting assigned linear_part, inverse_linear_part and translation
  /// @param linear_part linear part as std::array<std::array<float, 3>, 3>
  /// @param inverse_linear_part inverse linear part as std::array<std::array<float, 3>, 3>
  /// @param translation_vec translation vector as Vec
  constexpr Transformation(const std::array<std::array<float, 3>, 3> &linear_part,
                           const std::array<std::array<float, 3>, 3> &inverse_linear_part, const Vec &translation_vec,
                           const Vec &inverse_translation_vec) noexcept
      : hom_matrix(linear_part, translation_vec), inverse_hom_matrix(inverse_linear_part, inverse_translation_vec) {}

  /// @brief Constructor for translations
  /// @param translation_vec translation vector as Vec
  constexpr Transformation(const Vec &translation_vec) noexcept
      : hom_matrix(translation_vec), inverse_hom_matrix((-translation_vec)) {}

  /// @brief Constructor for rotations
  /// @param rotation_matrix rotation matrix as std::array<std::array<float, 3>, 3>
  constexpr Transformation(const std::array<std::array<float, 3>, 3> &rotation_matrix) noexcept {
    hom_matrix = HomMatrix(rotation_matrix);
    inverse_hom_matrix = HomMatrix();
    // Invert the rotation matrix by transposing it
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        inverse_hom_matrix.linear_part[i][j] = rotation_matrix[j][i];
  }

  /// @brief constructor for scalings and reflections (diagonal only)
  /// @param diagonal diagonal of the linear part as std::array<float, 3>
  constexpr Transformation(const std::array<float, 3> &diagonal) noexcept {
    hom_matrix = HomMatrix();
    inverse_hom_matrix = HomMatrix();
    for (int i = 0; i < 3; ++i) {
      hom_matrix.linear_part[i][i] = diagonal[i];
      inverse_hom_matrix.linear_part[i][i] = 1.0f / diagonal[i];
    }
  }

  //------------Methods-----------

  /// @brief Check if hom_matrix and inverse_hom_matrix are indeed inverse
  constexpr bool is_consistent() const noexcept {
    Transformation identity_check = *this * inverse();
    return identity_check.is_close(Transformation());
  }

  /// @brief Check if transformation is close to another transformation
  constexpr bool is_close(const Transformation &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const noexcept {
    return hom_matrix.is_close(other.hom_matrix, error_tolerance) &&
           inverse_hom_matrix.is_close(other.inverse_hom_matrix, error_tolerance);
  }

  /// @brief Return the inverse transofrmation
  constexpr Transformation inverse() const noexcept { return Transformation(inverse_hom_matrix, hom_matrix); }

  /// @brief Product between a transformation and a point
  constexpr Point operator*(const Point &p) const noexcept { return hom_matrix.apply(p); }

  /// @brief Product between a transformation and a vector
  constexpr Vec operator*(const Vec &v) const noexcept { return hom_matrix.apply(v); }

  /// @brief Product between a transformation and a normal
  constexpr Normal operator*(const Normal &n) const noexcept { return inverse_hom_matrix.apply_transposed(n); }

  /// @brief Product between two transformations
  constexpr Transformation operator*(const Transformation &other) const noexcept {
    // The components of the inverse matrix are computed using
    //     (T_A * T_B)^{-1} = T_B^{-1} * T_A^{-1}.
    return Transformation(hom_matrix.compose(other.hom_matrix), other.inverse_hom_matrix.compose(inverse_hom_matrix));
  }
};

//------------------------------------------------------------------------
//-------- SPECIFIC TRANSFORMATIONS, IMPLEMENTED AS DERIVED CLASSES ------
//-----------------------------------------------------------------------

class rotation_x : public Transformation {
public:
  /// @brief Constructor: builds rotation matrix around x-axis (calls Transformation constructor that accepts a rotation matrix)
  /// @param rotation angle (rads)
  constexpr rotation_x(const float &theta) noexcept
      : Transformation({{{1.f, 0.f, 0.f}, {0.f, std::cos(theta), -std::sin(theta)}, {0.f, std::sin(theta), std::cos(theta)}}}) {}
};

class rotation_y : public Transformation {
public:
  /// @brief Constructor: builds rotation matrix around y-axis (calls Transformation constructor that accepts a rotation matrix)
  /// @param rotation angle (rads)
  constexpr rotation_y(const float &theta) noexcept
      : Transformation({{{std::cos(theta), 0.f, std::sin(theta)}, {0.f, 1.f, 0.f}, {-std::sin(theta), 0.f, std::cos(theta)}}}) {}
};

class rotation_z : public Transformation {
public:
  /// @brief Constructor builds rotation matrix around z-axis (calls Transformation constructor that accepts a rotation matrix)
  /// @param rotation angle (rads)
  constexpr rotation_z(const float &theta) noexcept
      : Transformation({{{std::cos(theta), -std::sin(theta), 0.f}, {std::sin(theta), std::cos(theta), 0.f}, {0.f, 0.f, 1.f}}}) {}
};

class translation : public Transformation {
public:
  /// @brief Constructor: calls Transformation constructor that accepts Vec
  /// @param translation Vec
  constexpr translation(Vec vec) noexcept : Transformation{vec} {}
};

class scaling : public Transformation {
public:
  /// @brief Contructor: calls Transformation constructor that accepts the diagonal of the linear part
  /// @param array of length 3 representing diagonal of linear part
  constexpr scaling(std::array<float, 3> diagonal) noexcept : Transformation{diagonal} {}
};

//-------------------------------------------------------------------------------------------------------------
//------------------------------- FURTHER GLOBAL CONSTANTS ----------------------
//-------------------------------------------------------------------------------------------------------------

inline constexpr Vec VEC_X = Vec(1.f, 0.f, 0.f);
inline constexpr Vec VEC_Y = Vec(0.f, 1.f, 0.f);
inline constexpr Vec VEC_Z = Vec(0.f, 0.f, 1.f);

//-------------------------------------------------------------------------------------------------------------
//------------------------------- ORTHONORMAL BASIS OBJECT ----------------------
//-------------------------------------------------------------------------------------------------------------

///@brief orthonormal basis of 3d vectors
struct ONB {
  // ------- Properties --------
  Vec e1, e2, e3;

  // ------- Constructors --------
  /// @brief Default constructor: same basis as the world's reference frame
  constexpr ONB() noexcept : e1(VEC_X), e2(VEC_Y), e3(VEC_Z) {};

  /// @brief Constructor from three Vec
  constexpr ONB(Vec e1, Vec e2, Vec e3) noexcept : e1(e1), e2(e2), e3(e3) {};

  /// @brief Branchless constructor from a Vec (cast into e_3)
  /// @details Assumes the input Vec to be normalized. Based on the algorithm by Duff et al. (2017)
  /// @param vec normalized Vec = e_3
  constexpr ONB(Vec vec) noexcept : e3(vec) {
    float sign = std::copysignf(1.f, e3.z); // copysignf returns the absolute value of the first argument with the sign of the
                                            // second one (sign is set negative if e3.z=0)
    const float a = -1.f / (sign + e3.z);
    const float b = e3.x * e3.y * a;

    e1 = Vec(1.f + sign * e3.x * e3.x * a, sign * b, -sign * e3.x);
    e2 = Vec(b, sign + e3.y * e3.y * a, -e3.y);
  }

  // -------------------- Methods ----------------------
  /// @brief Returns true if it's a well formed ONB, false otherwise
  constexpr bool is_consistent() const noexcept {
    if (!are_close(e1 * e2, 0.f) || !are_close(e1 * e3, 0.f) || !are_close(e2 * e3, 0.f)) {
      return false;
    }
    if (!are_close(e1.squared_norm(), 1.f) || !are_close(e2.squared_norm(), 1.f) || !are_close(e3.squared_norm(), 1.f)) {
      return false;
    }
    return true;
  }
};

//-------------------------------------------------------------------------------------------------------------
//------------------------------- DEGREES TO RADIANS CONVERSION ----------------------
//-------------------------------------------------------------------------------------------------------------

inline constexpr float degs_to_rads(const float &angle_degs) noexcept { return angle_degs * std::numbers::pi_v<float> / 180.f; };
