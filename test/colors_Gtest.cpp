//-----------------------------------------------------------
//--------------------------- LIBRARIES ---------------------------
// -----------------------------------------------------------

#include "colors.hpp"
#include <cstring>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

using namespace std;

bool is_little_endian() {
  uint16_t word{0x1234};
  uint8_t *ptr{(uint8_t *)&word};

  return ptr[0] == 0x34;
}

//-------------------------------------------------------------------------------------------------------------
//----------------- TESTS FOR COLOR -----------------
//-------------------------------------------------------------------------------------------------------------

// test constructor, is_close_to
TEST(ColorTest, test_is_close) {
  Color color1(1.f, 2.f, 3.f);
  EXPECT_TRUE(color1.is_close_to(Color(1.f, 2.f, 3.f)));
  EXPECT_FALSE(color1.is_close_to(Color(2.f, 1.f, 3.f)));
  EXPECT_FALSE(color1.is_close_to(Color(1.f, 2.f, 3.0001f)));
}

// test +, *, scalar multiplication
TEST(ColorTest, test_color_operations) {
  Color color1(4.f, 5.f, 6.f);
  Color color2(1.f, 12.f, 7.f);
  float f1(1.5f);

  EXPECT_TRUE(are_close(color1 + color2, Color(5.f, 17.f, 13.f)));
  EXPECT_FALSE(are_close(color1 + color2, Color(5.f, 16.f, 13.5f)));

  EXPECT_TRUE(are_close(color1 * color2, Color(4.f, 60.f, 42.f)));

  EXPECT_TRUE(are_close(f1 * color1, Color(6.f, 7.5f, 9.f)));
  EXPECT_TRUE(are_close(color1 * f1, Color(6.f, 7.5f, 9.f)));
}

// test luminosity
TEST(ColorTest, test_luminosity) {
  Color col1 = Color(1.f, 2.f, 3.f);
  Color col2 = Color(9.f, 5.f, 7.f);

  EXPECT_EQ(col1.luminosity(), 2.f);
  EXPECT_EQ(col2.luminosity(), 7.f);
}

//-------------------------------------------------------------------------------------------------------------
//----------------- TESTS FOR HDRIMAGE -----------------
//-------------------------------------------------------------------------------------------------------------

// test basic constructor, get_pixel, set_pixel, _valid_indexes, _pixel_offset
TEST(HdrImageTest, test_basic_HdrImage) {
  HdrImage image1(20, 30);

  EXPECT_TRUE(image1.width == 20 && image1.height == 30);

  EXPECT_TRUE(image1._valid_indexes(3, 4));
  EXPECT_TRUE(image1._valid_indexes(19, 29));
  EXPECT_TRUE(image1._valid_indexes(0, 0));
  EXPECT_FALSE(image1._valid_indexes(-1, 0));
  EXPECT_FALSE(image1._valid_indexes(0, -1));
  EXPECT_FALSE(image1._valid_indexes(0, 30));
  EXPECT_FALSE(image1._valid_indexes(20, 0));

  EXPECT_TRUE(image1._pixel_offset(9, 5) == (9 + 5 * image1.width));

  Color color1(21.f, 18.f, 0.f);
  image1.set_pixel(15, 11, color1);
  Color color2 = image1.get_pixel(15, 11);

  EXPECT_TRUE(are_close(color1, color2));
}

// test helper function _read_line
TEST(PfmTest, test_pfm_read_line) {
  std::stringstream ss("Hello\nworld");

  EXPECT_EQ(_read_line(ss), "Hello");
  EXPECT_EQ(_read_line(ss), "world");
  EXPECT_EQ(_read_line(ss), "");
}

// test helper function _parse_endianness
TEST(PfmTest, test_pfm_parse_endianness) {
  EXPECT_EQ(_parse_endianness("1.0"), Endianness::big_endian);
  EXPECT_EQ(_parse_endianness("-1.0"), Endianness::little_endian);

  auto assign_endianness = [](const std::string &s) -> void { Endianness endianness = _parse_endianness(s); };

  EXPECT_THROW(assign_endianness("0.0"), InvalidPfmFileFormat);
  EXPECT_THROW(assign_endianness("abc"), InvalidPfmFileFormat);
}

// test helper function _parse_img_size
TEST(PfmTest, test_pfm_parse_img_size) {
  EXPECT_EQ(_parse_img_size("3 4"), std::pair(3, 4));

  auto assign_img_size = [](const std::string &s) -> void { std::pair<int, int> wh = _parse_img_size(s); };

  EXPECT_THROW(assign_img_size("3 -1"), InvalidPfmFileFormat);
  EXPECT_THROW(assign_img_size("3 -1 9"), InvalidPfmFileFormat);
  EXPECT_THROW(assign_img_size("3 "), InvalidPfmFileFormat);
}

// test helper functions _write_float, _read_float
TEST(PfmTest, test_pfm_read_write_float) {
  std::stringstream ss;

  uint32_t n = 3294682275;
  float x = *((float *)&n);

  _write_float(ss, x, Endianness::little_endian);
  EXPECT_EQ(_read_float(ss, Endianness::little_endian), x);

  _write_float(ss, x, Endianness::big_endian);
  EXPECT_EQ(_read_float(ss, Endianness::big_endian), x);

  _write_float(ss, x, Endianness::little_endian);
  EXPECT_NE(_read_float(ss, Endianness::big_endian), x);
}

// test write_pfm method
TEST(PfmTest, test_pfm_write) {
  // Fill in image as depicted in slide 15, lab lesson 3
  HdrImage image(3, 2);
  image.set_pixel(0, 0, Color(10.f, 20.f, 30.f));
  image.set_pixel(0, 1, Color(100.f, 200.f, 300.f));
  image.set_pixel(1, 0, Color(40.f, 50.f, 60.f));
  image.set_pixel(1, 1, Color(400.f, 500.f, 600.f));
  image.set_pixel(2, 0, Color(70.f, 80.f, 90.f));
  image.set_pixel(2, 1, Color(700.f, 800.f, 900.f));

  std::stringstream sstream_le;
  image.write_pfm(sstream_le, Endianness::little_endian);
  std::string string_le = sstream_le.str();

  std::stringstream sstream_be;
  image.write_pfm(sstream_be, Endianness::big_endian);
  std::string string_be = sstream_be.str();

  // Reference bytes from file "reference_le.pfm"
  unsigned char reference_le[] = {0x50, 0x46, 0x0a, 0x33, 0x20, 0x32, 0x0a, 0x2d, 0x31, 0x2e, 0x30, 0x0a, 0x00, 0x00,
                                  0xc8, 0x42, 0x00, 0x00, 0x48, 0x43, 0x00, 0x00, 0x96, 0x43, 0x00, 0x00, 0xc8, 0x43,
                                  0x00, 0x00, 0xfa, 0x43, 0x00, 0x00, 0x16, 0x44, 0x00, 0x00, 0x2f, 0x44, 0x00, 0x00,
                                  0x48, 0x44, 0x00, 0x00, 0x61, 0x44, 0x00, 0x00, 0x20, 0x41, 0x00, 0x00, 0xa0, 0x41,
                                  0x00, 0x00, 0xf0, 0x41, 0x00, 0x00, 0x20, 0x42, 0x00, 0x00, 0x48, 0x42, 0x00, 0x00,
                                  0x70, 0x42, 0x00, 0x00, 0x8c, 0x42, 0x00, 0x00, 0xa0, 0x42, 0x00, 0x00, 0xb4, 0x42};
  unsigned int reference_le_len = 84;

  // Reference bytes from file "reference_be.pfm"
  unsigned char reference_be[] = {0x50, 0x46, 0x0a, 0x33, 0x20, 0x32, 0x0a, 0x31, 0x2e, 0x30, 0x0a, 0x42, 0xc8, 0x00,
                                  0x00, 0x43, 0x48, 0x00, 0x00, 0x43, 0x96, 0x00, 0x00, 0x43, 0xc8, 0x00, 0x00, 0x43,
                                  0xfa, 0x00, 0x00, 0x44, 0x16, 0x00, 0x00, 0x44, 0x2f, 0x00, 0x00, 0x44, 0x48, 0x00,
                                  0x00, 0x44, 0x61, 0x00, 0x00, 0x41, 0x20, 0x00, 0x00, 0x41, 0xa0, 0x00, 0x00, 0x41,
                                  0xf0, 0x00, 0x00, 0x42, 0x20, 0x00, 0x00, 0x42, 0x48, 0x00, 0x00, 0x42, 0x70, 0x00,
                                  0x00, 0x42, 0x8c, 0x00, 0x00, 0x42, 0xa0, 0x00, 0x00, 0x42, 0xb4, 0x00, 0x00};
  unsigned int reference_be_len = 83;

  // check size
  EXPECT_EQ(string_le.size(), reference_le_len);
  EXPECT_EQ(string_be.size(), reference_be_len);

  // check that the bytes are the same
  EXPECT_EQ(memcmp(string_le.data(), reference_le, reference_le_len), 0);
  EXPECT_EQ(memcmp(string_be.data(), reference_be, reference_be_len), 0);
}

// test read_pfm constructor
TEST(PfmTest, test_pfm_read) {
  HdrImage image_from_le("../samples/reference_le.pfm");
  HdrImage image_from_be("../samples/reference_be.pfm");

  EXPECT_TRUE(are_close(image_from_le.get_pixel(0, 0), Color(10.f, 20.f, 30.f)));
  EXPECT_TRUE(are_close(image_from_le.get_pixel(0, 1), Color(100.f, 200.f, 300.f)));
  EXPECT_TRUE(are_close(image_from_le.get_pixel(1, 0), Color(40.f, 50.f, 60.f)));
  EXPECT_TRUE(are_close(image_from_le.get_pixel(1, 1), Color(400.f, 500.f, 600.f)));
  EXPECT_TRUE(are_close(image_from_le.get_pixel(2, 0), Color(70.f, 80.f, 90.f)));
  EXPECT_TRUE(are_close(image_from_le.get_pixel(2, 1), Color(700.f, 800.f, 900.f)));

  EXPECT_TRUE(are_close(image_from_be.get_pixel(0, 0), Color(10.f, 20.f, 30.f)));
  EXPECT_TRUE(are_close(image_from_be.get_pixel(0, 1), Color(100.f, 200.f, 300.f)));
  EXPECT_TRUE(are_close(image_from_be.get_pixel(1, 0), Color(40.f, 50.f, 60.f)));
  EXPECT_TRUE(are_close(image_from_be.get_pixel(1, 1), Color(400.f, 500.f, 600.f)));
  EXPECT_TRUE(are_close(image_from_be.get_pixel(2, 0), Color(70.f, 80.f, 90.f)));
  EXPECT_TRUE(are_close(image_from_be.get_pixel(2, 1), Color(700.f, 800.f, 900.f)));

  // We had issues reading the floats from the PFM file.
  // The reason was that the hexadecimal representation of the float number 10.0 contains the byte 0x20
  // which is the space character. However, stream reading ignores spaces and this caused the output to be misaligned.
  // Indeed we started having issues from in the first line (starting from bottom, i.e. second line) where 10.0 first
  // appears.
}

// test read_pfm constructor on an invalid input stream
TEST(PfmTest, test_pfm_read_wrong) {
  std::stringstream sstream;
  sstream << "PF\n4 5\n1.0\nstop";

  EXPECT_THROW([&]() -> void { HdrImage image_from_corrupted(sstream); }(), InvalidPfmFileFormat);
}

// test average_luminosity
TEST(HdrImageTest, test_average_luminosity) {
  HdrImage img(2, 1);
  img.set_pixel(0, 0, Color(5.f, 10.f, 15.f));
  img.set_pixel(1, 0, Color(500.f, 1000.f, 1500.f));

  EXPECT_EQ(img.average_luminosity(0.0), 100.f);

  HdrImage img_with_black(1, 1);
  img_with_black.set_pixel(0, 0, Color());

  EXPECT_TRUE(are_close(img_with_black.average_luminosity(), DEFAULT_DELTA_LOG));
}

// test normalize_image
TEST(HdrImageTest, test_normalize_image) {
  HdrImage img(2, 1);
  img.set_pixel(0, 0, Color(5.f, 10.f, 15.f));
  img.set_pixel(1, 0, Color(500.f, 1000.f, 1500.f));

  img.normalize_image(10.f);

  EXPECT_TRUE(img.get_pixel(0, 0).is_close_to(Color(5.0e-1f, 10.0e-1f, 15.0e-1f)));
  EXPECT_TRUE(img.get_pixel(1, 0).is_close_to(Color(5.0e1f, 10.0e1f, 15.0e1f)));
}

// test clamp_image
TEST(HdrImageTest, test_clamp_image) {
  HdrImage img(2, 1);
  img.set_pixel(0, 0, Color(2e3f, 4e5f, 6e1f));
  img.set_pixel(1, 0, Color(1e2f, 3e4f, 5e7f));

  img.clamp_image();

  for (auto pixel : img.pixels) {
    EXPECT_TRUE(pixel.r >= 0.f && pixel.r <= 1.f);
    EXPECT_TRUE(pixel.g >= 0.f && pixel.g <= 1.f);
    EXPECT_TRUE(pixel.b >= 0.f && pixel.b <= 1.f);
  }
}
