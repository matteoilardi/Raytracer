// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// ------------ LIBRARY FOR COMPILERS -----------------
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#include <cassert> // C++ library for assert (for debugging)
#include <cctype>  // C++ library for character classification functions (isalpha, isdigit, etc.)
#include <iostream>
#include <istream>
#include <memory>   // C++ library for std::shared_ptr (used for pushback token)
#include <optional> // C++ library for std::optional (used for pushback character and token)
#include <string>
#include <unordered_map> // C++ library for unordered maps (analogous to Python dictionaries) much faster than std::map
#include <unordered_set> // C++ library for unordered sets (necessary for command line overwriting of variables)
#include <variant>       // C++17 library for type-safe tagged union replacement (used for TokenValue)

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

const std::string SYMBOLS = "(){},[]:;=<>"; // declare our symbols // NOTE adjust as needed
enum class KeywordEnum;                     // forward declare our keywords (see section below)

//----------------------------------------------------------------------------------------------------
//------------------- KEYWORDS and related HELPERS -------------------------
//----------------------------------------------------------------------------------------------------

/// @brief Enum class representing the keywords used in the scene files
enum class KeywordEnum {
  NONE = 0, // default fallback
  NEW,
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
  FLOAT,
  POINT_LIGHT
};

/// @brief map of keywords to their enum values (analogue of Python dictionaries)
const std::unordered_map<std::string, KeywordEnum> KEYWORDS = {{"new", KeywordEnum::NEW},
                                                               {"material", KeywordEnum::MATERIAL},
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
                                                               {"float", KeywordEnum::FLOAT},
                                                               {"point_light", KeywordEnum::POINT_LIGHT}};

/// @brief convert a keyword enum to its string representation (for debugging and printing)
std::string to_string(KeywordEnum kw) {
  switch (kw) {
  case KeywordEnum::NEW:
    return "new";
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
  case KeywordEnum::FLOAT:
    return "float";
  case KeywordEnum::POINT_LIGHT:
    return "point_light";
  default:
    return "unknown";
  }
}

//----------------------------------------------------------------------------------------------------
//------------------- TOKEN CLASS (using std::variant instead of union) and AUXILIARY SUBCLASSES -------------------------
//----------------------------------------------------------------------------------------------------

//------------SOURCE LOCATION: contains file name, line number and column number of the token----

struct SourceLocation {
  //------- Properties --------
  std::string file; // File name (or empty string if not applicable e.g. source code was provided as a memory stream)
  int line;         // Line number (starting from 1)
  int column;       // Column number (starting from 1)

  //----------- Constructors -----------
  /// Default constructor initializes to empty file name, line one and column one
  SourceLocation(const std::string &file = "", int line = 1, int column = 1) : file(file), line(line), column(column) {}

  //----------- Methods -----------
  ///@brief convert source location to string (for debugging)
  std::string to_string() const {
    return "File: " + file + ", Line: " + std::to_string(line) + ", Column: " + std::to_string(column);
  }
};

//------------ TOKEN TYPE (used for dispatching at runtime)---------------

enum class TokenType {
  // these are the 6 types of token implemented in Pytracer
  STOP_TOKEN,     // A token signalling the end of a file
  KEYWORD,        // A token containing a keyword
  SYMBOL,         // A token containing a symbol
  IDENTIFIER,     // A token containing an identifier (i.e. a variable name)
  LITERAL_STRING, // A token containing a literal string
  LITERAL_NUMBER  // A token containing a literal number (i.e. a float)
};

// ------------ TOKEN VALUE: variant for possible token values ---------------
using TokenValue = std::variant<std::monostate, KeywordEnum, char, std::string, float>;

//------ TOKEN CLASS: product of `SourceLocation', `TokenType` and `TokenValue` (tagged union pattern in C++) ------
class Token {
public:
  //------- Properties --------
  SourceLocation source_location; // The source location
  TokenType type;                 // The "tag"
  TokenValue value;               // The type-safe tagged union

  //----------- Constructors -----------
  Token(const SourceLocation &loc, TokenType t) : source_location(loc), type(t), value(std::monostate{}) {
    switch (type) {
    case TokenType::STOP_TOKEN:
      // No value needed for STOP_TOKEN
      break;
    case TokenType::KEYWORD:
      value = KeywordEnum::NONE;
      break;
    case TokenType::SYMBOL:
      value = '\0';
      break;
    case TokenType::IDENTIFIER:
      value = std::string("");
      break;
    case TokenType::LITERAL_STRING:
      value = std::string("");
      break;
    case TokenType::LITERAL_NUMBER:
      value = 0.f;
      break;
    }
  }

  //----------- Methods -----------

  ///@brief assign a stop token (signals EndOfFile)
  void assign_stop_token() {
    type = TokenType::STOP_TOKEN;
    value = std::monostate{}; // No value needed for STOP_TOKEN
  }

  ///@brief assign a keyword token
  void assign_keyword(KeywordEnum kw) {
    type = TokenType::KEYWORD;
    value = kw;
  }

  ///@brief assign a single character symbol token
  void assign_symbol(char c) {
    type = TokenType::SYMBOL;
    value = c;
  }

  ///@brief assign an identifier token (e.g. variable name)
  void assign_identifier(const std::string &name) {
    type = TokenType::IDENTIFIER;
    value = name;
  }

  ///@brief assign a string token value
  void assign_string(const std::string &s) {
    type = TokenType::LITERAL_STRING;
    value = s;
  }

  ///@brief assign a numeric token value
  void assign_number(float val) {
    type = TokenType::LITERAL_NUMBER;
    value = val;
  }

  ///@brief print token information
  void print() const {
    std::cout << "Token Type: " + type_to_string();
    switch (type) {
    case TokenType::STOP_TOKEN:
      break;
    case TokenType::KEYWORD:
      std::cout << ", Value: " << to_string(std::get<KeywordEnum>(value))
                << " (KeywordEnum: " << static_cast<int>(std::get<KeywordEnum>(value)) << ")";
      break;
    case TokenType::SYMBOL:
      std::cout << ", Value: " << std::get<char>(value);
      break;
    case TokenType::IDENTIFIER:
      std::cout << ", Value: " << std::get<std::string>(value);
      break;
    case TokenType::LITERAL_STRING:
      std::cout << ", Value: " << std::get<std::string>(value);
      break;
    case TokenType::LITERAL_NUMBER:
      std::cout << ", Value: " << std::get<float>(value);
      break;
    }
    std::cout << ", Source Location: " << source_location.to_string() << std::endl;
  }

  ///@brief get the token value as a string
  std::string type_to_string() const {
    switch (type) {
    case TokenType::STOP_TOKEN:
      return "STOP_TOKEN";
    case TokenType::KEYWORD:
      return "KEYWORD";
    case TokenType::SYMBOL:
      return "SYMBOL";
    case TokenType::IDENTIFIER:
      return "IDENTIFIER";
    case TokenType::LITERAL_STRING:
      return "LITERAL_STRING";
    case TokenType::LITERAL_NUMBER:
      return "LITERAL_NUMBER";
    }
  }
};

//----------------------------------------------------------------------------------------------------
//------------------- ERROR CLASS FOR GRAMMAR/ LEXER / PARSER ERRORS -------------------------
//----------------------------------------------------------------------------------------------------

/// @brief An error found by the lexer/parser while reading a scene file
///
/// The fields of this class are:
/// - `location`: the source location where the error was discovered
/// - `message`: a user-friendly error message
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
  std::istream &stream;             // underlying input stream
  SourceLocation location;          // current location in the file (filename, line, column)
  std::optional<char> saved_char;   // Character "pushback" buffer (std::optional since possibly empty)
  SourceLocation saved_location;    // SourceLocation corresponding to the saved_char; not an std::optional: it has a very
                                    // different role from that of saved_char
  int tabulations;                  // Number of spaces a tab '\t' is worth (usually 4 or 8 spaces, default set to 8)
  std::optional<Token> saved_token; // Token "pushback" buffer (nullptr if unused)

  //----------- Constructor -----------
  /// @brief deafult constructor for InputStream (does not take ownership of the stream)
  InputStream(std::istream &stream, const std::string &file_name = "", int tabulations = 8)
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
    } else if (ch == '\n') { // new line: increment line number and reset column
      location.line += 1;
      location.column = 1;
    } else if (ch == '\t') { // tabulation: increment column by number of spaces a tab is worth
      location.column += tabulations;
    } else { // any other character: just increment column
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
          if (next == '\n' || next == '\r' || next == 0) // if we reach end of line or end of file break
            break;
        }
      }
      ch = read_char(); // read the first non-whitespace, non-comment character
      if (ch == 0) {
        return; // we got to the End of file, just return
      }
    }
    // Put back the first non-whitespace, non-comment char
    unread_char(ch);
  }

  // ----- CHECK FOR WHITESPACE (further helper function)
  // ---------------------------------------------------------------------------

  /// @brief Check if a character is a whitespace character (space, tab, newline, carriage return)
  static bool _is_whitespace(char ch) {
    // Accepts ' ', '\t', '\n', '\r'
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
  }

  // --- PARSE A STRING TOKEN (between double quotes) -----------------------------------------------------------
  // QUESTION should we support also single quotes? (e.g. 'string' instead of "string", Tomasi said we could)

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

    // create the token with the parsed string and the appropriate constructor/methods
    Token token(token_location, TokenType::LITERAL_STRING);
    token.assign_string(token_str);
    return token;
  }

  // --- PARSE A FLOATING POINT NUMBER --------------------------------------------------------------------------

  /// @brief Parse a floating-point number from the input stream
  Token parse_float_token(char first_char, const SourceLocation &token_location) {
    std::string token_str(1, first_char);

    // read stream characters until we reach a non-digit character
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
    Token token(token_location, TokenType::LITERAL_NUMBER);
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
      Token token(token_location, TokenType::KEYWORD);
      token.assign_keyword(kw_it->second); // kw_it->second is the KeywordEnum value
      return token;
    } else { // If it is not a keyword, it must be an identifier
      Token token(token_location, TokenType::IDENTIFIER);
      token.assign_identifier(token_str);
      return token;
    }
  }

  //--------------------- Main methods: read_token() and unread_token() -----------------------------

  // ----------- READ A TOKEN FROM THE STREAM ----------------------------------------------

  /// @brief read a token from the input stream, skipping whitespace and comments
  /// @return token read from the stream, or a STOP_TOKEN if end of file is reached
  Token read_token() {
    if (saved_token.has_value()) { // Use saved token if available
      Token result = *saved_token; // Dereference the shared pointer to get the actual Token
      saved_token = std::nullopt;
      return result;
    }

    _skip_whitespaces_and_comments(); // get rid of whitespace and comments, then start reading the next token

    // Save current location for token start
    // (do it BEFORE reading the character: we want error location to be at the start of the token)
    SourceLocation token_location = location;

    char ch = read_char(); // read the first character

    if (ch == 0) { // You got to the end of file: create and return a STOP_TOKEN
      Token token(token_location, TokenType::STOP_TOKEN);
      return token;
    }

    // Handle single-character symbols or otherwise start parsing other token types
    if (SYMBOLS.find(ch) != std::string::npos) {      // std::string::npos means ch was not found in SYMBOLS string
      Token token(token_location, TokenType::SYMBOL); // Create a symbol token
      token.assign_symbol(ch);
      return token;
    } else if (ch == '"') {
      // If it starts with a double quote, parse a string token
      return parse_string_token(token_location);
    } else if (std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.') {
      // If it starts with a digit or a sign, parse a float token
      return parse_float_token(ch, token_location);
    } else if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
      // If it starts with an alphabetic character or '_', parse a keyword or identifier token
      return parse_keyword_or_identifier_token(ch, token_location);
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
  }
};

// ------------------------------------------------------------------------------------------------------------
// ----------------------------------- SCENE DATA STRUCTURE & PARSING METHODS ---------------------------------
// ------------------------------------------------------------------------------------------------------------

// NOTE I am implementing the parse functions as methods of the scene class, but I am not sure it is the best option
//  in the other case these methods should access the FloatVariables and overriddenVariables in scene class

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
    if (token.type != TokenType::SYMBOL || std::get<char>(token.value) != symbol) {
      throw GrammarError(token.source_location,
                         "got '" + std::string(1, std::get<char>(token.value)) + "' instead of '" + symbol + "'");
    }
  }

  /// @brief Read a token and check it is one of the keywords your grammar allow in that position. Return the keyword.
  KeywordEnum expect_keywords(InputStream &input_stream, const std::vector<KeywordEnum> &keywords) {
    // second argument is a {...} list of keywords from KeywordEnum class
    Token token = input_stream.read_token();
    if (token.type != TokenType::KEYWORD) {
      throw GrammarError(token.source_location, "expected KEYWORD instead of '" + token.type_to_string() + "'");
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
    if (token.type == TokenType::LITERAL_NUMBER) {
      return std::get<float>(token.value);
    } else if (token.type == TokenType::IDENTIFIER) {
      const std::string &variable_name = std::get<std::string>(token.value);
      auto map_it = float_variables.find(variable_name); // Look for the map entry with the variable name
      if (map_it == float_variables.end()) {             // If not found, throw an error
        throw GrammarError(token.source_location, "unknown variable \"" + variable_name + "\"");
      }
      return map_it->second; // Otherwise return the value of the variable stored in the `dictionary'
    } else {
      throw GrammarError(token.source_location,
                         "expected LITERAL_NUMBER or IDENTIFIER instead of \"" + token.type_to_string() + "\"");
    }
  }

  /// @brief Read a token and check that it is a literal string
  std::string expect_string(InputStream &input_stream) {
    Token token = input_stream.read_token();
    if (token.type != TokenType::LITERAL_STRING) {
      throw GrammarError(token.source_location, "expected LITERAL_STRING instead of \"" + token.type_to_string() + "\"");
    }
    return std::get<std::string>(token.value);
  }

  /// @brief Read a token and check that it is an identifier
  std::string expect_identifier(InputStream &input_stream) {
    Token token = input_stream.read_token();
    if (token.type != TokenType::IDENTIFIER) {
      throw GrammarError(token.source_location, "expected IDENTIFIER instead of \"" + token.type_to_string() + "\"");
    }
    return std::get<std::string>(token.value);
  }

  //-----------------PARSER METHODS-----------------

  ///@brief parse a vector from the input stream (expected format in our grammar: [x, y, z])
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

  /// @brief parse a color from the input stream (expected format in our grammar: <r, g, b>)
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

  ///@brief parse a pigment from the input stream according to the expected format
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
      std::ifstream image_file(
          file_name, std::ios::binary); // ios::binary is to open the file in binary mode (for non text files, like images)
      if (!image_file) {
        throw GrammarError(input_file.location, "could not open image file: " + file_name);
      }
      auto image = std::make_shared<HdrImage>(image_file);
      result = std::make_shared<ImagePigment>(*image);
      break;
    }
    default:
      throw GrammarError(input_file.location, "unknown pigment type");
    }

    expect_symbol(input_file, ')');
    return result;
  }
  // TODO 1) throw different exception if !file, 2) does it work for images?, 3) check that number is indeed an integer, 4) even
  // the default case of the switch should not throw a GrammarError to be fair (same problem in parse brdf and parse
  // transformation)

  /// @brief parse a BRDF from the input stream according to the expected format
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
      throw GrammarError(input_stream.location, "unknown BRDF type");
    }
  }

  /// @brief parse a Material from the input stream according to the expected format
  std::shared_ptr<Material> parse_material(InputStream &input_stream) {
    expect_symbol(input_stream, '(');
    auto brdf = parse_brdf(input_stream);
    expect_symbol(input_stream, ',');
    auto emitted_radiance = parse_pigment(input_stream);
    expect_symbol(input_stream, ')');

    return std::make_shared<Material>(brdf, emitted_radiance);
  }

  /// @brief parse a Transformation from the input stream: lookahead of one token required
  Transformation parse_transformation(InputStream &input_stream) {
    Transformation result{};

    while (true) {
      KeywordEnum transformation_keyword =
          expect_keywords(input_stream, {KeywordEnum::IDENTITY, KeywordEnum::TRANSLATION, KeywordEnum::ROTATION_X,
                                         KeywordEnum::ROTATION_Y, KeywordEnum::ROTATION_Z, KeywordEnum::SCALING});
      expect_symbol(input_stream, '(');
      switch (transformation_keyword) {
      case KeywordEnum::IDENTITY: {
      }
      case KeywordEnum::TRANSLATION: {
        expect_symbol(input_stream, '(');
        result = result * translation(parse_vector(input_stream));
        expect_symbol(input_stream, ')');
        break;
      }
      case KeywordEnum::ROTATION_X: {
        expect_symbol(input_stream, '(');
        result = result * rotation_x(expect_number(input_stream));
        expect_symbol(input_stream, ')');
        break;
      }
      case KeywordEnum::ROTATION_Y: {
        expect_symbol(input_stream, '(');
        result = result * rotation_y(expect_number(input_stream));
        expect_symbol(input_stream, ')');
        break;
      }
      case KeywordEnum::ROTATION_Z: {
        expect_symbol(input_stream, '(');
        result = result * rotation_z(expect_number(input_stream));
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
        throw GrammarError(input_stream.location, "unknown transformation type");
      }
      expect_symbol(input_stream, ')');

      Token next_token = input_stream.read_token();
      if (next_token.type != TokenType::SYMBOL || std::get<char>(next_token.value) != '*') {
        input_stream.unread_token(next_token);
        break;
      }
    }

    return result;
  }

  /// @brief parse the description of a Sphere from the input stream
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

  /// @brief parse the description of a Plane from the input stream
  std::shared_ptr<Sphere> parse_plane(InputStream &input_stream) {
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
    return std::make_shared<Sphere>(plane_transformation, materials_it->second);
  }

  /// @brief parse the description of a Camera from the input stream
  std::shared_ptr<Camera> parse_camera(InputStream &input_stream) {
    expect_symbol(input_stream, '(');

    // Parse Camera type
    KeywordEnum camera_type = expect_keywords(input_stream, {KeywordEnum::PERSPECTIVE, KeywordEnum::ORTHOGONAL});

    // Parse Transformation
    expect_symbol(input_stream, ',');
    Transformation transformation = parse_transformation(input_stream);

    // Parse aspect ratio
    expect_symbol(input_stream, ',');
    float asp_ratio = expect_number(input_stream);

    // Parse screen-observer distance (only in case of a PerspectiveCamera)
    float distance;
    if (camera_type == KeywordEnum::PERSPECTIVE) {
      expect_symbol(input_stream, ',');
      distance = expect_number(input_stream);
    }

    expect_symbol(input_stream, ')');
    if (camera_type == KeywordEnum::PERSPECTIVE) {
      return std::make_shared<PerspectiveCamera>(distance, asp_ratio, transformation);
    } else { // Only other case: KeywordEnum::ORTHOGONAL
      return std::make_shared<OrthogonalCamera>(asp_ratio, transformation);
    }
  }

  /// @brief parse the description of a PointLightSource from the input stream
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

    return std::make_shared<PointLightSource>(position.to_point(), emitted_radiance, emission_radius);
  }
  // TODO perhaps you want to allow the user to provide arguments in a different order and to omit emission_radius

  void parse_scene(InputStream &input_stream) {
    while (true) {
      Token new_token = input_stream.read_token();
      if (new_token.type == TokenType::STOP_TOKEN) {
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
        // See if a variable with the same name has already been defined
        auto map_it = float_variables.find(float_name);
        if (map_it != float_variables.end()) {
          // If so, see if it is a variable for which overwriting is allowed
          auto set_it = overwritten_variables.find(float_name);
          if (set_it == overwritten_variables.end()) {
            throw GrammarError(source_location,
                               "float variable \"" + float_name + "\" already declared and overwriting not allowed");
          }
        } else {
          // If the variable can be defined or overwritten add map entry
          expect_symbol(input_stream, '(');
          float_variables[float_name] = expect_number(input_stream);
          expect_symbol(input_stream, ')');
        }
        break;
      }
      case KeywordEnum::MATERIAL: {
        std::string material_name = expect_identifier(input_stream);
        // See if a variable with the same name has already been defined, throw in this case
        auto map_it = materials.find(material_name);
        if (map_it != materials.end()) {
          throw GrammarError(source_location, "float variable \"" + material_name + "\" already declared");
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
          throw GrammarError(source_location, "a camera was already defined");
        }
        // Parse Camera and assign to Scene data member
        camera = parse_camera(input_stream);
        break;
      }
      case KeywordEnum::POINT_LIGHT: {
        // Add Point Light to World
        world->add_light_source(parse_point_light(input_stream));
      }
      default:
        throw GrammarError(source_location, "definition of \"" + to_string(keyword) + "\" not allowed");
      }
    }
  }
};
