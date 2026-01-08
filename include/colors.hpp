// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// LIBRARY FOR COLORS AND IMAGES
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#define _CRT_SECURE_NO_WARNINGS // disable warning due to strerr function in the external library (for MVSC)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" // external library for LDR images

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// ------------------------------------------------------------------------------------------------------------
// ----- CONSTANTS, ENDIANNESS, EXCEPTIONS, GLOBAL FUNCTIONS, FORWARD DECLARATIONS
// ------------------------------------------------------------------------------------------------------------

class Color;

constexpr float DEFAULT_ERROR_TOLERANCE = 1e-5f; // Default tolerance to decide if two float numbers are close
constexpr float DEFAULT_DELTA_LOG = 1e-10f;      // Default quantity added to the argument to prevent caluculating the
                                                 // logarithm of zero
constexpr float DEFAULT_AVG_LUMINOSITY_DARK_MODE =
    0.1f; // Default value replacing the average luminosity of the HDR image in tone mapping (reciprocal of exposure): to be used
          // for dark (almost-black) images. THe default value here provided (0.1) is fine as long as the non-dark portions of the
          // image have average luminosity of the same order of magnitude, which is often the case.

/// @brief Byte endianness (for floats)
enum class Endianness {
  little_endian, // encoded with +1 or any positive value
  big_endian     // encoded with -1 or any negative value
};

/// @brief Inspect device endianness
[[maybe_unused]]
constexpr Endianness inspect_device_endianness() {
  static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big,
                "Mixed endianness is not supported");

  if constexpr (std::endian::native == std::endian::little)
    return Endianness::little_endian;
  else
    return Endianness::big_endian;
}

/// @brief ad hoc error message/exception to throw when reading pfm files
class InvalidPfmFileFormat : public std::exception {
private:
  std::string error_message;

public:
  InvalidPfmFileFormat(const std::string &em) : error_message("Invalid PFM file format: " + em) {}

  const char *what() const noexcept override {
    // Convert to cstring for compatibility
    return error_message.c_str();
  }
};

/// @brief Check whether two floats are equal within a given tolerance
bool are_close(float x, float y, float error_tolerance = DEFAULT_ERROR_TOLERANCE) noexcept { return std::fabs(x - y) < error_tolerance; }

/// @brief normalize a float number (between 0 and 1) using the formula
/// x/(1+x) (almost x for small x, but saturates to 1 for large x)
/// @param x
/// @return
inline constexpr float _clamp(float x) noexcept { return x / (1.f + x); }

// ------------------------------------------------------------------------------------------------------------
// COLOR
// ------------------------------------------------------------------------------------------------------------

/// @brief content of each pixel (3 rgb floats) together with methods to
/// write/read them
class Color {
public:
  // Properties
  float r, g, b; // 32-bit for memory efficiency

  // Constructors
  constexpr Color() noexcept : r(0.f), g(0.f), b(0.f) {} // Default constructor (sets color to black)

  constexpr Color(float red, float green, float blue) noexcept // Constructor with externally assigned values
      : r(red), g(green), b(blue) {}

  // Methods

  /// @brief Check if this color is close to another color within a given tolerance
  bool is_close(const Color &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const noexcept {
    return (are_close(r, other.r, error_tolerance) && are_close(g, other.g, error_tolerance) &&
            are_close(b, other.b, error_tolerance));
  }

  /// @brief Check if two colors are close (symmetric syntax)
  friend bool are_close(const Color &color1, const Color &color2) noexcept { return color1.is_close(color2); }

  /// @brief Sum of two colors
  constexpr Color operator+(const Color &other) const noexcept { return Color(r + other.r, g + other.g, b + other.b); }

  /// @brief Compound addition assigmenent
  constexpr Color &operator+=(const Color &other) noexcept {
    *this = *this + other;
    return *this;
  }

  /// @brief Product of two colors
  constexpr Color operator*(const Color &other) const noexcept { return Color(r * other.r, g * other.g, b * other.b); }

  /// @brief Compound product assigmenent of two colors
  constexpr Color &operator*=(const Color &other) noexcept {
    *this = *this * other;
    return *this;
  }

  /// @brief Product of color and scalar
  constexpr Color operator*(float scalar) const noexcept { return Color(r * scalar, g * scalar, b * scalar); }

  /// @brief Compound product assigmenent of a color and a scalar
  constexpr Color &operator*=(float scalar) noexcept {
    *this = *this * scalar;
    return *this;
  }

  /// @brief Division of a color by a scalar
  constexpr Color operator/(float scalar) const noexcept { return *this * (1.f / scalar); }

  /// @brief Compound division assigment of a color by a scalar
  constexpr Color &operator/=(float scalar) noexcept {
    *this = *this / scalar;
    return *this;
  }

  /// @brief Friend function to allow multiplying a scalar and a color
  constexpr friend Color operator*(float scalar, const Color &my_color) noexcept {
    return my_color * scalar; // Reuse the member function
  }

  /// Helper method to create a string representing the color
  std::string to_string() const {
    std::ostringstream oss;
    oss << "Color(r: " << r << ", g: " << g << ", b: " << b << ")";
    return oss.str();
  }

  // Helper method to display the color
  void display() const { std::cout << to_string() << std::endl; }

  /// @brief luminosity of the color (computed using Shirley & Morley
  /// formula)
  constexpr float luminosity() const noexcept {
    return 0.5f * (std::min({r, g, b}) + std::max({r, g, b})); // Shirley & Morley's formula (empirically
                                                               // best formula for luminosity)
  }

  /// @brief luminosity of the color (computed as arithmetic average of rgb,
  /// instead of Shirley & Morley formula)
  constexpr float luminosity_arithemic_avg() const noexcept { return (r + g + b) / 3.f; }
};

// ------------------------------------------------------------------------------------------------------------
// HDR IMAGE UTILS
// ------------------------------------------------------------------------------------------------------------
// The following 5 functions (_write_float, _read_float, _read_line, _parse_img_size, _parse_endianness) are helper functions for
// the methods HdrImage::write_pfm and HdrImage::read_pfm_file

/// @brief Takes a float and return its 4 bytes into the stream
/// @param stream
/// @param value
/// @param endianness
void _write_float(std::ostream &stream, float value, Endianness endianness) {
  // Convert "value" in a sequence of 32 bits

  uint32_t double_word = std::bit_cast<uint32_t>(value);
  // uint32_t double_word{*((uint32_t *)&value)}; // C-style solution, violates C++'s aliasing rules

  // Extract the four bytes in "double_word" using bit-level operators
  uint8_t bytes[] = {
      static_cast<uint8_t>(double_word & 0xFF), // Least significant byte
      static_cast<uint8_t>((double_word >> 8) & 0xFF), static_cast<uint8_t>((double_word >> 16) & 0xFF),
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

/// @brief Reads a stream of bytes and convert them to a float
/// @param stream
/// @param endianness
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
  uint32_t double_word{(static_cast<uint32_t>(bytes[0]) << 0) | (static_cast<uint32_t>(bytes[1]) << 8) |
                       (static_cast<uint32_t>(bytes[2]) << 16) | (static_cast<uint32_t>(bytes[3]) << 24)};

  // float value{*((float *)&double_word)}; // C-style solution, violates C++'s aliasing rules
  float value = std::bit_cast<float>(double_word);
  return value;
}

/// @brief Read the line of bytes already converting into ASCII
/// @param stream
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

/// @brief Parse image dimensions (columns, rows) from the appropriate line of a PFM file header
/// @param line to parse
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

/// @brief Parse endianness from the appropriate line of a PFM file header
/// @param line
Endianness _parse_endianness(const std::string &line) {
  std::istringstream iss(line);
  float value = 0.f;
  if (!(iss >> value)) {
    // Fed value is not a valid float
    throw InvalidPfmFileFormat("Missing endianness specification");
  }

  if (value == 0.f) {
    throw InvalidPfmFileFormat("Invalid endianness specification");
  } else if (value < 0.f) {
    return Endianness::little_endian;
  } else if (value > 0.f) {
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

/// @brief Class for storing images in HDR format (3 rgb floats for each pixel)
class HdrImage {

private:
  /// @brief Read a PFM file and create the corresponding HDR image
  void read_pfm_file(std::istream &stream) {
    // Read and validate the magic bytes (first bytes in a binary file are
    // usually called «magic bytes»)
    std::string magic = _read_line(stream);
    if (magic != "PF") {
      throw InvalidPfmFileFormat("Invalid magic in PFM file");
    }

    // Read the image size line and extract width and height
    std::string img_size_line = _read_line(stream);
    int w, h;
    std::tie(w, h) = _parse_img_size(img_size_line);

    // Read the endianness specification line and parse it
    std::string endianness_line = _read_line(stream);
    Endianness endianness = _parse_endianness(endianness_line);

    // Create the image by reading pixel data from bottom to top (PFM files store
    // scanlines in reverse order)

    width = w;
    height = h;
    pixels = std::vector<Color>(static_cast<size_t>(width * height), Color());

    float r, g, b;

    // Scan the image initializing each pixel as in the pfm file
    for (int y = height - 1; y >= 0; y--) {
      for (int x = 0; x < width; x++) {
        // Read three floats for the pixel (red, green, blue)
        r = _read_float(stream, endianness);
        g = _read_float(stream, endianness);
        b = _read_float(stream, endianness);

        if (stream.eof()) {
          throw InvalidPfmFileFormat("Fewer pixels than expected");
        }

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

  HdrImage(int32_t w, int32_t h) : width(w), height(h) {
    if (w <= 0 || h <= 0) {
      throw std::invalid_argument("HdrImage dimensions must be positive");
    }
    pixels.resize(static_cast<size_t>(w * h), Color());
  }

  /// @brief PFM file --> HDR image
  /// @param inupt stream
  explicit HdrImage(std::istream &stream) { read_pfm_file(stream); }

  /// @brief PFM file --> HDR image
  /// @param inupt file name
  explicit HdrImage(const std::string &file_name) {
    std::ifstream stream(file_name, std::ios::binary); // Open the file in binary mode
    if (!stream.is_open()) {
      std::string error_msg = "Failed to open file \"" + file_name + "\"";
      if (errno) {                                        // errno is a system-specific number that identifies an error occured
        error_msg += ": " + std::string(strerror(errno)); // convert errno to the string describing the message
      }
      throw std::runtime_error(error_msg);
    }

    read_pfm_file(stream);
    stream.close();
  }

  // Methods (those with a leading underscore are intended to be private)

  void write_pfm(std::ostream &stream, Endianness endianness = Endianness::little_endian) {
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

  void write_pfm(const std::string &filename) {
    std::ofstream of(filename);
    write_pfm(of);
    of.close();
  }

  /// @brief Check if row and col are nonnegative and within bounds
  bool _valid_indexes(int32_t col, int32_t row) const { return (col >= 0 && row >= 0 && col < width && row < height); }

  /// @brief Return the index of the pixels vector corresponding to the given matrix row and column, using row-major ordering
  int32_t _pixel_offset(int32_t col, int32_t row) const {
    assert(_valid_indexes(col, row) && "Invalid indices in _pixel_offset");
    return row * width + col;
  }

  /// @brief Return color at given column and row
  Color get_pixel(int32_t col, int32_t row) const {
    // Assign _pixel_offset to a variable (useful for debugging)
    int32_t offset = _pixel_offset(col, row);
    return pixels[offset];
  }

  /// @brief Set color at given column and row
  void set_pixel(int32_t col, int32_t row, const Color &c) {
    int32_t offset = _pixel_offset(col, row);
    pixels[offset] = c;
  }

  /// @brief Compute the average luminosity of the image
  /// @param delta (default value to prevent taking log(0))
  /// @return return the average luminosity of the image as float
  float average_luminosity(float delta = DEFAULT_DELTA_LOG) const noexcept {
    float cumsum = 0.f;
    for (auto pixel : pixels) {
      cumsum += std::log10(delta + pixel.luminosity());
    }

    return std::pow(10.f, cumsum / pixels.size());
  }

  /// @brief Normalize image applying the same factor to all pixel rgb values
  /// @param alpha factor
  /// @param optional value to replace average pixel luminosity (useful in case of dark images)
  void normalize_image(float alpha, std::optional<float> avg_luminosity = std::nullopt) noexcept {
    float avg_lum = avg_luminosity.value_or(average_luminosity());

    for (auto &pixel : pixels) {
      pixel.r = pixel.r * (alpha / avg_lum);
      pixel.g = pixel.g * (alpha / avg_lum);
      pixel.b = pixel.b * (alpha / avg_lum);
    }
  }

  /// @brief Clamp the image rgb values between 0 and 1
  void clamp_image() noexcept {
    for (auto &pixel : pixels) {
      pixel.r = _clamp(pixel.r);
      pixel.g = _clamp(pixel.g);
      pixel.b = _clamp(pixel.b);
    }
  }

  /// @brief Take a normalized and clamped HDR image, apply gamma correction &
  /// produce LDR image in PNG format
  /// @param file name of LDR image to be produced
  /// @param gamma correction factor (default 1.0)
  void write_ldr_image(const std::string &filename, float gamma = 1.f) const {
    // Create a buffer to hold all pixels as 8-bit unsigned integers (R, G, B)
    // Each pixel needs 3 bytes, so we reserve enough memory up front
    std::vector<uint8_t> buffer;
    buffer.reserve(width * height * 3);

    // Lambda function to:
    // - Apply gamma correction
    // - Convert float [0,1] to uint8_t [0,255]
    auto transform_to_LDR = [gamma](float x) {
      float gamma_corrected = std::pow(x, 1.f / gamma);                 // Apply gamma correction
      return static_cast<uint8_t>(std::round(gamma_corrected * 255.f)); // Scale and convert
    };

    // Note that PNG and JPEG images are filled in from left to right, from top to bottom.
    // You might need to change buffer order if you want to support other LDR formats.
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        const Color &c = get_pixel(x, y);

        // Convert and store the R, G, B values in the buffer
        buffer.push_back(transform_to_LDR(c.r));
        buffer.push_back(transform_to_LDR(c.g));
        buffer.push_back(transform_to_LDR(c.b));
      }
    }

    // Save as PNG — stride = width * 3 bytes per row
    stbi_write_png(filename.c_str(), width, height, 3, buffer.data(), width * 3);
  }
};

//-------------------------------------------------------------------------------------------------------------
//---------- PREDEFINED COLORS ------------------
//-------------------------------------------------------------------------------------------------------------

inline constexpr Color BLACK(0.0f, 0.0f, 0.0f);
inline constexpr Color WHITE(1.0f, 1.0f, 1.0f);
inline constexpr Color RED(1.0f, 0.0f, 0.0f);
inline constexpr Color GREEN(0.0f, 1.0f, 0.0f);
inline constexpr Color BLUE(0.0f, 0.0f, 1.0f);
inline constexpr Color YELLOW(1.0f, 1.0f, 0.0f);
inline constexpr Color PURPLE(1.0f, 0.0f, 1.0f);
inline constexpr Color CYAN(0.0f, 1.0f, 1.0f);
