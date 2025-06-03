// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// TESTS FOR INPUT STREAM and its LEXER METHODS
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

#include "scenefiles.hpp"
#include <gtest/gtest.h>
#include <sstream>

// ------------------------------------------------------------------------------------------------------------
// ---- Helper functions for token assertions ------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

void assert_is_keyword(const Token &token, KeywordEnum keyword) {
  EXPECT_EQ(token.type, TokenType::KEYWORD);
  EXPECT_EQ(std::get<KeywordEnum>(token.value), keyword);
}

void assert_is_identifier(const Token &token, const std::string &identifier) {
  EXPECT_EQ(token.type, TokenType::IDENTIFIER);
  EXPECT_EQ(std::get<std::string>(token.value), identifier);
}

void assert_is_symbol(const Token &token, char symbol) {
  EXPECT_EQ(token.type, TokenType::SYMBOL);
  EXPECT_EQ(std::get<char>(token.value), symbol);
}

void assert_is_number(const Token &token, float number) {
  EXPECT_EQ(token.type, TokenType::LITERAL_NUMBER);
  EXPECT_FLOAT_EQ(std::get<float>(token.value), number);
}

void assert_is_string(const Token &token, const std::string &s) {
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

  // change on purpose from 'a' to 'A': you can unread any char X, not necessarily the one you just read
  stream.unread_char('A');
  EXPECT_EQ(stream.location.line, 1);
  EXPECT_EQ(stream.location.column, 1);

  // Now read again, since you just unread 'A', you should get it back
  EXPECT_EQ(stream.read_char(), 'A');
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
  stream.skip_whitespaces_and_comments();
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
            <5.0, 500.0, 300.0>
        ) # Comment at the end of the line
    )");
  // create an InputStream object with the string stream
  InputStream input_file(ss);

  assert_is_keyword(input_file.read_token(), KeywordEnum::NEW);
  assert_is_keyword(input_file.read_token(), KeywordEnum::MATERIAL);
  assert_is_identifier(input_file.read_token(), "sky_material");
  assert_is_symbol(input_file.read_token(), '(');
  assert_is_keyword(input_file.read_token(), KeywordEnum::DIFFUSE);
  assert_is_symbol(input_file.read_token(), '(');
  assert_is_keyword(input_file.read_token(), KeywordEnum::IMAGE);
  assert_is_symbol(input_file.read_token(), '(');
  assert_is_string(input_file.read_token(), "my file.pfm");
  assert_is_symbol(input_file.read_token(), ')');
  assert_is_symbol(input_file.read_token(), ')');
  assert_is_symbol(input_file.read_token(), ',');

  // Check <5.0, 500.0, 300.0>
  assert_is_symbol(input_file.read_token(), '<');
  assert_is_number(input_file.read_token(), 5.0f);
  assert_is_symbol(input_file.read_token(), ',');
  assert_is_number(input_file.read_token(), 500.0f);
  assert_is_symbol(input_file.read_token(), ',');
  assert_is_number(input_file.read_token(), 300.0f);
  assert_is_symbol(input_file.read_token(), '>');
  assert_is_symbol(input_file.read_token(), ')');

  // Should be at end of file since comments and whitespaces are skipped
  Token eof = input_file.read_token();
  EXPECT_EQ(eof.type, TokenType::STOP_TOKEN);
}
