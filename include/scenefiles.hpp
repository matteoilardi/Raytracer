// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// ------------ LIBRARY FOR COMPILERS -----------------
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#include <cassert>
#include <cctype> // constains character classification functions (isalpha, isdigit, etc.)
#include <iostream>
#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant> // type-safe tagged union replacement (used for TokenValue)

#include "cameras.hpp"
#include "colors.hpp"
#include "geometry.hpp"
#include "materials.hpp"
#include "random.hpp"
#include "renderers.hpp"
#include "shapes.hpp"

// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ----------------------------------------------------------------------------------------

const std::string SYMBOLS = "()[]<>,*"; // Declare our symbols
enum class KeywordEnum;                 // Forward declare our keywords (see section below)

//----------------------------------------------------------------------------------------------------
//------------------- KEYWORDS and related HELPERS -------------------------
//----------------------------------------------------------------------------------------------------

/// @brief Enum class for the keywords used in the scene files
enum class KeywordEnum {
  MATERIAL,
  PLANE,
  SPHERE,
  DIFFUSE,
  SPECULAR,
  UNIFORM,
  CHECKERED,
  IMAGE,
  IDENTITY,
  TRANSLATION,
  ROTATION_X,
  ROTATION_Y,
  ROTATION_Z,
  SCALING,
  CAMERA,
  ORTHOGONAL,
  PERSPECTIVE,
  EXACT_ASP_RATIO,
  FLOAT,
  POINT_LIGHT
};

/// @brief String to keyword map
const std::unordered_map<std::string, KeywordEnum> KEYWORDS = {{"material", KeywordEnum::MATERIAL},
                                                               {"plane", KeywordEnum::PLANE},
                                                               {"sphere", KeywordEnum::SPHERE},
                                                               {"diffuse", KeywordEnum::DIFFUSE},
                                                               {"specular", KeywordEnum::SPECULAR},
                                                               {"uniform", KeywordEnum::UNIFORM},
                                                               {"checkered", KeywordEnum::CHECKERED},
                                                               {"image", KeywordEnum::IMAGE},
                                                               {"identity", KeywordEnum::IDENTITY},
                                                               {"translation", KeywordEnum::TRANSLATION},
                                                               {"rotation_x", KeywordEnum::ROTATION_X},
                                                               {"rotation_y", KeywordEnum::ROTATION_Y},
                                                               {"rotation_z", KeywordEnum::ROTATION_Z},
                                                               {"scaling", KeywordEnum::SCALING},
                                                               {"camera", KeywordEnum::CAMERA},
                                                               {"orthogonal", KeywordEnum::ORTHOGONAL},
                                                               {"perspective", KeywordEnum::PERSPECTIVE},
                                                               {"exact_asp_ratio", KeywordEnum::EXACT_ASP_RATIO},
                                                               {"float", KeywordEnum::FLOAT},
                                                               {"point_light", KeywordEnum::POINT_LIGHT}};

/// @brief Convert a keyword enum to its string representation
std::string to_string(KeywordEnum kw) {
  switch (kw) {
  case KeywordEnum::MATERIAL:
    return "material";
  case KeywordEnum::PLANE:
    return "plane";
  case KeywordEnum::SPHERE:
    return "sphere";
  case KeywordEnum::DIFFUSE:
    return "diffuse";
  case KeywordEnum::SPECULAR:
    return "specular";
  case KeywordEnum::UNIFORM:
    return "uniform";
  case KeywordEnum::CHECKERED:
    return "checkered";
  case KeywordEnum::IMAGE:
    return "image";
  case KeywordEnum::IDENTITY:
    return "identity";
  case KeywordEnum::TRANSLATION:
    return "translation";
  case KeywordEnum::ROTATION_X:
    return "rotation_x";
  case KeywordEnum::ROTATION_Y:
    return "rotation_y";
  case KeywordEnum::ROTATION_Z:
    return "rotation_z";
  case KeywordEnum::SCALING:
    return "scaling";
  case KeywordEnum::CAMERA:
    return "camera";
  case KeywordEnum::ORTHOGONAL:
    return "orthogonal";
  case KeywordEnum::PERSPECTIVE:
    return "perspective";
  case KeywordEnum::EXACT_ASP_RATIO:
    return "exact_asp_ratio";
  case KeywordEnum::FLOAT:
    return "float";
  case KeywordEnum::POINT_LIGHT:
    return "point_light";
  default:
    throw std::logic_error("Unreachable code: unknown KEYWORD");
  }
}

//----------------------------------------------------------------------------------------------------
//------------------- TOKEN CLASS (using std::variant instead of union) and AUXILIARY SUBCLASSES -------------------------
//----------------------------------------------------------------------------------------------------

//------------SOURCE LOCATION: contains file name, line number and column number of the token----

struct SourceLocation {
  //------- Properties --------
  std::string file; // file name (or empty string if not applicable e.g. source code was provided as a memory stream)
  int line;         // line number (starting from 1)
  int column;       // column number (starting from 1)

  //----------- Constructors -----------
  /// @brief Default constructor initializing to empty file name, line one and column one
  SourceLocation(const std::string &file = "", int line = 1, int column = 1) : file(file), line(line), column(column) {}

  //----------- Methods -----------
  /// @brief Convert source location to string (for debugging)
  std::string to_string() const {
    return "File: " + file + ", Line: " + std::to_string(line) + ", Column: " + std::to_string(column);
  }
};

//------------ TOKEN TYPE (used for dispatching at runtime)---------------

enum class TokenKind {
  // these are the 6 types of token implemented in Pytracer
  STOP_TOKEN,     // token signalling the end of a file
  KEYWORD,        // token containing a keyword
  SYMBOL,         // token containing a symbol
  IDENTIFIER,     // token containing an identifier (i.e. a variable name)
  LITERAL_STRING, // token containing a literal string
  LITERAL_NUMBER  // token containing a literal number (i.e. a float)
};

// ------------ TOKEN VALUE: variant for possible token values ---------------
using TokenValue = std::variant<std::monostate, KeywordEnum, char, std::string, float>;

//------ TOKEN CLASS: product of `SourceLocation', `TokenKind` and `TokenValue` (tagged union pattern in C++) ------
class Token {
public:
  //------- Properties --------
  SourceLocation source_location; // source location
  TokenKind type;                 // tag
  TokenValue value;               // type-safe tagged union

  //----------- Constructors -----------
  Token(const SourceLocation &loc, TokenKind type) : source_location(loc), type(type), value(std::monostate{}) {};

  //----------- Methods -----------

  /// @brief Assign a stop token (signals EndOfFile)
  void assign_stop_token() {
    type = TokenKind::STOP_TOKEN;
    value = std::monostate{}; // No value needed for STOP_TOKEN
  }

  /// @brief Assign a keyword token
  void assign_keyword(KeywordEnum kw) {
    type = TokenKind::KEYWORD;
    value = kw;
  }

  /// @brief Assign a single character symbol token
  void assign_symbol(char c) {
    type = TokenKind::SYMBOL;
    value = c;
  }

  /// @brief Assign an identifier token (e.g. variable name)
  void assign_identifier(const std::string &name) {
    type = TokenKind::IDENTIFIER;
    value = name;
  }

  /// @brief Assign a string token value
  void assign_string(const std::string &s) {
    type = TokenKind::LITERAL_STRING;
    value = s;
  }

  /// @brief Assign a numeric token value
  void assign_number(float val) {
    type = TokenKind::LITERAL_NUMBER;
    value = val;
  }

  /// @brief Print token information
  void print() const {
    std::cout << "Token Type: " + type_to_string();
    switch (type) {
    case TokenKind::STOP_TOKEN:
      break;
    case TokenKind::KEYWORD:
      std::cout << ", Value: " << to_string(std::get<KeywordEnum>(value))
                << " (KeywordEnum: " << static_cast<int>(std::get<KeywordEnum>(value)) << ")";
      break;
    case TokenKind::SYMBOL:
      std::cout << ", Value: " << std::get<char>(value);
      break;
    case TokenKind::IDENTIFIER:
      std::cout << ", Value: " << std::get<std::string>(value);
      break;
    case TokenKind::LITERAL_STRING:
      std::cout << ", Value: " << std::get<std::string>(value);
      break;
    case TokenKind::LITERAL_NUMBER:
      std::cout << ", Value: " << std::get<float>(value);
      break;
    }
    std::cout << ", Source Location: " << source_location.to_string() << std::endl;
  }

  /// @brief Get string corresponding to the token value
  std::string type_to_string() const {
    switch (type) {
    case TokenKind::STOP_TOKEN:
      return "STOP_TOKEN";
    case TokenKind::KEYWORD:
      return "KEYWORD";
    case TokenKind::SYMBOL:
      return "SYMBOL";
    case TokenKind::IDENTIFIER:
      return "IDENTIFIER";
    case TokenKind::LITERAL_STRING:
      return "LITERAL_STRING";
    case TokenKind::LITERAL_NUMBER:
      return "LITERAL_NUMBER";
    default:
      throw std::logic_error("Unreachable code: unknown TokenKind");
    }
  }
};

//----------------------------------------------------------------------------------------------------
//------------------- ERROR CLASS FOR GRAMMAR/ LEXER / PARSER ERRORS -------------------------
//----------------------------------------------------------------------------------------------------

/// @brief An error found by the lexer/parser while reading a scene file
class GrammarError : public std::exception {
public:
  //----------- Properties -----------
  SourceLocation location;  // Location in the source file
  std::string message;      // Human-readable error message (explanation)
  std::string full_message; // Cached full message (location and explanation) for what()

  //----------- Constructors -----------
  GrammarError(const SourceLocation &loc, const std::string &msg) : location(loc), message(msg) {
    full_message = "GrammarError at " + location.to_string() + ": " + message;
  }

  //----------- Methods -----------
  ///@brief Retrieve the error message as a C-style string (for use with std::exception)
  const char *what() const noexcept override { return full_message.c_str(); }
};

// ------------------------------------------------------------------------------------------------------------
// -----------INPUT STREAM CLASS: High-level wrapper around std::istream for scene file parsing-------------
// ------------------------------------------------------------------------------------------------------------

class InputStream {
public:
  //----------- Properties -----------
  std::istream &stream;                   // underlying input stream
  SourceLocation location;                // current location in the file (filename, line, column)
  std::optional<char> saved_char;         // character "pushback" buffer (std::optional since possibly empty)
  SourceLocation saved_location;          // SourceLocation corresponding to the saved_char; not an std::optional: it has a very
                                          // different role from that of saved_char
  int tabulations;                        // number of spaces a tab '\t' is worth (usually 4 or 8 spaces, default set to 4)
  std::optional<Token> saved_token;       // Token "pushback" buffer (nullptr if unused)
  SourceLocation last_on_stream_location; // most recent location of a token *in the stream* (as opposed to the location of the
                                          // saved token): needed after after reading consuming the saved token

  //----------- Constructor -----------
  /// @brief Deafult constructor for InputStream (does not take ownership of the stream)
  InputStream(std::istream &stream, const std::string &file_name = "", int tabulations = 4)
      : stream(stream), location(file_name), // Start at line 1, column 1
        saved_char(std::nullopt), saved_location(file_name), tabulations(tabulations), saved_token(std::nullopt) {}

  //------------------------------Methods------------------------------------

  //----------------------Helper methods for main function read_token() below ----------------------

  // --- UPDATE POSITION AFTER READING A CHARACTER --------------------------------------------------------------

  /// @brief Update the SourceLocation after reading character ch
  void _update_pos(char ch) {
    if (ch == 0) {
      // Do nothing if you reach end of the stream (read_char() returns '\0' in this case)
      return;
    } else if (ch == '\n') { // New line: increment line number and reset column
      location.line += 1;
      location.column = 1;
    } else if (ch == '\t') { // Tabulation: increment column by number of spaces a tab is worth
      location.column += tabulations;
    } else { // Any other character: just increment column
      location.column += 1;
    }
  }

  // --- READ A SINGLE CHARACTER FROM THE STREAM (handles pushback/unreading) ---------------------------------------------

  /// @brief Read a single character from the input stream, handling pushback/unreading of characters
  char read_char() {
    char ch;

    if (saved_char.has_value()) {
      ch = saved_char.value();
      saved_char = std::nullopt;
    } else {
      ch = static_cast<char>(stream.get());
      if (!stream) {
        ch = 0; // Assign 0, i. e. '\0' if you reach end of stream (hence this is the last character to be read)
      }
    }

    saved_location = location;
    _update_pos(ch); // Update the source location based on the character read
    return ch;
  }

  // --- UNREAD (PUSH BACK) A CHARACTER INTO THE STREAM ---------------------------------------------------------

  /// @brief Push back a single character into the input stream (for lookahead)
  void unread_char(char ch) {
    assert(!saved_char.has_value()); // Only one char of pushback at a time is supported, this should never happen
    saved_char = ch;
    location = saved_location; // Saved location should always have a value here. If for some reason it hasn't, an exception
                               // should be raised
  }

  // --- SKIP WHITESPACES AND COMMENTS (we start comments with # ... until end of line) -------------------------

  /// @brief Skip over whitespace characters and comments (starting with '#') in the input stream
  void _skip_whitespaces_and_comments() {
    char ch = read_char();
    while (_is_whitespace(ch) || ch == '#') {
      if (ch == '#') { // It's a comment: Skip until end of line or EOF
        while (true) {
          char next = read_char();
          if (next == '\n' || next == '\r' || next == 0) // If we reach end of line or end of file break
            break;
        }
      }
      ch = read_char(); // Read the first non-whitespace, non-comment character
      if (ch == 0) {
        return; // If end of file is reached, just return
      }
    }
    // Put back the first non-whitespace, non-comment char
    unread_char(ch);
  }

  // ----- CHECK FOR WHITESPACE (further helper function)--------------------------------------------------------

  /// @brief Check if a character is a whitespace character (space, tab, newline, carriage return)
  static bool _is_whitespace(char ch) {
    // Accepts ' ', '\t', '\n', '\r'
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
  }

  // --- PARSE A STRING TOKEN (between double quotes) -----------------------------------------------------------

  /// @brief Parse a string  from the input stream, expected to be enclosed in double quotes
  Token parse_string_token(const SourceLocation &token_location) {
    std::string token_str;
    while (true) {
      char ch = read_char();

      if (ch == '"') { // You got to the ethe nd of string
        break;
      }
      if (ch == 0) { // End of file before closing quote
        throw GrammarError(token_location, "unterminated string");
      }
      token_str += ch; // Append character to the string
    }

    // Create the token with the parsed string and the appropriate constructor/methods
    Token token(token_location, TokenKind::LITERAL_STRING);
    token.assign_string(token_str);
    return token;
  }

  // --- PARSE A FLOATING POINT NUMBER --------------------------------------------------------------------------

  /// @brief Parse a floating-point number from the input stream
  Token parse_float_token(char first_char, const SourceLocation &token_location) {
    std::string token_str(1, first_char);

    // Read stream characters until we reach a non-digit character
    while (true) {
      char ch = read_char();
      if (!std::isdigit(ch) && ch != '.' && ch != 'e' && ch != 'E') { // Allow digits, '.', 'e', 'E' for scientific notation
        unread_char(ch);
        break;
      }
      token_str += ch; // Append character to the number string and keep looping
    }

    std::size_t processed = 0; // number of characters of the string used to form the float
    float value;
    try {
      value = std::stof(token_str, &processed);
    } catch (...) {
      // If string to float conversion fails, throw a GrammarError specific for floating-point numbers
      throw GrammarError(token_location, "'" + token_str + "' is an invalid floating-point number");
    }
    if (processed != token_str.size()) {
      // Not all characters were consumed: malformed float (e.g. "12.3.4" or "12.4ab")
      throw GrammarError(token_location, "'" + token_str + "' is an invalid floating-point number");
    }

    // Create a token with the parsed number and the appropriate constructor/methods
    Token token(token_location, TokenKind::LITERAL_NUMBER);
    token.assign_number(value);
    return token;
  }

  // --- PARSE A KEYWORD OR IDENTIFIER --------------------------------------------------------------------------

  /// @brief Parse a keyword or identifier token from the input stream
  Token parse_keyword_or_identifier_token(char first_char, const SourceLocation &token_location) {
    std::string token_str(1, first_char);

    // Read characters until we reach a non-alphanumeric character
    while (true) {
      char ch = read_char();

      if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') { // Accept alphanumeric and '_' for snake_case identifiers
        // use std::isalnum built-in function for alphanumeric check
        unread_char(ch);
        break;
      }
      token_str += ch; // Append character to the alphanumeric string and keep looping
    }

    // Check if it is a keyword
    auto kw_it = KEYWORDS.find(token_str);
    if (kw_it != KEYWORDS.end()) { // if it is a keyword, create a token accordingly
      Token token(token_location, TokenKind::KEYWORD);
      token.assign_keyword(kw_it->second); // kw_it->second is the KeywordEnum value
      return token;
    } else { // If it is not a keyword, it must be an identifier
      Token token(token_location, TokenKind::IDENTIFIER);
      token.assign_identifier(token_str);
      return token;
    }
  }

  //--------------------- Main methods: read_token() and unread_token() -----------------------------

  // ----------- READ A TOKEN FROM THE STREAM ----------------------------------------------

  /// @brief Read a token from the input stream, skipping whitespace and comments
  /// @return token read from the stream, or a STOP_TOKEN if end of file is reached
  Token read_token() {
    if (saved_token.has_value()) { // Use saved token if available
      Token result = *saved_token;
      saved_token = std::nullopt;
      location = last_on_stream_location;
      return result;
    }

    _skip_whitespaces_and_comments(); // Get rid of whitespace and comments, then start reading the next token

    // Save current location for token start
    // (do it BEFORE reading the character: we want error location to be at the start of the token)
    SourceLocation token_location = location;

    char ch = read_char(); // Read the first character

    if (ch == 0) { // You got to the end of file: create and return a STOP_TOKEN
      Token token(token_location, TokenKind::STOP_TOKEN);
      return token;
    }

    // Handle single-character symbols or otherwise start parsing other token types
    if (SYMBOLS.find(ch) != std::string::npos) {      // std::string::npos means ch was not found in SYMBOLS string
      Token token(token_location, TokenKind::SYMBOL); // Create a symbol token
      token.assign_symbol(ch);
      _skip_whitespaces_and_comments();
      return token;
    } else if (ch == '"') {
      // If it starts with a double quote, parse a string token
      Token token = parse_string_token(token_location);
      _skip_whitespaces_and_comments();
      return token;
    } else if (std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.') {
      // If it starts with a digit or a sign, parse a float token
      Token token = parse_float_token(ch, token_location);
      _skip_whitespaces_and_comments();
      return token;
    } else if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
      // If it starts with an alphabetic character or '_', parse a keyword or identifier token
      Token token = parse_keyword_or_identifier_token(ch, token_location);
      _skip_whitespaces_and_comments();
      return token;
    } else {
      // If it is an invalid character (all valid cases are ruled out already), throw a GrammarError
      throw GrammarError(token_location, std::string("invalid character: '") + ch + "'");
    }
  }

  // --- UNREAD (PUSH BACK) A TOKEN -----------------------------------------------------------------------------

  /// @brief Push back a token into the input stream (for lookahead)
  void unread_token(const Token &token) {
    assert(!saved_token.has_value()); // Only one token of lookahead is supported (LL(1) grammar), so this should never happen
    saved_token = std::make_optional<Token>(token);
    last_on_stream_location = location;
    location = token.source_location;
  }
};

// ------------------------------------------------------------------------------------------------------------
// ----------------------------------- SCENE DATA STRUCTURE & PARSING METHODS ---------------------------------
// ------------------------------------------------------------------------------------------------------------

class Scene {
public:
  // ---- properties ----
  std::unordered_map<std::string, std::shared_ptr<Material>> materials; // map of material names to Material objects
  std::shared_ptr<World> world;                                         // world object top render
  std::shared_ptr<Camera> camera = nullptr;                             // camera used for firing rays
  std::unordered_map<std::string, float> float_variables;               // float identifiers table
  std::unordered_set<std::string> overwritten_variables; // set of float identifiers that can be overwritten from command line

  // -------- constructors --------
  Scene() : world(std::make_shared<World>()) {}

  // -------- methods --------

  // -------- parsing helpers: EXPECT_* --------

  /// @brief Read a token and check that it matches the symbol expected from our grammar.
  void expect_symbol(InputStream &input_stream, char symbol) {
    Token token = input_stream.read_token();
    if (token.type != TokenKind::SYMBOL || std::get<char>(token.value) != symbol) {
      throw GrammarError(token.source_location,
                         "got '" + std::string(1, std::get<char>(token.value)) + "' instead of '" + symbol + "'");
    }
  }

  /// @brief Read a token and check it is one of the keywords your grammar allow in that position. Return the keyword.
  KeywordEnum expect_keywords(InputStream &input_stream, const std::vector<KeywordEnum> &keywords) {
    // second argument is a {...} list of keywords from KeywordEnum class
    Token token = input_stream.read_token();
    if (token.type != TokenKind::KEYWORD) {
      throw GrammarError(token.source_location, "expected KEYWORD instead of " + token.type_to_string());
    }
    KeywordEnum kw = std::get<KeywordEnum>(token.value);
    for (const auto &x : keywords) { // check if the keyword is one of the expected ones
      if (kw == x)
        return kw;
    }
    throw GrammarError(token.source_location, "unexpected KEYWORD");
  }

  /// @brief Read a token and check that it is either a literal number or a float variable defined in scene
  float expect_number(InputStream &input_stream) {
    Token token = input_stream.read_token();
    if (token.type == TokenKind::LITERAL_NUMBER) {
      return std::get<float>(token.value);
    } else if (token.type == TokenKind::IDENTIFIER) {
      const std::string &variable_name = std::get<std::string>(token.value);
      auto map_it = float_variables.find(variable_name); // Look for the map entry with the variable name
      if (map_it == float_variables.end()) {             // If not found, throw an error
        throw GrammarError(token.source_location, "unknown variable \"" + variable_name + "\"");
      }
      return map_it->second; // Otherwise return the value of the variable stored in the `dictionary'
    } else {
      throw GrammarError(token.source_location, "expected LITERAL_NUMBER or IDENTIFIER instead of " + token.type_to_string());
    }
  }

  /// @brief Read a token and check that it is a literal string
  std::string expect_string(InputStream &input_stream) {
    Token token = input_stream.read_token();
    if (token.type != TokenKind::LITERAL_STRING) {
      throw GrammarError(token.source_location, "expected LITERAL_STRING instead of " + token.type_to_string());
    }
    return std::get<std::string>(token.value);
  }

  /// @brief Read a token and check that it is an identifier
  std::string expect_identifier(InputStream &input_stream) {
    Token token = input_stream.read_token();
    if (token.type != TokenKind::IDENTIFIER) {
      throw GrammarError(token.source_location, "expected IDENTIFIER instead of " + token.type_to_string());
    }
    return std::get<std::string>(token.value);
  }

  //-----------------PARSER METHODS-----------------

  ///@brief Parse a vector from the input stream (expected format in our grammar: [x, y, z])
  Vec parse_vector(InputStream &input_stream) {
    expect_symbol(input_stream, '[');
    float x = expect_number(input_stream);
    expect_symbol(input_stream, ',');
    float y = expect_number(input_stream);
    expect_symbol(input_stream, ',');
    float z = expect_number(input_stream);
    expect_symbol(input_stream, ']');
    return Vec(x, y, z);
  }

  /// @brief Parse a color from the input stream (expected format in our grammar: <r, g, b>)
  Color parse_color(InputStream &input_stream) {
    expect_symbol(input_stream, '<');
    float red = expect_number(input_stream);
    expect_symbol(input_stream, ',');
    float green = expect_number(input_stream);
    expect_symbol(input_stream, ',');
    float blue = expect_number(input_stream);
    expect_symbol(input_stream, '>');
    return Color(red, green, blue);
  }

  ///@brief Parse a pigment from the input stream according to the expected format
  std::shared_ptr<Pigment> parse_pigment(InputStream &input_file) {
    std::shared_ptr<Pigment> result;

    // Make sure the pigment name is one of those expected (and currently implemented)
    KeywordEnum pigment_keyword = expect_keywords(input_file, {KeywordEnum::UNIFORM, KeywordEnum::CHECKERED, KeywordEnum::IMAGE});

    expect_symbol(input_file, '(');

    // Parse the expected structure depending on the type of the pigment
    switch (pigment_keyword) {
    case KeywordEnum::UNIFORM: {
      Color color = parse_color(input_file);
      result = std::make_shared<UniformPigment>(color);
      break;
    }
    case KeywordEnum::CHECKERED: {
      Color color1 = parse_color(input_file);
      expect_symbol(input_file, ',');
      Color color2 = parse_color(input_file);
      expect_symbol(input_file, ',');
      int n_intervals = static_cast<int>(expect_number(input_file));
      result = std::make_shared<CheckeredPigment>(color1, color2, n_intervals);
      break;
    }
    case KeywordEnum::IMAGE: {
      std::string file_name = expect_string(input_file);
      HdrImage image{file_name};
      result = std::make_shared<ImagePigment>(image);
      break;
    }
    default:
      throw std::logic_error("Unreachable code: unknown pigment type");
    }

    expect_symbol(input_file, ')');
    return result;
  }

  /// @brief Parse a BRDF from the input stream according to the expected format
  std::shared_ptr<BRDF> parse_brdf(InputStream &input_stream) {

    // Make sure the BRDF type is one of those expected (and currently implemented)
    KeywordEnum brdf_keyword = expect_keywords(input_stream, {KeywordEnum::DIFFUSE, KeywordEnum::SPECULAR});
    expect_symbol(input_stream, '(');
    std::shared_ptr<Pigment> pigment = parse_pigment(input_stream);
    expect_symbol(input_stream, ')');

    // Depending on the type of BRDF, create the appropriate object
    switch (brdf_keyword) {
    case KeywordEnum::DIFFUSE:
      return std::make_shared<DiffusiveBRDF>(pigment);
    case KeywordEnum::SPECULAR:
      return std::make_shared<SpecularBRDF>(pigment);
    default:
      throw std::logic_error("Unreachable code: unknown BRDF type");
    }
  }

  /// @brief Parse a Material from the input stream according to the expected format
  std::shared_ptr<Material> parse_material(InputStream &input_stream) {
    expect_symbol(input_stream, '(');
    auto brdf = parse_brdf(input_stream);
    expect_symbol(input_stream, ',');
    auto emitted_radiance = parse_pigment(input_stream);
    expect_symbol(input_stream, ')');

    return std::make_shared<Material>(brdf, emitted_radiance);
  }

  /// @brief Parse a Transformation from the input stream: lookahead of one token required
  Transformation parse_transformation(InputStream &input_stream) {
    Transformation result{}; // Initialize with identity

    while (true) {
      KeywordEnum transformation_keyword =
          expect_keywords(input_stream, {KeywordEnum::IDENTITY, KeywordEnum::TRANSLATION, KeywordEnum::ROTATION_X,
                                         KeywordEnum::ROTATION_Y, KeywordEnum::ROTATION_Z, KeywordEnum::SCALING});
      switch (transformation_keyword) {
      case KeywordEnum::IDENTITY: {
        break;
      }
      case KeywordEnum::TRANSLATION: {
        expect_symbol(input_stream, '(');
        result = result * translation(parse_vector(input_stream));
        expect_symbol(input_stream, ')');
        break;
      }
      case KeywordEnum::ROTATION_X: {
        expect_symbol(input_stream, '(');
        result = result * rotation_x(degs_to_rads(expect_number(input_stream)));
        expect_symbol(input_stream, ')');
        break;
      }
      case KeywordEnum::ROTATION_Y: {
        expect_symbol(input_stream, '(');
        result = result * rotation_y(degs_to_rads(expect_number(input_stream)));
        expect_symbol(input_stream, ')');
        break;
      }
      case KeywordEnum::ROTATION_Z: {
        expect_symbol(input_stream, '(');
        result = result * rotation_z(degs_to_rads(expect_number(input_stream)));
        expect_symbol(input_stream, ')');
        break;
      }
      case KeywordEnum::SCALING: {
        expect_symbol(input_stream, '(');
        Vec scaling_vec = parse_vector(input_stream);
        result = result * scaling({scaling_vec.x, scaling_vec.y, scaling_vec.z});
        expect_symbol(input_stream, ')');
        break;
      }
      default:
        throw std::logic_error("Unreachable code: unknown transformation type");
      }

      Token next_token = input_stream.read_token();
      if (next_token.type != TokenKind::SYMBOL || std::get<char>(next_token.value) != '*') {
        input_stream.unread_token(next_token);
        break;
      }
    }

    return result;
  }

  /// @brief Parse the description of a Sphere from the input stream
  std::shared_ptr<Sphere> parse_sphere(InputStream &input_stream) {
    // Parse transformation
    expect_symbol(input_stream, '(');
    Transformation sphere_transformation = parse_transformation(input_stream);
    expect_symbol(input_stream, ',');

    // Parse material identifier
    SourceLocation source_location =
        input_stream.location; // Save location of the material identifier token in case an exception needs to be raised
    std::string material_identifier = expect_identifier(input_stream);
    auto materials_it = materials.find(material_identifier); // Look for the material in the material map
    if (materials_it == materials.end()) {                   // If not found, throw an error
      throw GrammarError(source_location, "unknown material \"" + material_identifier + "\"");
    }

    expect_symbol(input_stream, ')');
    return std::make_shared<Sphere>(sphere_transformation, materials_it->second);
  }

  /// @brief Parse the description of a Plane from the input stream
  std::shared_ptr<Plane> parse_plane(InputStream &input_stream) {
    // Parse transformation
    expect_symbol(input_stream, '(');
    Transformation plane_transformation = parse_transformation(input_stream);
    expect_symbol(input_stream, ',');

    // Parse material identifier
    SourceLocation source_location =
        input_stream.location; // Save location of the material identifier token in case an exception needs to be raised
    std::string material_identifier = expect_identifier(input_stream);
    auto materials_it = materials.find(material_identifier); // Look for the material in the material map
    if (materials_it == materials.end()) {                   // If not found, throw an error
      throw GrammarError(source_location, "unknown material \"" + material_identifier + "\"");
    }

    expect_symbol(input_stream, ')');
    return std::make_shared<Plane>(plane_transformation, materials_it->second);
  }

  /// @brief Parse the description of a Camera from the input stream
  std::shared_ptr<Camera> parse_camera(InputStream &input_stream) {
    expect_symbol(input_stream, '(');

    // Parse Camera type
    KeywordEnum camera_type = expect_keywords(input_stream, {KeywordEnum::PERSPECTIVE, KeywordEnum::ORTHOGONAL});

    // Parse Transformation
    expect_symbol(input_stream, ',');
    Transformation transformation = parse_transformation(input_stream);

    // Parse aspect ratio
    std::optional<float> asp_ratio;
    expect_symbol(input_stream, ',');
    Token asp_ratio_token = input_stream.read_token();
    if (asp_ratio_token.type == TokenKind::KEYWORD) {
      input_stream.unread_token(asp_ratio_token); // NOTE is it ok to use unread even if strictly speaking it is not necessary?
      expect_keywords(input_stream, {KeywordEnum::EXACT_ASP_RATIO});
      asp_ratio = std::nullopt;
    } else {
      input_stream.unread_token(asp_ratio_token);
      asp_ratio = std::make_optional<float>(expect_number(input_stream));
    }

    // Parse screen-observer distance (only in case of a PerspectiveCamera)
    float distance;
    if (camera_type == KeywordEnum::PERSPECTIVE) {
      expect_symbol(input_stream, ',');
      distance = expect_number(input_stream);
    }

    expect_symbol(input_stream, ')');
    if (camera_type == KeywordEnum::PERSPECTIVE) {
      auto cam = std::make_shared<PerspectiveCamera>(distance, asp_ratio, transformation);
      return cam;
    } else { // Only other case: KeywordEnum::ORTHOGONAL
      return std::make_shared<OrthogonalCamera>(asp_ratio, transformation);
    }
  }

  /// @brief Parse the description of a PointLightSource from the input stream
  std::shared_ptr<PointLightSource> parse_point_light(InputStream &input_stream) {
    expect_symbol(input_stream, '(');

    // Parse position
    Vec position = parse_vector(input_stream);

    // Parse emitted radiance
    expect_symbol(input_stream, ',');
    Color emitted_radiance = parse_color(input_stream);

    // Parse emission radius
    expect_symbol(input_stream, ',');
    float emission_radius = expect_number(input_stream);

    expect_symbol(input_stream, ')');
    return std::make_shared<PointLightSource>(position.to_point(), emitted_radiance, emission_radius);
  }

  /// @brief Parse a scene from a stream
  /// @details It is meant to be called after initialize_float_variables_with_priority() if you want to overwrite float vars from
  /// command line
  void parse_scene(InputStream &input_stream) {
    // The scene actually consists of a sequence of definitions. The user is allowed to define the following types: float,
    // material, sphere, plane, camera, point_light
    while (true) {
      Token new_token = input_stream.read_token();
      if (new_token.type == TokenKind::STOP_TOKEN) {
        break;
      } else {
        input_stream.unread_token(new_token);
      }

      SourceLocation source_location = input_stream.location;
      KeywordEnum keyword = expect_keywords(input_stream, {KeywordEnum::FLOAT, KeywordEnum::MATERIAL, KeywordEnum::SPHERE,
                                                           KeywordEnum::PLANE, KeywordEnum::CAMERA, KeywordEnum::POINT_LIGHT});
      switch (keyword) {
      case KeywordEnum::FLOAT: {
        std::string float_name = expect_identifier(input_stream);

        // Throw if a variable with the same name has already been defined but is not among the overwritten ones
        if (float_variables.count(float_name) && !overwritten_variables.count(float_name)) {
          throw GrammarError(source_location, "float variable \"" + float_name + "\" already declared elsewhere in the file");
        }
        expect_symbol(input_stream, '(');
        float float_value = expect_number(input_stream);
        expect_symbol(input_stream, ')');

        // Assign value only if float_name is not among the overwritten variables
        if (!overwritten_variables.count(float_name)) {
          float_variables[float_name] = float_value;
        }
        break;
      }

      case KeywordEnum::MATERIAL: {
        std::string material_name = expect_identifier(input_stream);
        // Check if a variable with the same name has already been defined, throw exception in case it has
        if (materials.count(material_name)) {
          throw GrammarError(source_location, "material variable \"" + material_name + "\" already declared");
        }
        // Parse material definition and add map entry
        materials[material_name] = parse_material(input_stream);
        break;
      }

      case KeywordEnum::SPHERE: {
        // Add Sphere to World
        world->add_object(parse_sphere(input_stream));
        break;
      }

      case KeywordEnum::PLANE: {
        // Add Plane to World
        world->add_object(parse_plane(input_stream));
        break;
      }

      case KeywordEnum::CAMERA: {
        // Throw if a Camera was already defined
        if (camera) {
          throw GrammarError(source_location, "camera already defined");
        }
        // Parse Camera and assign to Scene data member
        camera = parse_camera(input_stream);
        break;
      }

      case KeywordEnum::POINT_LIGHT: {
        // Add Point Light to World
        world->add_light_source(parse_point_light(input_stream));
        break;
      }

      default:
        throw GrammarError(source_location, "definition of \"" + to_string(keyword) + "\" not allowed");
      }
    }
  }

  void initialize_float_variables_with_priority(std::unordered_map<std::string, float> &&variables_from_cl) {
    assert(float_variables.empty()); // For the logic of parse_scene() to work correctly, float variables from command line are to
                                     // be parsed and added to float_variables before parsing the scene file

    float_variables = std::move(variables_from_cl);
    for (const auto &name : float_variables) {
      overwritten_variables.insert(name.first);
    }
  }
};
