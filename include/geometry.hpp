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

  /// convert vector to a normal
  Normal to_normal() const { return Normal(x, y, z); }

  /// convert vector to a point
  Point to_point() const { return Point(x, y, z); }
};

// ------------------------------------------------------------------------------------------------------------
// POINT CLASS
// ------------------------------------------------------------------------------------------------------------

class Point {
public:
  //-------Properties--------

  float x, y, z;

  //-----------Constructors-----------

  /// Default constructor initializes to (0, 0, 0)
  Point() : x(0), y(0), z(0) {}

  /// Constructor with parameters
  Point(float x, float y, float z) : x(x), y(y), z(z) {}

  //------------Methods-----------

  /// convert point to string
  std::string to_string() const {
    std::ostringstream oss;
    oss << "(" << x << ", " << y << ", " << z << ")";
    return oss.str();
  }

  /// print point to screen
  void print() const { std::cout << to_string() << std::endl; }

  /// check if two points are close
  bool is_close(const Point &other,
                float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
    return are_close(x, other.x, error_tolerance) &&
           are_close(y, other.y, error_tolerance) &&
           are_close(z, other.z, error_tolerance);
  }

  /// convert point to a vector
  Vec to_vector() const { return Vec(x, y, z); }

};

// ------------------------------------------------------------------------------------------------------------
// -------------NORMAL CLASS ----------------------------
// ------------------------------------------------------------------------------------------------------------

class Normal {
public:
  //-------Properties--------

  float x, y, z;

  //-----------Constructors-----------

  /// Default constructor initializes to (0, 0, 0)
  Normal() : x(0), y(0), z(0) {}

  /// Constructor with parameters
  Normal(float x, float y, float z) : x(x), y(y), z(z) {}

  //------------Methods-----------

  /// convert normal to string
  std::string to_string() const {
    std::ostringstream oss;
    oss << "(" << x << ", " << y << ", " << z << ")";
    return oss.str();
  }

  /// print normal to screen
  void print() const { std::cout << to_string() << std::endl; }

  /// check if two normals are close
  bool is_close(const Normal &other,
                float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
    return are_close(x, other.x, error_tolerance) &&
           are_close(y, other.y, error_tolerance) &&
           are_close(z, other.z, error_tolerance);
  }

  /// return negative normal
  Normal negative() const { return Normal(-x, -y, -z); }

  /// return squared norm of normal
  float squared_norm() const { return x * x + y * y + z * z; }

  /// return norm of normal
  float norm() const { return std::sqrt(squared_norm()); }

  /// normalize the normal 
  Normal normalize() const {
    float n = norm();
    if (n == 0) {
      throw std::runtime_error("Cannot normalize a zero normal");
    }
    return Normal(x / n, y / n, z / n);
  }

  /// convert normal to a vector
  Vec to_vector() const { return Vec(x, y, z); }
};

//---------------------------------------------------------------
//--------MIXED OPERATIONS BETWEEN GEOMETRIC OBJECTS----------------------
//---------------------------------------------------------------

/// Generic sum operation `In1 + In2 → Out`
template <typename In1, typename In2, typename Out>
Out _sum(const In1 &a, const In2 &b) {
  return Out{a.x + b.x, a.y + b.y, a.z + b.z};
}

/// sum of two vectors, returning a vector
Vec operator+(const Vec &a, const Vec &b) { return _sum<Vec, Vec, Vec>(a, b); }

/// sum of a point and a vector, returning a point
Point operator+(const Point &a, Vec &b) {
  return _sum<Point, Vec, Point>(a, b);
}

/// Generic difference operation `In1 - In2 → Out`
template <typename In1, typename In2, typename Out>
Out _difference(const In1 &a, const In2 &b) {
  return Out{a.x - b.x, a.y - b.y, a.z - b.z};
}

/// difference of two vectors, returning a vector
Vec operator-(const Vec &a, const Vec &b) {
  return _difference<Vec, Vec, Vec>(a, b);
}

/// difference of a point and a vector, returning a point
Point operator-(const Point &a, Vec &b) {
  return _difference<Point, Vec, Point>(a, b);
}

/// difference of a point and a point, returning a vector
Vec operator-(const Point &a, const Point &b) {
  return _difference<Point, Point, Vec>(a, b);
}

/// Generic dot product operation `In1 - In2 → Out`
template <typename In1, typename In2, typename Out>
Out _dot_product(const In1 &a, const In2 &b) {
  return Out{a.x * b.x + a.y * b.y + a.z * b.z};
}

/// dot product of two vectors
float operator*(const Vec &a, const Vec &b) {
  return _dot_product<Vec, Vec, float>(a, b);
}

/// dot product of a vector and a normal
float operator*(const Vec &a, const Normal &b) {
  return _dot_product<Vec, Normal, float>(a, b);
}

/// dot product of a normal and a vector
float operator*(const Normal &a, const Vec &b) {
  return _dot_product<Normal, Vec, float>(a, b);
}

/// dot product of two normals
float operator*(const Normal &a, const Normal &b) {
  return _dot_product<Normal, Normal, float>(a, b);
}

/// Generic cross product operation `In1 x In2 → Out`
template <typename In1, typename In2, typename Out>
Out _cross_product(const In1 &a, const In2 &b) {
  return Out{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
             a.x * b.y - a.y * b.x};
}

/// cross product of two vectors, returning a vector
Vec operator^(const Vec &a, const Vec &b) {
  return _cross_product<Vec, Vec, Vec>(a, b);
}

/// cross product of a vector and a normal, returning a vector
Vec operator^(const Vec &a, const Normal &b) {
  return _cross_product<Vec, Normal, Vec>(a, b);
}

/// cross product of a normal and a vector, returning a vector
Vec operator^(const Normal &a, const Vec &b) {
  return _cross_product<Normal, Vec, Vec>(a, b);
}

/// cross product of two normals, returning a vector
Vec operator^(const Normal &a, const Normal &b) {
  return _cross_product<Normal, Normal, Vec>(a, b);
}

///Generic right scalar multiplication operation `In * scalar → Out`
template <typename In1, typename In2, typename Out>
Out _scalar_multiplication(const In1 &a, const In2 &b) {
  return Out{a.x * b, a.y * b, a.z * b};
}

///Generic left scalar multiplication operation `scalar * In→ Out`
template <typename In1, typename In2, typename Out>
Out _scalar_multiplication(const In1 &a, const In2 &b) {
  return Out{a * b.x, a * b.y, a * b.z};
}

/// right scalar multiplication of a vector and a float, returning a vector
Vec operator*(const Vec &a, float b) {
  return _scalar_multiplication<Vec, float, Vec>(a, b);
}

/// left scalar multiplication of a float and a vector, returning a vector
Vec operator*(float a, const Vec &b) {
  return _scalar_multiplication<float, Vec, Vec>(a, b);
}

/// right scalar multiplication of a normal and a float, returning a normal
Normal operator*(const Normal &a, float b) {
  return _scalar_multiplication<Normal, float, Normal>(a, b);
}

/// left scalar multiplication of a float and a normal, returning a normal
Normal operator*(float a, const Normal &b) {
  return _scalar_multiplication<float, Normal, Normal>(a, b);
}

//TODO complete Transformation class
//-------------------------------------------------------------------------------------------------------------
//------------------------------- TRANSFORMATION CLASS ------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
class Transformation {
public:
  //-------Properties--------
  

  //-----------Constructors-----------
  

  //------------Methods-----------

};


//TODO complete HomMatrix class
//-------------------------------------------------------------------------------------------------------------
//------------------------------- HOM_MATRIX CLASS ------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
class HomMatrix {
  public:
    //-------Properties--------
    
  
    //-----------Constructors-----------
    
  
    //------------Methods-----------
  
  };