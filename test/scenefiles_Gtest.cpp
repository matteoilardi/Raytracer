// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// TESTS FOR INPUT STREAM and its LEXER METHODS
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

#include "scenefiles.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <iostream>

// ------------------------------------------------------------------------------------------------------------
// ---- Helper functions for token assertions ------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

void expect_eq_keyword(const Token &token, KeywordEnum keyword) {
  EXPECT_EQ(token.type, TokenType::KEYWORD);
  EXPECT_EQ(std::get<KeywordEnum>(token.value), keyword);
}

void expect_eq_identifier(const Token &token, const std::string &identifier) {
  EXPECT_EQ(token.type, TokenType::IDENTIFIER);
  EXPECT_EQ(std::get<std::string>(token.value), identifier);
}

void expect_eq_symbol(const Token &token, char symbol) {
  EXPECT_EQ(token.type, TokenType::SYMBOL);
  EXPECT_EQ(std::get<char>(token.value), symbol);
}

void expect_eq_number(const Token &token, float number) {
  EXPECT_EQ(token.type, TokenType::LITERAL_NUMBER);
  EXPECT_FLOAT_EQ(std::get<float>(token.value), number);
}

void expect_eq_string(const Token &token, const std::string &s) {
  EXPECT_EQ(token.type, TokenType::LITERAL_STRING);
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
        new material sky_material(
            diffuse(image("my file.pfm")),
            <1.0, .33, 0.7>
        ) # Comment at the end of the line
    )");
  // create an InputStream object with the string stream
  InputStream input_file(ss);

  expect_eq_keyword(input_file.read_token(), KeywordEnum::NEW);
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
  EXPECT_EQ(eof.type, TokenType::STOP_TOKEN);
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
