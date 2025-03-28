// Libraries
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <bit>
#include <array>

// ------------------------------------------------------------------------------------------------------------
// CONSTANTS, ENDIANNESS, EXCEPTIONS
// ------------------------------------------------------------------------------------------------------------

constexpr double DEFAULT_ERROR_TOLERANCE =
    1e-5; // Default tolerance to decide if two float numbers are close

/// @brief endianness is order you read floats with (recall 32_bit_float =4
/// bytes) (left to right or right to left)
enum class Endianness {
  little_endian, // encoded with +1 or any positive value
  big_endian     // encoded with -1 or any negative value
};

/// @brief ad hoc error message/exception to throw when reading pfm files
class InvalidPfmFileFormat : public std::exception {
public:
  // Properties
  std::string error_message;

  // Constructor (just takes as input the desired error message)
  InvalidPfmFileFormat(std::string em) : error_message(em) {}

  // Methods

  // override of the what() method from parent class std::exception
  const char *what() const noexcept override {
    // just a convert string format of C++ into cstring format of C (needed for
    // compatibility)
    return error_message.c_str();
  }
};

// ------------------------------------------------------------------------------------------------------------
// COLOR
// ------------------------------------------------------------------------------------------------------------

/// @brief content of each pixel (3 rgb floats) together with methods to
/// write/read them
class Color {
public:
  // Properties
  float r, g, b; // Use 32-bit format to avoid memory waste

  // Constructors

  Color()
      : r(0.0f), g(0.0f), b(0.0f) {
  } // Default constructor (sets color to black)

  Color(float red, float green,
        float blue) // Constructor with externally assigned values
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

// ------------------------------------------------------------------------------------------------------------
// HDR IMAGE UTILS
// ------------------------------------------------------------------------------------------------------------

// The following 5 functions (_write_float, _read_float, _read_line,
// _parse_img_size, _parse_endianness)
//  are used to implement the 'read_pfm_file' method of the 'HdrImage' class
//  you might want to move them somewhere else (e.g. inside HdrImage?)

/// @brief Takes a float and return its 4 bytes into the stream (NO TEST NEEDED)
/// @param stream
/// @param value
/// @param endianness
void _write_float(std::ostream &stream, float value, Endianness endianness) {
  // Convert "value" in a sequence of 32 bits
  uint32_t double_word{*((uint32_t *)&value)};

  // Extract the four bytes in "double_word" using bit-level operators
  uint8_t bytes[] = {
      static_cast<uint8_t>(double_word & 0xFF), // Least significant byte
      static_cast<uint8_t>((double_word >> 8) & 0xFF),
      static_cast<uint8_t>((double_word >> 16) & 0xFF),
      static_cast<uint8_t>((double_word >> 24) & 0xFF), // Most significant byte
  };

  switch (endianness) {
  case Endianness::little_endian:
    for (int i{}; i < 4; ++i) // Forward loop
      stream << bytes[i];
    break;

  case Endianness::big_endian:
    for (int i{3}; i >= 0; --i) // Backward loop
      stream << bytes[i];
    break;
  }
}

/// @brief Reads a stream of bytes and convert them to floats (NO TEST NEEDED)
/// @param stream
/// @param endianness
/// @return
float _read_float(std::istream &stream, Endianness endianness) {
  uint8_t bytes[4];

  switch (endianness) {
  case Endianness::little_endian:
    for (int i{0}; i < 4; ++i) // Forward loop
      stream >> std::noskipws >> bytes[i];
    break;

  case Endianness::big_endian:
    for (int i{3}; i >= 0; --i) // Backward loop
      stream >> std::noskipws >> bytes[i];
    break;
  }
  uint32_t double_word{(static_cast<uint32_t>(bytes[0]) << 0) |
                       (static_cast<uint32_t>(bytes[1]) << 8) |
                       (static_cast<uint32_t>(bytes[2]) << 16) |
                       (static_cast<uint32_t>(bytes[3]) << 24)};
  float value{*((float *)&double_word)}; // This line has the same effect as
  // the line below
  //float value;
  //std::memcpy(&value, &double_word, sizeof(float));

  return value;
}

/// @brief Read the line of bytes already converting into ascii (MISSING TESTS)
/// @param stream
/// @return
std::string _read_line(std::istream &stream) {
  std::string result = "";
  char cur_byte;

  while (stream.get(cur_byte)) {
    if (cur_byte == '\n') {
      return result;
    }
    result += cur_byte;
  }
  return result;
}

/// @brief read the image dimensions (columns, rows) from a line (of a pfm file
/// ideally) (TEST NEEDED) you might want to handle float entries a bit better
/// @param line
/// @return
std::pair<int, int> _parse_img_size(const std::string &line) {
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

/// @brief read the endianness from a pfm file
/// @param line
/// @return
Endianness _parse_endianness(const std::string &line) {
  std::istringstream iss(line);
  float value = 0.0;
  if (!(iss >> value)) // Note: if value is fed e. g. "abc", the conversion
                       // fails and value remains unchanged!
  {
    throw InvalidPfmFileFormat("Missing endianness specification");
  }

  if (value == 0.0) {
    throw InvalidPfmFileFormat("Invalid endianness specification");
  } else if (value < 0.0) {
    return Endianness::little_endian;
  } else if (value > 0.0) {
    return Endianness::big_endian;
  }

  // This line will never be reached (all possible cases are covered and the
  // function always return an Endianness) but the compiler does not realize
  // this and raise a warning otherwise
  throw std::logic_error("Unreachable code reached in _parse_endianness");
}

// ------------------------------------------------------------------------------------------------------------
// HDR IMAGE
// ------------------------------------------------------------------------------------------------------------

/// @brief class for images in hdr format (3 rgb floats for each pixel), pixel
/// matrix converted into a vector
class HdrImage {

private:

  /// @brief Read a pfm file and create the corresponding hdr image
  /// based on the 5 functions below (_write_float, _read_float, _read_line,
  /// _parse_img_size, _parse_endianness) which you might want to move here
  /// (inside HdrImage?)
  void read_pfm_file(std::istream &stream) {
    // Read and validate the magic bytes (first bytes in a binary file are
    // usually called «magic bytes»)
    std::string magic = _read_line(stream);
    if (magic != "PF") {
      throw InvalidPfmFileFormat("invalid magic in PFM file");
    }

    // Read the image size line and extract width and height.
    std::string img_size_line = _read_line(stream);
    int w, h;
    std::tie(w, h) = _parse_img_size(img_size_line);

    // Read the endianness specification line and parse it.
    std::string endianness_line = _read_line(stream);
    Endianness endianness = _parse_endianness(endianness_line);

    // Create the image by reading pixel data from bottom to top.
    // (PFM files store scanlines in reverse order.)

    width = w;
    height = h;
    pixels = std::vector<Color>(static_cast<size_t>(width * height), Color());

    float r, g, b;

    // scan the image initializing each pixel as in the pfm file
    for (int y = height - 1; y >= 0; y--) {
      for (int x = 0; x < width; x++) {
        // Read three floats for the pixel (red, green, blue).
        r = _read_float(stream, endianness);
        g = _read_float(stream, endianness);
        b = _read_float(stream, endianness);

        if(stream.eof()) { throw InvalidPfmFileFormat("Fewer pixels than expected"); }

        // Set the pixel at position (x, y) with the color just read
        set_pixel(x, y, Color(r, g, b));
      }
    }

    // Ensure nothing more is left to read
    std::string leftover;
    if (stream >> leftover) {
      throw InvalidPfmFileFormat("More pixels than expected");
    }
  }

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
      : width(w), height(h),
        pixels((w > 0 && h > 0) ? static_cast<size_t>(w * h) : 0, Color()) {}

  // First constructor pfm file --> Hdr image
  // Read a PFM file from a stream invoking `read_pfm_file` method
  HdrImage(std::istream &stream) { read_pfm_file(stream); }

  // Second constructor pfm file --> Hdr image
  // Open a PFM file and read the stream of bytes from it, again invoking
  // `read_pfm_file` method
  HdrImage(const std::string &file_name) {
    std::ifstream stream(file_name);
    if (!stream.is_open()) {
      throw std::ios_base::failure("Failed to open file: " + file_name);
    }

    read_pfm_file(stream);
    stream.close();
  }

  // Methods
  // Some other methods should be private, but following the convention are
  // declared public with underscore (_) in front.

  void write_pfm(std::ostream &stream, Endianness endianness) {
    std::string endianness_str;
    if (endianness == Endianness::little_endian) {
      endianness_str = "-1.0";
    } else if (endianness == Endianness::big_endian) {
      endianness_str = "1.0";
    }

    // Write header
    stream << "PF\n";
    stream << width << " " << height << '\n';
    stream << endianness_str << '\n';

    // Write pixels
    for (int y = height - 1; y >= 0; y--) {
      for (int x = 0; x < width; x++) {
        Color color = get_pixel(x, y);
        _write_float(stream, color.r, endianness);
        _write_float(stream, color.g, endianness);
        _write_float(stream, color.b, endianness);
      }
    }
  }

  // _valid_indexes returns true if row and col are nonnegative and within
  // bounds.
  bool _valid_indexes(int32_t col, int32_t row) const {
    return (col >= 0 && row >= 0 && col < width && row < height);
  }

  // _pixel_offset returns the index of the pixels vector corresponding to the
  // given matrix row and column, using row-major ordering. Uses assert to
  // ensure that the provided indices are valid.
  int32_t _pixel_offset(int32_t col, int32_t row) const {
    // assert(_valid_indexes(col, row) && "Invalid indices in _pixel_offset");
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
