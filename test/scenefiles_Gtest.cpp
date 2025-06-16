// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// TESTS FOR SCENE_FILES (Token, InputStream, Scene, GrammarErrro, Lexer and Parser)
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

#include "scenefiles.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

// ------------------------------------------------------------------------------------------------------------
// ---- Helper functions for token assertions ------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

void expect_eq_keyword(const Token &token, KeywordEnum keyword) {
  EXPECT_EQ(token.type, TokenKind::KEYWORD);
  EXPECT_EQ(std::get<KeywordEnum>(token.value), keyword);
}

void expect_eq_identifier(const Token &token, const std::string &identifier) {
  EXPECT_EQ(token.type, TokenKind::IDENTIFIER);
  EXPECT_EQ(std::get<std::string>(token.value), identifier);
}

void expect_eq_symbol(const Token &token, char symbol) {
  EXPECT_EQ(token.type, TokenKind::SYMBOL);
  EXPECT_EQ(std::get<char>(token.value), symbol);
}

void expect_eq_number(const Token &token, float number) {
  EXPECT_EQ(token.type, TokenKind::LITERAL_NUMBER);
  EXPECT_FLOAT_EQ(std::get<float>(token.value), number);
}

void expect_eq_string(const Token &token, const std::string &s) {
  EXPECT_EQ(token.type, TokenKind::LITERAL_STRING);
  EXPECT_EQ(std::get<std::string>(token.value), s);
}

// ------------------------------------------------------------------------------------------------------------
// ---- Actual tests ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

//---------------- Test character reading functionalities (read and unread char, update location) in InputStream ----------------

TEST(InputStreamTest, test_input_file) {
  std::istringstream ss("abc   \nd\nef"); // Create a string stream to simulate an input file
  InputStream stream(ss);                 // Create an InputStream object with the string stream

  // initial position (we start at line 1, column 1)
  EXPECT_EQ(stream.location.line, 1);
  EXPECT_EQ(stream.location.column, 1);

  // read first character
  EXPECT_EQ(stream.read_char(), 'a');
  EXPECT_EQ(stream.location.line, 1);
  EXPECT_EQ(stream.location.column, 2);

  // change on purpose from 'a' to 'X': you can unread any char X, not necessarily the one you just read
  stream.unread_char('X');
  EXPECT_EQ(stream.location.line, 1);
  EXPECT_EQ(stream.location.column, 1);

  // Now read again, since you just unread 'X', you should get it back
  EXPECT_EQ(stream.read_char(), 'X');
  EXPECT_EQ(stream.location.line, 1);
  EXPECT_EQ(stream.location.column, 2);

  // Now make sure the reading continues from the next character, as if nothing happened
  EXPECT_EQ(stream.read_char(), 'b');
  EXPECT_EQ(stream.location.line, 1);
  EXPECT_EQ(stream.location.column, 3);

  // keep reading characters
  EXPECT_EQ(stream.read_char(), 'c');
  EXPECT_EQ(stream.location.line, 1);
  EXPECT_EQ(stream.location.column, 4);

  // skip whitespaces and comments and check if reading continues correctly
  stream._skip_whitespaces_and_comments();
  EXPECT_EQ(stream.read_char(), 'd');
  EXPECT_EQ(stream.location.line, 2);
  EXPECT_EQ(stream.location.column, 2);

  // check if you read break line correctly and if the location is updated
  EXPECT_EQ(stream.read_char(), '\n');
  EXPECT_EQ(stream.location.line, 3);
  EXPECT_EQ(stream.location.column, 1);

  // keep reading characters
  EXPECT_EQ(stream.read_char(), 'e');
  EXPECT_EQ(stream.location.line, 3);
  EXPECT_EQ(stream.location.column, 2);

  // keep reading characters
  EXPECT_EQ(stream.read_char(), 'f');
  EXPECT_EQ(stream.location.line, 3);
  EXPECT_EQ(stream.location.column, 3);

  // check if you read end of stream correctly
  EXPECT_EQ(stream.read_char(), 0); // 0 signals end of stream in InputStream
}

// -------------- Test lexer functionality (token recognition) in InputStream ----------------

TEST(InputStreamTest, test_lexer) {
  // create a string stream to simulate an input file (R "..." syntax is to allow breaking line in the string)
  std::istringstream ss(R"(
        # This is a comment
        # This is another comment
        material sky_material(
            diffuse(image("my file.pfm")),
            <1.0, .33, 0.7>
        ) # Comment at the end of the line
    )");
  // create an InputStream object with the string stream
  InputStream input_file(ss);

  expect_eq_keyword(input_file.read_token(), KeywordEnum::MATERIAL);
  expect_eq_identifier(input_file.read_token(), "sky_material");
  expect_eq_symbol(input_file.read_token(), '(');
  expect_eq_keyword(input_file.read_token(), KeywordEnum::DIFFUSE);
  expect_eq_symbol(input_file.read_token(), '(');
  expect_eq_keyword(input_file.read_token(), KeywordEnum::IMAGE);
  expect_eq_symbol(input_file.read_token(), '(');
  expect_eq_string(input_file.read_token(), "my file.pfm");
  expect_eq_symbol(input_file.read_token(), ')');
  expect_eq_symbol(input_file.read_token(), ')');
  expect_eq_symbol(input_file.read_token(), ',');

  // Check <5.0, 500.0, 300.0>
  expect_eq_symbol(input_file.read_token(), '<');
  expect_eq_number(input_file.read_token(), 1.f);
  expect_eq_symbol(input_file.read_token(), ',');
  expect_eq_number(input_file.read_token(), 0.33f);
  expect_eq_symbol(input_file.read_token(), ',');
  expect_eq_number(input_file.read_token(), 0.7f);
  expect_eq_symbol(input_file.read_token(), '>');
  expect_eq_symbol(input_file.read_token(), ')');

  // Should be at end of file since comments and whitespaces are skipped
  Token eof = input_file.read_token();
  EXPECT_EQ(eof.type, TokenKind::STOP_TOKEN);
}

// ---- Test GrammarError functionality of lexer --------------------------------------------------------

TEST(InputStreamTest, test_GrammarError) {
  // 1. Test invalid float number
  {
    // Set up a stream with an invalid float: "12.3.4" is not a valid float (two dots not allowed)
    std::istringstream ss("12.3.4");
    InputStream input_file(ss);

    try {
      input_file.read_token(); // This should try to parse and throw GrammarError
      // If no exception is thrown, FAIL() will execute and the test will fail here
      FAIL() << "A GrammarError for invalid float was expected, but none was thrown";
    } catch (const GrammarError &err) {
      // If a GrammarError is thrown, control jumps here and the following assertions run

      // Check that the error message says it's an invalid floating-point number
      EXPECT_NE(std::string(err.what()).find("invalid floating-point number"), std::string::npos);

      // Check that the error is reported on line 1, column 1
      // (in IputStream code is arranged so that error are reported from the start of the token)
      EXPECT_EQ(err.location.line, 1);
      EXPECT_EQ(err.location.column, 1);

    } catch (...) {
      // If an unexpected exception type is thrown, mark the test as failed
      FAIL() << "Expected GrammarError, got different exception";
    }
  }

  // 2. Test invalid character (e.g. @)
  {
    // Set up a stream with an invalid character: "@" is not valid in your syntax
    std::istringstream ss("@");
    InputStream input_file(ss);

    try {
      input_file.read_token(); // Should throw GrammarError for invalid character
      // If no exception is thrown, FAIL() will execute and the test will fail here
      FAIL() << "A GrammarError for invalid character was expected, but none was thrown";
    } catch (const GrammarError &err) {
      // If a GrammarError is thrown, control jumps here and the following assertions run

      // Check that the error message says 'Invalid character'
      EXPECT_NE(std::string(err.what()).find("invalid character"), std::string::npos);

      // Check the error is on line 1, column 1 (where the "@" appears)
      // (in IputStream code is arranged so that error are reported from the start of the token)
      EXPECT_EQ(err.location.line, 1);
      EXPECT_EQ(err.location.column, 1);
    } catch (...) {
      // If a different exception is thrown, mark the test as failed
      FAIL() << "Expected GrammarError, got different exception";
    }
  }
}

// -------------- Test parser (implemented in Scene) ----------------
// TODO angles: degrees or rads???? //QUESTION Are you asking if we want to switch to radians?
// TODO perhaps change name of parse_scene?

// create a string stream with somewhat messy input file (R "..." syntax is to allow breaking line in the string)
TEST(SceneTest, test_parse_scene) {
  std::istringstream ss(R"(
		float clock(150)

        material sky_material(
            diffuse(uniform(<0, 0, 0>)),
            uniform(<0.7, 0.5, 1>)
        )

        # Here is a comment

        material ground_material(
            diffuse(checkered(<0.3, 0.5, 0.1>,
                              <0.1, 0.2, 0.5>, 4)),
            uniform(<0, 0, 0>)
        )

        material sphere_material(
            specular(uniform(<0.5, 0.5, 0.5>)),
            uniform(<0, 0, 0>)
        )

        plane (translation([0, 0, 100]) * rotation_y(clock), sky_material)
        plane(identity, ground_material)

        sphere(translation([0, 0, 1]), sphere_material)

        camera(perspective, rotation_z(30) * translation([-4, 0, 1]), 1.0, 2.0)
        )");

  InputStream input_stream(ss);
  Scene scene;
  scene.parse_scene(input_stream);

  // Check defined float variables
  EXPECT_EQ(scene.float_variables.size(), 1);
  EXPECT_EQ(scene.float_variables.count("clock"), 1);
  EXPECT_EQ(scene.float_variables["clock"], 150.f);

  // Check defined Materials
  EXPECT_EQ(scene.materials.size(), 3);
  EXPECT_EQ(scene.materials.count("sphere_material"), 1);
  EXPECT_EQ(scene.materials.count("sky_material"), 1);
  EXPECT_EQ(scene.materials.count("ground_material"), 1);

  // retrieve the pointers to the materials and the BRDFs
  auto sphere_material = scene.materials["sphere_material"];
  auto sky_material = scene.materials["sky_material"];
  auto ground_material = scene.materials["ground_material"];

  auto sky_brdf = dynamic_pointer_cast<DiffusiveBRDF>(sky_material->brdf);
  auto sky_brdf_pigment = dynamic_pointer_cast<UniformPigment>(sky_material->brdf->pigment);
  // dynamic_pointer_cast converts a smart pointer to the base class into a smart pointer to a specific derived class
  // it fails (returns nullptr) if the object pointed at is NOT an instance of the derived class

  EXPECT_TRUE(sky_brdf);
  EXPECT_TRUE(sky_brdf_pigment);
  EXPECT_TRUE(sky_brdf_pigment->color.is_close(Color())); // color is a member of UniformPigment only, not of the base class so
                                                          // dynamic_pointer_cast is required for this line to compile

  auto ground_brdf = dynamic_pointer_cast<DiffusiveBRDF>(ground_material->brdf);
  auto ground_brdf_pigment = dynamic_pointer_cast<CheckeredPigment>(ground_material->brdf->pigment);
  EXPECT_TRUE(ground_brdf);
  EXPECT_TRUE(ground_brdf_pigment);
  EXPECT_TRUE(ground_brdf_pigment->color1.is_close(Color(0.3f, 0.5f, 0.1f)));
  EXPECT_TRUE(ground_brdf_pigment->color2.is_close(Color(0.1f, 0.2f, 0.5f)));
  EXPECT_EQ(ground_brdf_pigment->n_intervals, 4);

  auto sphere_brdf = dynamic_pointer_cast<SpecularBRDF>(sphere_material->brdf);
  auto sphere_brdf_pigment = dynamic_pointer_cast<UniformPigment>(sphere_material->brdf->pigment);
  EXPECT_TRUE(sphere_brdf);
  EXPECT_TRUE(sphere_brdf_pigment);
  EXPECT_TRUE(sphere_brdf_pigment->color.is_close(Color(0.5f, 0.5f, 0.5f)));

  auto sky_emitted_radiance = dynamic_pointer_cast<UniformPigment>(sky_material->emitted_radiance);
  auto ground_emitted_radiance = dynamic_pointer_cast<UniformPigment>(ground_material->emitted_radiance);
  auto sphere_emitted_radiance = dynamic_pointer_cast<UniformPigment>(sphere_material->emitted_radiance);
  EXPECT_TRUE(sky_emitted_radiance);
  EXPECT_TRUE(sky_emitted_radiance->color.is_close(Color(0.7f, 0.5f, 1.f)));
  EXPECT_TRUE(ground_emitted_radiance);
  EXPECT_TRUE(ground_emitted_radiance->color.is_close(Color(0.f, 0.f, 0.f)));
  EXPECT_TRUE(sphere_emitted_radiance);
  EXPECT_TRUE(sphere_emitted_radiance->color.is_close(Color(0.f, 0.f, 0.f)));

  // Check defined Shapes
  EXPECT_EQ(scene.world->objects.size(), 3);
  EXPECT_TRUE(dynamic_pointer_cast<Plane>(scene.world->objects[0]));
  EXPECT_TRUE(
      scene.world->objects[0]->transformation.is_close(translation(Vec(0.f, 0.f, 100.f)) * rotation_y(degs_to_rads(150.f))));
  EXPECT_TRUE(dynamic_pointer_cast<Plane>(scene.world->objects[1]));
  EXPECT_TRUE(scene.world->objects[1]->transformation.is_close(Transformation()));
  EXPECT_TRUE(dynamic_pointer_cast<Sphere>(scene.world->objects[2]));
  EXPECT_TRUE(scene.world->objects[2]->transformation.is_close(translation(Vec(0.f, 0.f, 1.f))));

  // Check defined Camera
  auto cam = dynamic_pointer_cast<PerspectiveCamera>(scene.camera);
  EXPECT_TRUE(cam);
  EXPECT_TRUE(cam->transformation.is_close(rotation_z(degs_to_rads(30.f)) * translation(Vec(-4.f, 0.f, 1.f))));
  EXPECT_TRUE(are_close(*cam->asp_ratio, 1.f));
  EXPECT_TRUE(are_close(cam->distance, 2.f));
}

TEST(SceneTest, test_parse_scene_undefined_material) {
  std::istringstream ss("plane(identity, this_material_does_not_exist)");
  InputStream input_stream(ss);
  Scene scene;

  try {
    scene.parse_scene(input_stream); // Should throw GrammarError for unknown material
    // If no exception is thrown, FAIL() will execute and the test will fail here
    FAIL() << "A GrammarError for unknown material was expected, but none was thrown";
  } catch (const GrammarError &err) {
    // Check that the error message says "unknown material"
    EXPECT_NE(std::string(err.what()).find("unknown material"), std::string::npos);

    // Check the error is on line 1, column 17 (where the unknown material appears)
    EXPECT_EQ(err.location.line, 1);
    EXPECT_EQ(err.location.column, 17);
  } catch (...) {
    // If a different exception is thrown, mark the test as failed
    FAIL() << "Expected GrammarError, got different exception";
  }
}

TEST(SceneTest, test_parse_scene_double_camera) {
  std::istringstream ss(
      "camera(perspective, rotation_z(30) * translation([-4, 0, 1]), 1.0, 1.0)\ncamera(orthogonal, identity, 1.0, 1.0)");

  InputStream input_stream(ss);
  Scene scene;

  try {
    scene.parse_scene(input_stream); // Should throw GrammarError for multiple camera definition
    // If no exception is thrown, FAIL() will execute and the test will fail here
    FAIL() << "A GrammarError for multiple camera definition was expected, but none was thrown";
  } catch (const GrammarError &err) {
    // Check that the error message says "camera already defined"
    EXPECT_NE(std::string(err.what()).find("camera already defined"), std::string::npos);

    // Check the error is on line 2, column 1 (where the second camera definition appears)
    EXPECT_EQ(err.location.line, 2);
    EXPECT_EQ(err.location.column, 1);
  } catch (...) {
    // If a different exception is thrown, mark the test as failed
    FAIL() << "Expected GrammarError, got different exception";
  }
}
