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
#include <variant>       // C++17 library for type-safe tagged union replacement (used for TokenValue)

// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ----------------------------------------------------------------------------------------

enum class KeywordEnum;

//----------------------------------------------------------------------------------------------------
//------------------- TOKEN CLASS (using std::variant) and AUXILIARY SUBCLASSES -------------------------
//----------------------------------------------------------------------------------------------------

//------------SOURCE LOCATION: contains file name, line number and column number of the token----
struct SourceLocation {
  //------- Properties --------
  std::string file; // File name (or empty string if not applicable e.g. source code was provided as a memory stream, or through a
                    // network connection)
  int line;         // Line number (starting from 1)
  int column;       // Column number (starting from 1)

  //----------- Constructors -----------
  // Default constructor initializes to empty file and ZERO line/column
  SourceLocation(const std::string &file = "", int line = 0, int column = 0) : file(file), line(line), column(column) {}

  //----------- Methods -----------
  ///@brief convert source location to string (for debugging)
  std::string to_string() const {
    return "File: " + file + ", Line: " + std::to_string(line) + ", Column: " + std::to_string(column);
  }
};

//------------ TOKEN TYPE (used for dispatching at runtime)---------------
enum class TokenType {
  // these are the 6 types of token implemented in Pytracer
  STOP_TOKEN,     // An (optional) token signalling the end of a file
  KEYWORD,        // A token containing a keyword
  SYMBOL,         // A token containing a symbol
  IDENTIFIER,     // A token containing an identifier (i.e. a variable name)
  LITERAL_STRING, // A token containing a literal string
  LITERAL_NUMBER  // A token containing a literal number (i.e. a float)
};

//------------ TOKEN VALUE: variant for possible token values ---------------
using TokenValue = std::variant<std::monostate, bool, KeywordEnum, char, std::string, float>;

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
      value = false; // default stop_token = false
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
      value = 0.0f;
      break;
    }
  }

  //----------- Methods -----------

  ///@brief assign a stop token (signals EndOfFile)
  void assign_stop_token(bool b = false) {
    type = TokenType::STOP_TOKEN;
    value = b;
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

  ///@brief print token information to the console
  void print() const {
    std::cout << "Token Type: " << static_cast<int>(type) << ", Value: ";
    switch (type) {
    case TokenType::STOP_TOKEN:
      std::cout << (std::get<bool>(value) ? "true" : "false");
      break;
    case TokenType::KEYWORD:
      std::cout << to_string(std::get<KeywordEnum>(value)) << " (KeywordEnum: " << static_cast<int>(std::get<KeywordEnum>(value))
                << ")";
      break;
    case TokenType::SYMBOL:
      std::cout << std::get<char>(value);
      break;
    case TokenType::IDENTIFIER:
      std::cout << std::get<std::string>(value);
      break;
    case TokenType::LITERAL_STRING:
      std::cout << std::get<std::string>(value);
      break;
    case TokenType::LITERAL_NUMBER:
      std::cout << std::get<float>(value);
      break;
    default:
      std::cout << "Unknown token type";
      break;
    }
    std::cout << ", Source Location: " << source_location.to_string() << std::endl;
  }
};

//----------------------------------------------------------------------------------------------------
//------------------- KEYWORDS and related HELPERS -------------------------
//----------------------------------------------------------------------------------------------------

/// @brief Enum class representing the keywords used in the scene files
// NOTE here I am just using those from Pytracer, modify as needed and modify helpers below accordingly
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
//------------------- ERROR CLASS FOR GRAMMAR/ LEXER / PARSER ERRORS -------------------------
//----------------------------------------------------------------------------------------------------

/// @brief An error found by the lexer/parser while reading a scene file
///
/// The fields of this class are:
/// - `location`: the source location where the error was discovered
/// - `message`: a user-friendly error message
class GrammarError : public std::exception {
  //----------- Properties -----------
  SourceLocation location;  // Location in the source file
  std::string message;      // Human-readable error message (explanation)
  std::string full_message; // Cached full message (location and explanation) for what()

public:
  //----------- Constructors -----------
  GrammarError(const SourceLocation &loc, const std::string &msg) : location(loc), message(msg) {
    full_message = "GrammarError at " + location.to_string() + ": " + message;
  }

  //----------- Methods -----------
  ///@brief Retrieve the error message as a C-style string (for use with std::exception)
  const char *what() const noexcept override { return full_message.c_str(); }

  ///@brief Access the original SourceLocation of the error
  const SourceLocation &get_location() const { return location; }

  ///@brief Access the user message
  const std::string &get_message() const { return message; }
};

// ------------------------------------------------------------------------------------------------------------
// -----------INPUT STREAM CLASS: High-level wrapper around std::istream for scene file parsing-------------
// ------------------------------------------------------------------------------------------------------------

class InputStream {
public:
  //----------- Properties -----------
  std::istream &stream;                         // underlying input stream
  SourceLocation location;                      // current location in the file (filename, line, column)
  std::optional<char> saved_char;               // Character "pushback" buffer (std::optional since possibly empty)
  std::optional<SourceLocation> saved_location; // SourceLocation corresponding to the saved_char (again std::optional)
  int tabulations;                              // Number of spaces a tab '\t' is worth (usually 4 or 8 spaces, default set to 8)
  std::shared_ptr<Token> saved_token;           // Token "pushback" buffer (nullptr if unused)

  //----------- Constructor -----------
  /// @brief deafult constructor for InputStream (does not take ownership of the stream)
  InputStream(std::istream &stream_, const std::string &file_name = "", int tabulations_ = 8)
      : stream(stream_), location(file_name, 1, 1), // Start at line 1, column 1
        saved_char(std::nullopt), saved_location(std::nullopt), tabulations(tabulations_), saved_token(nullptr) {}

  //------------------------------Methods------------------------------------

  //----------------------Helper methods for main function read_token() below ----------------------

  // --- UPDATE POSITION AFTER READING A CHARACTER --------------------------------------------------------------

  /// @brief Update the SourceLocation after reading character ch
  void update_pos(char ch) {
    if (ch == 0 || ch == EOF) {
      // Do nothing if there is no character
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

    if (saved_char.has_value()) { // If a character was pushed back, use it first
      ch = saved_char.value();
      saved_char = std::nullopt;
      // Restore saved location
      if (saved_location.has_value()) {
        location = saved_location.value();
        saved_location = std::nullopt;
      }
    } else {
      ch = static_cast<char>(stream.get());
      if (!stream) {
        ch = 0; // End of stream
      }
    }

    saved_location = location; // Save current location so we can restore it in unread_char
    update_pos(ch);            // Update the source location based on the character read
    return ch;
  }

  // --- UNREAD (PUSH BACK) A CHARACTER INTO THE STREAM ---------------------------------------------------------

  /// @brief Push back a single character into the input stream (for lookahead)
  void unread_char(char ch) {
    assert(!saved_char.has_value()); // Only one char of pushback at a time is supported, this should never happen!
    saved_char = ch;
    saved_location = location;
  }

  // --- SKIP WHITESPACES AND COMMENTS (we start comments with # ... until end of line) -------------------------

  /// @brief Skip over whitespace characters and comments (starting with '#') in the input stream
  void skip_whitespaces_and_comments() {
    char ch = read_char();
    while (is_whitespace(ch) || ch == '#') {
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
  static bool is_whitespace(char ch) {
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

    // try converting the string to a float and creating the token
    try {
      float value = std::stof(token_str); // Convert the string to a float
      // Create a token with the parsed number and the appropriate constructor/methods
      Token token(token_location, TokenType::LITERAL_NUMBER);
      token.assign_number(value);
      return token;
    } catch (...) { // If conversion fails, throw a GrammarError specific for floating-point numbers
      throw GrammarError(token_location, "'" + token_str + "' is an invalid floating-point number");
    }
  }

  // --- PARSE A KEYWORD OR IDENTIFIER --------------------------------------------------------------------------

  /// @brief Parse a keyword or identifier token from the input stream
  Token parse_keyword_or_identifier_token(char first_char, const SourceLocation &token_location) {
    std::string token_str(1, first_char);

    // Read characters until we reach a non-alphanumeric character
    while (true) {
      char ch = read_char();

      if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') { // Accept alphanumeric and '_' for CamelCase identifiers
        // use std::isalnum built-in function for alphanumeric check
        unread_char(ch);
        break;
      }
      token_str += ch; // Append character to the alphanumeric string and keep looping
    }

    // Check if it is a keyword
    auto kw_it = KEYWORDS.find(token_str);
    if (kw_it != KEYWORDS.end()) { // if it is a keyword, create a token with the appropriate constructor/methods
      Token token(token_location, TokenType::KEYWORD);
      token.assign_keyword(kw_it->second); // kw_it->second is the KeywordEnum value
      return token;
    } else { // If it is not a keyword, it must be an identifier
      // Create again the appropirate token
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
    if (saved_token) {             // Use saved token if available
      Token result = *saved_token; // Dereference the shared pointer to get the actual Token
      saved_token = nullptr;
      return result;
    }

    skip_whitespaces_and_comments(); // get rid of whitespace and comments, then start reading the next token

    char ch = read_char(); // read the first character

    if (ch == 0) { // You got to the end of file: create and return a STOP_TOKEN
      Token token(location, TokenType::STOP_TOKEN);
      token.assign_stop_token(false);
      return token;
    }

    // Save current location for token start
    SourceLocation token_location = location;

    // Handle single-character symbols or otherwise start parsing other token types
    const std::string SYMBOLS = "(){},[]:;=";         // NOTE I am taking these as in Pytracer, adjust as needed
    if (SYMBOLS.find(ch) != std::string::npos) {      // std::string::npos means ch was not found in SYMBOLS string
      Token token(token_location, TokenType::SYMBOL); // Create a symbol token
      token.assign_symbol(ch);
      return token;
    } else if (ch == '"') { // If it starts with a double quote, parse a string token
      return parse_string_token(token_location);
    } else if (std::isdigit(ch) || ch == '+' || ch == '-' || ch == '.') {
      // If it starts with a digit or a sign, parse a float token
      return parse_float_token(ch, token_location);
    } else if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
      // If it starts with an alphabetic character or '_', parse a keyword or identifier token
      return parse_keyword_or_identifier_token(ch, token_location);
    } else {
      // If it is an invalid character (all valid cases are ruled out already), throw a GrammarError
      throw GrammarError(location, std::string("Invalid character: '") + ch + "'");
    }
  }

  // --- UNREAD (PUSH BACK) A TOKEN -----------------------------------------------------------------------------

  /// @brief Push back a token into the input stream (for lookahead)
  void unread_token(const Token &token) {
    assert(!saved_token); // Only one token of pushback at a time is supported, this should never happen
    saved_token = std::make_shared<Token>(token);
  }
};
