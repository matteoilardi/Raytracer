// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// LIBRARY FOR GEOMETRIC OPERATIONS ON IMAGES
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

// NOTE we might want to change the external library (if so, let's decide soon)
#include "colors.hpp"
#include "stb_image_write.h" //external library for LDR images

// ------------------------------------------------------------------------------------------------------------
// VECTOR CLASS
// ------------------------------------------------------------------------------------------------------------

class Vec {
public:
  //-------Properties--------
  float x, y, z;

  //-----------Constructors-----------

  /// Default constructor initializes to (0, 0, 0)
  Vec() : x(0), y(0), z(0) {}

  /// Constructor with parameters
  Vec(float x, float y, float z) : x(x), y(y), z(z) {}

  //--------------------Methods----------------------

  /// return negative vector
  Vec negative() const { return Vec(-x, -y, -z); }

  /// return squared norm of vector
  float squared_norm() const { return x * x + y * y + z * z; }

  /// return norm of vector
  float norm() const { return std::sqrt(squared_norm()); }

  /// normalize the vector
  Vec normalize() const {
    float n = norm();
    if (n == 0) {
      throw std::runtime_error("Cannot normalize a zero vector");
    }
    return Vec(x / n, y / n, z / n);
  }

  /// convert vector to string
  std::string to_string() const {
    std::ostringstream oss;
    oss << "(" << x << ", " << y << ", " << z << ")";
    return oss.str();
  }

  /// print vector to screen
  void print() const { std::cout << to_string() << std::endl; }

  /// check if two vectors are close
  bool is_close(const Vec &other,
                float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
    return are_close(x, other.x, error_tolerance) &&
           are_close(y, other.y, error_tolerance) &&
           are_close(z, other.z, error_tolerance);
  }

  ///convert vector to a normal
  //TODO insert this method when Normal class exists
};

//---------------------------------------------------------------
//--------MIXED OPERATIONS BETWEEN GEOMETRIC OBJECTS----------------------
//---------------------------------------------------------------

/// Generic sum operation `In1 + In2 → Out`
template <typename In1, typename In2, typename Out>
Out _sum(const In1 &a, const In2 &b) {
  return Out{a.x + b.x, a.y + b.y, a.z + b.z};
}

///sum of two vectors (using the generic sum operation)
Vec operator+(const Vec &a, const Vec &b) {
  return _sum<Vec, Vec, Vec>(a, b);
}

//TODO: sum of vector and a point

/// Generic difference operation `In1 - In2 → Out`
template <typename In1, typename In2, typename Out>
Out _difference(const In1 &a, const In2 &b) {
  return Out{a.x - b.x, a.y - b.y, a.z - b.z};
}

///difference of two vectors (using the generic difference operation)
Vec operator-(const Vec &a, const Vec &b) {
  return _difference<Vec, Vec, Vec>(a, b);
}

//TODO: difference of a point and vector

/// Generic dot product operation `In1 - In2 → Out`
template <typename In1, typename In2, typename Out>
Out _dot_product(const In1 &a, const In2 &b) {
  return Out {a.x * b.x + a.y * b.y + a.z * b.z};
}

/// dot product of two vectors (using the generic dot product operation)
float operator*(const Vec &a, const Vec &b) {
  return _dot_product<Vec, Vec, float>(a, b);
}

//TODO: dot product of vector and normal

/// Generic cross product operation `In1 x In2 → Out`
template <typename In1, typename In2, typename Out>
Out _cross_product(const In1 &a, const In2 &b) {
  return Out{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
             a.x * b.y - a.y * b.x};
}

/// cross product of two vectors (using the generic cross product operation)
Vec operator^(const Vec &a, const Vec &b) {
    return _cross_product<Vec, Vec, Vec>(a,b);
}

//TODO: cross product of vector and normal
