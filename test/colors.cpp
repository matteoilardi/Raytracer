#include "colors.hpp"

using namespace std;

bool is_little_endian() {
  uint16_t word{0x1234};
  uint8_t *ptr{(uint8_t *)&word};

  return ptr[0] == 0x34;
}

void test_basic_color() {
  Color color1(1.0, 2.0, 3.0);

  assert(color1.is_close_to(Color(1.0, 2.0, 3.0)));
  assert(!color1.is_close_to(Color(2.0, 1.0, 3.0)));
  assert(!color1.is_close_to(Color(1.0, 2.0, 3.0001)));
}

void test_color_operations() {
  Color color1(4.0, 5.0, 6.0);
  Color color2(1.0, 12.0, 7.0);
  float f1(1.5);

  assert(are_close(color1 + color2, Color(5.0, 17.0, 13.0)));
  assert(!are_close(color1 + color2, Color(5.0, 16.0, 13.5)));

  assert(are_close(color1 * color2, Color(4.0, 60.0, 42.0)));

  assert(are_close(f1 * color1, Color(6.0, 7.5, 9.0)));
  assert(are_close(color1 * f1, Color(6.0, 7.5, 9.0)));
}

void test_basic_HdrImage() {
  HdrImage image1(20, 30);
  Color color1(21.0, 18.0, 0.0);

  assert(image1.width == 20 && image1.height == 30);

  image1.set_pixel(15, 11, color1);
  Color color2 = image1.get_pixel(15, 11);

  assert(color1.is_close_to(color2));
  assert(color2.is_close_to(Color(21.0, 18.0, 0.0)));

  assert(image1._valid_indexes(3, 4));
  assert(image1._valid_indexes(19, 29));
  assert(image1._valid_indexes(0, 0));
  assert(!image1._valid_indexes(-1, 0));
  assert(!image1._valid_indexes(0, -1));
  assert(!image1._valid_indexes(0, 30));
  assert(!image1._valid_indexes(20, 0));

  assert(image1._pixel_offset(9, 5) == (9 + 5 * image1.width));
}

void test_pfm_read_line() {
  std::stringstream ss("Hello\nworld");

  assert(_read_line(ss) == "Hello");
  assert(_read_line(ss) == "world");
  assert(_read_line(ss) == "");
}

void test_pfm_parse_endianness() {
  assert(_parse_endianness("1.0") == Endianness::big_endian);
  assert(_parse_endianness("-1.0") == Endianness::little_endian);

  bool check_exception1 = false;
  bool check_exception2 = false;

  try {
    Endianness endianness = _parse_endianness("0.0");
  } catch (InvalidPfmFileFormat &e) {
    check_exception1 = true;
  }
  try {
    Endianness endianness = _parse_endianness("abc");
  } catch (InvalidPfmFileFormat &e) {
    check_exception2 = true;
  }

  assert(check_exception1);
  assert(check_exception2);
}

void test_pfm_parse_img_size() {
  assert(_parse_img_size("3 4") == std::pair(3, 4));

  bool check_exception1 = false;
  bool check_exception2 = false;
  bool check_exception3 = false;

  try {
    std::pair<int, int> wh = _parse_img_size("3 -1");
  } catch (InvalidPfmFileFormat &e) {
    check_exception1 = true;
  }
  try {
    std::pair<int, int> wh = _parse_img_size("3 -1 9");
  } catch (InvalidPfmFileFormat &e) {
    check_exception2 = true;
  }
  try {
    std::pair<int, int> wh = _parse_img_size("3 ");
  } catch (InvalidPfmFileFormat &e) {
    check_exception3 = true;
  }

  assert(check_exception1);
  assert(check_exception2);
  assert(check_exception3);
}

void test_pfm_read_write_float() {
  std::stringstream ss;

  uint32_t n = 3294682275;
  float x = *((float *)&n);
  _write_float(ss, x, Endianness::little_endian);
  assert(_read_float(ss, Endianness::little_endian) == x);

  _write_float(ss, x, Endianness::big_endian);
  assert(_read_float(ss, Endianness::big_endian) == x);

  _write_float(ss, x, Endianness::little_endian);
  assert(!(_read_float(ss, Endianness::big_endian) == x));
}

void test_pfm_write() {
  // Fill in image as depicted in slide 15, lab lesson 3
  HdrImage image(3, 2);
  image.set_pixel(0, 0, Color(10, 20, 30));
  image.set_pixel(0, 1, Color(100, 200, 300));
  image.set_pixel(1, 0, Color(40, 50, 60));
  image.set_pixel(1, 1, Color(400, 500, 600));
  image.set_pixel(2, 0, Color(70, 80, 90));
  image.set_pixel(2, 1, Color(700, 800, 900));

  std::stringstream sstream_le;
  image.write_pfm(sstream_le, Endianness::little_endian);
  std::string string_le = sstream_le.str();

  std::stringstream sstream_be;
  image.write_pfm(sstream_be, Endianness::big_endian);
  std::string string_be = sstream_be.str();

  // Reference bytes from file "reference_le.pfm"
  unsigned char reference_le[] = {
      0x50, 0x46, 0x0a, 0x33, 0x20, 0x32, 0x0a, 0x2d, 0x31, 0x2e, 0x30, 0x0a,
      0x00, 0x00, 0xc8, 0x42, 0x00, 0x00, 0x48, 0x43, 0x00, 0x00, 0x96, 0x43,
      0x00, 0x00, 0xc8, 0x43, 0x00, 0x00, 0xfa, 0x43, 0x00, 0x00, 0x16, 0x44,
      0x00, 0x00, 0x2f, 0x44, 0x00, 0x00, 0x48, 0x44, 0x00, 0x00, 0x61, 0x44,
      0x00, 0x00, 0x20, 0x41, 0x00, 0x00, 0xa0, 0x41, 0x00, 0x00, 0xf0, 0x41,
      0x00, 0x00, 0x20, 0x42, 0x00, 0x00, 0x48, 0x42, 0x00, 0x00, 0x70, 0x42,
      0x00, 0x00, 0x8c, 0x42, 0x00, 0x00, 0xa0, 0x42, 0x00, 0x00, 0xb4, 0x42};
  unsigned int reference_le_len = 84;

  // Reference bytes from file "reference_be.pfm"
  unsigned char reference_be[] = {
      0x50, 0x46, 0x0a, 0x33, 0x20, 0x32, 0x0a, 0x31, 0x2e, 0x30, 0x0a, 0x42,
      0xc8, 0x00, 0x00, 0x43, 0x48, 0x00, 0x00, 0x43, 0x96, 0x00, 0x00, 0x43,
      0xc8, 0x00, 0x00, 0x43, 0xfa, 0x00, 0x00, 0x44, 0x16, 0x00, 0x00, 0x44,
      0x2f, 0x00, 0x00, 0x44, 0x48, 0x00, 0x00, 0x44, 0x61, 0x00, 0x00, 0x41,
      0x20, 0x00, 0x00, 0x41, 0xa0, 0x00, 0x00, 0x41, 0xf0, 0x00, 0x00, 0x42,
      0x20, 0x00, 0x00, 0x42, 0x48, 0x00, 0x00, 0x42, 0x70, 0x00, 0x00, 0x42,
      0x8c, 0x00, 0x00, 0x42, 0xa0, 0x00, 0x00, 0x42, 0xb4, 0x00, 0x00};
  unsigned int reference_be_len = 83;

  // check size
  assert(string_le.size() == reference_le_len);
  assert(string_be.size() == reference_be_len);

  // check that the bytes are the same
  assert(memcmp(string_le.data(), reference_le, reference_le_len) == 0);
  assert(memcmp(string_be.data(), reference_be, reference_be_len) == 0);
}

void test_pfm_read() {
  HdrImage image_from_le("../test/reference_le.pfm");
  HdrImage image_from_be("../test/reference_be.pfm");

  assert(are_close(image_from_le.get_pixel(0, 0), Color(10, 20, 30)));
  assert(are_close(image_from_le.get_pixel(0, 1), Color(100, 200, 300)));
  assert(are_close(image_from_le.get_pixel(1, 0), Color(40, 50, 60)));
  assert(are_close(image_from_le.get_pixel(1, 1), Color(400, 500, 600)));
  assert(are_close(image_from_le.get_pixel(2, 0), Color(70, 80, 90)));
  assert(are_close(image_from_le.get_pixel(2, 1), Color(700, 800, 900)));

  assert(are_close(image_from_be.get_pixel(0, 0), Color(10, 20, 30)));
  assert(are_close(image_from_be.get_pixel(0, 1), Color(100, 200, 300)));
  assert(are_close(image_from_be.get_pixel(1, 0), Color(40, 50, 60)));
  assert(are_close(image_from_be.get_pixel(1, 1), Color(400, 500, 600)));
  assert(are_close(image_from_be.get_pixel(2, 0), Color(70, 80, 90)));
  assert(are_close(image_from_be.get_pixel(2, 1), Color(700, 800, 900)));

  ofstream of("out.txt");
  image_from_le.write_pfm(of, Endianness::little_endian);
  of.close();

  std::stringstream sstream;
  bool check_exception1 = false;
  sstream << "PF\n4 5\n1.0\nstop";
  try {
    HdrImage image_from_corrupted(sstream);
  } catch (InvalidPfmFileFormat &e) {
    check_exception1 = true;
  }
  assert(check_exception1);
}

int main() {

  // Test 1: Creation of a color object, is_close function
  test_basic_color();

  // Test 2: Color operations
  test_color_operations();

  // Test 3: HdrImage creation, height and width, valid_indexes method,
  // _pixel_offset method
  test_basic_HdrImage();

  // Test 4
  test_pfm_read_line();

  // Test 5
  test_pfm_parse_endianness();

  // Test 6
  test_pfm_parse_img_size();

  // Test 7
  test_pfm_read_write_float();

  // Test 8
  test_pfm_write();

  // Test 9
  test_pfm_read();

  //std::cout << "Is the system little_endian? Answer: " << is_little_endian() << std::endl; YES

  return EXIT_SUCCESS;
}