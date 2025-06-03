// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// ------------ LIBRARY FOR COMPILERS -----------------
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#include "materials.hpp"
#include "shapes.hpp"

#include <unordered_map> //C++ library for unordered maps (analogous to Python dictionaries) much faster than std::map

// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ----------------------------------------------------------------------------------------

enum class KeywordEnum;

//----------------------------------------------------------------------------------------------------
//------------------- TOKEN CLASS (tagged union) and AUXILIARY SUBCLASSES -------------------------
//----------------------------------------------------------------------------------------------------

//------------SOURCE LOCATION: contains file name, line number and column number of the token----
struct SourceLocation {
  //------- Properties --------
  std::string file; // File name (or empty string if not applicable e.g. source code was provided as a memory stream, or through a
                    // network connection)
  int line;         // Line number (starting from 1)
  int column;       // Column number (starting from 1)

  //----------- Constructors -----------
  // Default constructor initializes to empty file and zero line/column
  SourceLocation(const std::string &file = "", int line = 0, int column = 0) : file(file), line(line), column(column) {}

  //----------- Methods -----------
  ///@brief convert source location to string (for debugging)
  std::string to_string() const {
    return "File: " + file + ", Line: " + std::to_string(line) + ", Column: " + std::to_string(column);
  }
};

//------------ TOKEN TYPE (used for tagged union pattern in C++)---------------
enum class TokenType {
  // NOTE these are the 6 types of token implemented in Pytracer
  STOP_TOKEN,     // An (optional) token signalling the end of a file
  KEYWORD,        // A token containing a keyword
  SYMBOL,         // A token containing a symbol
  IDENTIFIER,     // A token containing an identifier (i.e. a variable name)
  LITERAL_STRING, // A token containing a literal string
  LITERAL_NUMBER, // A token containing a literal number (i.e. a float)
};

//------------ TOKEN VALUE: union struct for possible token values ---------------
union TokenValue {
  bool stop_token;        // Used for STOP_TOKEN type //QUESTION not sure this is the right/best option for stop_token
  KeywordEnum keyword;    // Used for KEYWORD type
  char symbol;            // Used for SYMBOL type
  std::string identifier; // Used for IDENTIFIER type
  std::string string;     // Used for LITERAL_STRING type
  float number;           // Used for LITERAL_NUMBER type

  // The default constructor and destructor are *mandatory* for unions structs to be used in the tagged union class Token below
  TokenValue() {}  // no default field initialized
  ~TokenValue() {} // intentionally empty — `Token' class will handl cleanup
};

//------ TOKEN CLASS: product of `SourceLocation', `TokenType` and `TokenValue` (tagged union pattern in C++) ------
#include <new> // Needed for placement new

//------ TOKEN CLASS: product of `SourceLocation', `TokenType` and `TokenValue` (tagged union pattern in C++) ------
class Token {
public:
  //------- Properties --------
  SourceLocation source_location; // The source location
  TokenType type;                 // The "tag"
  TokenValue value;               // The "union"

  //----------- Constructors -----------
  Token(const SourceLocation &loc, TokenType t) : source_location(loc), type(t) {
    switch (type) {
    case TokenType::STOP_TOKEN:
      value.stop_token = false;
      break;
    case TokenType::KEYWORD:
      value.keyword = KeywordEnum::NONE;
      break;
    case TokenType::SYMBOL:
      value.symbol = '\0';
      break;
    case TokenType::IDENTIFIER:
      new (&value.identifier) std::string("");
      break;
    case TokenType::LITERAL_STRING:
      new (&value.string) std::string("");
      break;
    case TokenType::LITERAL_NUMBER:
      value.number = 0.0f;
      break;
    }
  }

  //----------- Destructor -----------
  ~Token() { destroy_value(); }

  ///@brief destroy the active union field if needed
  void destroy_value() {
    switch (type) {
    case TokenType::IDENTIFIER:
      value.identifier.~basic_string();
      break;
    case TokenType::LITERAL_STRING:
      value.string.~basic_string();
      break;
    default:
      break;
    }
  }

  //----------- Methods -----------

  ///@brief assign a numeric token value
  void assign_number(float val) {
    destroy_value();
    type = TokenType::LITERAL_NUMBER;
    value.number = val;
  }

  ///@brief assign a string token value
  void assign_string(const std::string &s) {
    destroy_value();
    type = TokenType::LITERAL_STRING;
    new (&value.string) std::string(s);
  }

  ///@brief assign a stop token (usually signals EOF)
  void assign_stop_token(bool b = false) {
    destroy_value();
    type = TokenType::STOP_TOKEN;
    value.stop_token = b;
  }

  ///@brief assign a keyword token
  void assign_keyword(KeywordEnum kw) {
    destroy_value();
    type = TokenType::KEYWORD;
    value.keyword = kw;
  }

  ///@brief assign a single character symbol token
  void assign_symbol(char c) {
    destroy_value();
    type = TokenType::SYMBOL;
    value.symbol = c;
  }

  ///@brief assign an identifier token (e.g. variable name)
  void assign_identifier(const std::string &name) {
    destroy_value();
    type = TokenType::IDENTIFIER;
    new (&value.identifier) std::string(name);
  }

  ///@brief convert a TokenType enum to a string (for printing/debugging)
  static std::string to_string(TokenType t) {
    switch (t) {
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
    default:
      return "UNKNOWN";
    }
  }

  ///@brief print token information to the console
  void print() const {
    std::cout << "Token Type: " << to_string(type) << ", Value: ";
    switch (type) {
    case TokenType::STOP_TOKEN:
      std::cout << (value.stop_token ? "true" : "false");
      break;
    case TokenType::KEYWORD:
      std::cout << ::to_string(value.keyword) << " (KeywordEnum: " << static_cast<int>(value.keyword) << ")";
      break;
    case TokenType::SYMBOL:
      std::cout << value.symbol;
      break;
    case TokenType::IDENTIFIER:
      std::cout << value.identifier;
      break;
    case TokenType::LITERAL_STRING:
      std::cout << value.string;
      break;
    case TokenType::LITERAL_NUMBER:
      std::cout << value.number;
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
