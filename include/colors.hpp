// Libraries
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>

// Constants
constexpr double DEFAULT_ERROR_TOLERANCE = 1e-5;  // Default tolerance to decide if two float numbers are close

// Namespaces? Avoid for now

class Color {
public:
    // Properties
    float r, g, b;  // Use 32-bit format to avoid memory waste

    // Constructors
    Color() : r(0.0f), g(0.0f), b(0.0f) {}   // Default constructor (sets color to black)

    Color(float red, float green, float blue)   // Constructor with externally assigned values
        : r(red), g(green), b(blue) {}

    // Methods
    // Check if color is close to another color within some tolerance.
    // If no tolerance is provided, DEFAULT_ERROR_TOLERANCE is used.
    bool is_close_to(const Color& other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
        return (std::fabs(r - other.r) < error_tolerance &&
                std::fabs(g - other.g) < error_tolerance &&
                std::fabs(b - other.b) < error_tolerance);
    }

    // Sum of two colors
    Color operator+(const Color& other) const {
        return Color(r + other.r, g + other.g, b + other.b);
    }

    // Product of two colors
    Color operator*(const Color& other) const {
        return Color(r * other.r, g * other.g, b * other.b);
    }

    // Product: color * scalar
    Color operator*(float scalar) const {
        return Color(r * scalar, g * scalar, b * scalar);
    }

    // Friend function to allow commutative product: scalar * color
    friend Color operator*(float scalar, const Color& my_color) {
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
    int32_t width, height; // Use int32_t format to allow large dimensions (and negative values for tests)
    std::vector<Color> pixels;

    // Constructors
    // Default constructor: if dimensions are positive, initializes the pixels vector
    // with width*height copies of Color(); otherwise, creates an empty vector.
    HdrImage(int32_t w, int32_t h)
        : width(w), height(h),
          pixels((w > 0 && h > 0) ? static_cast<size_t>(w * h) : 0, Color()) {}

    // Methods
    // Some methods should be private, but following the convention are declared public with underscore (_) in front.
    
    // _valid_indexes returns true if row and col are nonnegative and within bounds.
    bool _valid_indexes(int32_t row, int32_t col) const {
        return (row >= 0 && col >= 0 && row < height && col < width);
    }

    // _pixel_offset returns the index of the pixels vector corresponding to the given matrix row and column, using row-major ordering.
    // Uses assert to ensure that the provided indices are valid.
    int32_t _pixel_offset(int32_t row, int32_t col) const {
        assert(_valid_indexes(row, col) && "Invalid indices in _pixel_offset");
        return row * width + col;
    }

    // get_pixel returns the color at the given row and column.
    Color get_pixel(int32_t row, int32_t col) const {
        int32_t offset = _pixel_offset(row, col); // Assign _pixel_offset to a variable (useful for debugging)
        return pixels[offset];
    }

    // set_pixel sets the pixel at the given row and column to the provided color.
    void set_pixel(int32_t row, int32_t col, const Color &c) {
        int32_t offset = _pixel_offset(row, col); // Assign _pixel_offset to a variable (useful for debugging)
        pixels[offset] = c;
    }
};
