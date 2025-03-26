// Libraries
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include <sstream>


// Constants
constexpr double DEFAULT_ERROR_TOLERANCE =
    1e-5; // Default tolerance to decide if two float numbers are close

// Namespaces? Avoid for now


class Color {
public:
  // Properties
  float r, g, b; // Use 32-bit format to avoid memory waste

  // Constructors
  Color()
      : r(0.0f), g(0.0f), b(0.0f) {
  } // Default constructor (sets color to black)

  Color(float red, float green, float blue) // Constructor with externally assigned values
      : r(red), g(green), b(blue) {}

  // Methods
  // Check if color is close to another color within some tolerance.
  // If no tolerance is provided, DEFAULT_ERROR_TOLERANCE is used.
  bool is_close_to(const Color &other,
                   float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
    return (std::fabs(r - other.r) < error_tolerance &&
            std::fabs(g - other.g) < error_tolerance &&
            std::fabs(b - other.b) < error_tolerance);
  }

  // Check if two colors are close calling is_close_to on the first one
  // (symmetric syntax)
  friend bool are_close(const Color &color1, const Color &color2) {
    return color1.is_close_to(color2);
  }

  // Sum of two colors
  Color operator+(const Color &other) const {
    return Color(r + other.r, g + other.g, b + other.b);
  }

  // Product of two colors
  Color operator*(const Color &other) const {
    return Color(r * other.r, g * other.g, b * other.b);
  }

  // Product: color * scalar
  Color operator*(float scalar) const {
    return Color(r * scalar, g * scalar, b * scalar);
  }

  // Friend function to allow commutative product: scalar * color
  friend Color operator*(float scalar, const Color &my_color) {
    return my_color * scalar; // Reuse the member function
  }

  // Helper method to display the color.
  void display() const {
    std::cout << "r: " << r << " g: " << g << " b: " << b;
  }
};


class HdrImage {
public:
  // Properties
  int32_t width, height; // Use int32_t format to allow large dimensions (and
                         // negative values for tests)
  std::vector<Color> pixels;

  // Constructors
  // Default constructor: if dimensions are positive, initializes the pixels
  // vector with width*height copies of Color(); otherwise, creates an empty
  // vector.
  HdrImage(int32_t w, int32_t h)
      : height(w), width(h),
        pixels((w > 0 && h > 0) ? static_cast<size_t>(w * h) : 0, Color()) {}

  // Methods
  // Some methods should be private, but following the convention are declared
  // public with underscore (_) in front.

  void read_pfm_image(std::istream& is){
    ///
  };

  // _valid_indexes returns true if row and col are nonnegative and within
  // bounds.
  bool _valid_indexes(int32_t col, int32_t row) const {
    return (col >= 0 && row >= 0 && col < height && row < width);
  }

  // _pixel_offset returns the index of the pixels vector corresponding to the
  // given matrix row and column, using row-major ordering. Uses assert to
  // ensure that the provided indices are valid.
  int32_t _pixel_offset(int32_t col, int32_t row) const {
    // assert(_valid_indexes(row, col) && "Invalid indices in _pixel_offset");
    return row * width + col;
  }

  // get_pixel returns the color at the given row and column.
  Color get_pixel(int32_t col, int32_t row) const {
    int32_t offset = _pixel_offset(
        col, row); // Assign _pixel_offset to a variable (useful for debugging)
    return pixels[offset];
  }

  // set_pixel sets the pixel at the given row and column to the provided color.
  void set_pixel(int32_t col, int32_t row, const Color &c) {
    int32_t offset = _pixel_offset(
        col, row); // Assign _pixel_offset to a variable (useful for debugging)
    pixels[offset] = c;
  }
};



enum class Endianness { little_endian, big_endian };


class InvalidPfmFileFormat : public std::exception {
  public:
  // Constructor
  InvalidPfmFileFormat(std::string em) : error_message(em){}

  // Methods
  const char* what() const noexcept override {
    return error_message.c_str();
  }

  // Property
  std::string error_message;
};




/// @brief Takes a float and return its 4 bytes into the stream (NO TEST NEEDED)
/// @param stream 
/// @param value 
/// @param endianness 
void _write_float(std::ostream &stream, float value, Endianness endianness) {
  // Convert "value" in a sequence of 32 bit
  uint32_t double_word{*((uint32_t *)&value)};

  // Extract the four bytes in "double_word" using bit-level operators
  uint8_t bytes[] = {
    static_cast<uint8_t>(double_word & 0xFF),         // Least significant byte
    static_cast<uint8_t>((double_word >> 8) & 0xFF),
    static_cast<uint8_t>((double_word >> 16) & 0xFF),
    static_cast<uint8_t>((double_word >> 24) & 0xFF), // Most significant byte
  };

  switch (endianness) {
    case Endianness::little_endian:
      for (int i{}; i < 4; ++i)    // Forward loop
        stream << bytes[i];
    break;

    case Endianness::big_endian:
      for (int i{3}; i >= 0; --i)  // Backward loop
        stream << bytes[i];
    break;
  }
}

/// @brief Reads a stream of bytes and convert them to floats (NO TEST NEEDED)
/// @param stream 
/// @param endianness 
/// @return 
float read_float(std::istream stream, Endianness endianness){
  uint8_t bytes[4];

  switch (endianness) {
    case Endianness::little_endian:
      for (int i{}; i < 4; ++i)   // Forward loop
        stream >> bytes[i];
    break;

    case Endianness::big_endian:
      for (int i{3}; i >= 0; --i)  // Backward loop
        stream >> bytes[i];
    break;
  }
  uint32_t double_word{
    (static_cast<uint32_t>(bytes[0]) << 0)
    | (static_cast<uint32_t>(bytes[1]) << 8)
    | (static_cast<uint32_t>(bytes[2]) << 16)
    | (static_cast<uint32_t>(bytes[3]) << 24)
  };
  //float value{*((float *)&double_word)}; // This line has the same effect as the line below
  float value;
  std::memcpy(&value, &double_word, sizeof(float));

  return value;
}

/// @brief Read the line already converting into ascii (MISSING TESTS)
/// @param stream 
/// @return 
std::string _read_line(std::ifstream& stream){
  std::string result;
  char cur_byte;

  while(stream.get(cur_byte)){
    if(cur_byte == '\n') { return result; }
    result += cur_byte;
  }
  return result;
}

/// you might want to handle float entries a bit better
/// @brief read the image dimensions (columns, rows) from a line (of a pfm file ideally) (TEST NEEDED)
/// @param line 
/// @return 
std::pair<int, int> _parse_img_size(const std::string& line) {
  std::istringstream iss(line);
  int width, height;

  // Read two integers
  if (!(iss >> width >> height)) {
    throw InvalidPfmFileFormat("Invalid image size specification");
  }


  // Ensure no extra characters after the numbers
  std::string leftover;
  if (iss >> leftover) {
    throw InvalidPfmFileFormat("Too many elements in image size specification");
  }

  // Validate width and height are strinctly positive
  if (width < 0 || height < 0) {
    throw InvalidPfmFileFormat("Invalid width/height");
  }

  return {width, height};
}


Endianness _parse_endianness(const std::string& line){
  std::istringstream iss(line);
  float value;
  if (!(iss >> value)) {
    //throw InvalidPfmFileFormat("Missing endianness specification");
  }


  if(value == 0) { //throw InvalidPfmFileFormat("Invalid endianness specification, it cannot be zero");}
  else if(value < 0) { return Endianness::little_endian; }
  else if(value > 0) { return Endianness::big_endian; }
}






// ECCEZIONI

// struct InvalidPfmFileFormat : public std::runtime_error {
// using std::runtime_error::runtime_error;
// };
