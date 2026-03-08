#include <cassert>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <optional>
#include <variant>
#include <string>
#include <vector>

// let's plan out the parser to close out this session
// i am thinking of having like a stack of depth,
// like storing each list and level of recursion in a LIFO structure
// then when i reach the end of a list, like a closing ], i pop it from the
// stack, i am using shared_ptr's,
// when i see like [ I add a new std::vector to the stack
// i want to be lenient about commas
// goodbye

typedef enum {
  TOKEN_LBRAK,
  TOKEN_RBRAK,
  TOKEN_COLON,
  TOKEN_LBRAC,
  TOKEN_RBRAC,
  TOKEN_STRING,
  TOKEN_NUMBER,
  TOKEN_BOOL,
  TOKEN_NULL,
  TOKEN_COMMA
} TokenType;


bool is_num(char c) {
  return std::isdigit(c) || c == '.' || c == 'E' || c == '+' || c == '-' || c == 'e';
}

typedef std::variant<std::monostate, double, std::string, bool> TokenContents;

typedef struct {
  TokenType tt;
  TokenContents tc;
} Token;

void skip_whitespace(std::ifstream &f) {
  while (true)
  {
    char c = f.peek();
    if (c == std::char_traits<char>::eof()) {
      return;
    };
   switch (c) {
    case '\n':
    case ' ':
    case '\t':
    case '\r':
      f.get();
      break;
    default:
      return;
    }
  }
}


std::vector<Token> list_tokens(std::ifstream &f) {
  
  std::vector<Token> tokens = std::vector<Token>();
  while (true) {
    Token t;
    skip_whitespace(f);
    char c = f.get();
    if (c == std::char_traits<char>::eof()) {
      return tokens;
    }
    t.tc = std::monostate();
    bool found = true;
    switch (c) {
    case '{':
      t.tt = TOKEN_LBRAC;
      break;
    case '}':
      t.tt = TOKEN_RBRAC;
      break;
    case '[':
      t.tt = TOKEN_LBRAK;
      break;
    case ']':
      t.tt = TOKEN_RBRAK;
      break;
    case ':':
      t.tt = TOKEN_COLON;
      break;
    case ',':
      t.tt = TOKEN_COMMA;
      break;
    default:
      found = false;
    }
    if (found) {
      tokens.push_back(t);
      continue;
    }
    if (is_num(c)) {
      std::string full = "";
      full.push_back(c);
      while (is_num(c)) {
          char c = f.peek();
          if (is_num(c)) {
            full.push_back(c);
            f.get();
          } else {
            break;
          }
        }
        double val = std::stod(full);
        t.tc = val;
        t.tt = TOKEN_NUMBER;
        tokens.push_back(t);
        continue;
    }

    if (c == 't') {
      std::string true_str = "true";
      for (int _ = 0; _ < 4; _++) {
        assert(true_str[_] == c);
        if (_ != 3)
          {
            c = f.get();
          }
      }
      t.tc = true;
      t.tt = TOKEN_BOOL;
      tokens.push_back(t);
      continue;
    }
    if (c == 'f') {
      std::string false_str = "false";
      for (int _ = 0; _ < 5; _++) {
        assert(false_str[_] == c);
        if (_ != 4)
          {
            c = f.get();
          }
      }
      t.tc = false;
      t.tt = TOKEN_BOOL;
      tokens.push_back(t);
      continue;
    }
        if (c == 'n') {
      std::string null_str = "null";
      for (int _ = 0; _ < 4; _++) {
        assert(null_str[_] == c);
        if (_ != 3)
          {
            c = f.get();
          }
      }
      t.tc = false;

      t.tc = std::monostate();
      t.tt = TOKEN_NULL;
      tokens.push_back(t);
      continue;
    }
    if (c == '\"') {
      std::string parsed = "";
      while (true) {
        c = f.peek();
        if (c != '\\' && c != '\"') {
          f.get();
          parsed.push_back(c);
          continue;
        }
        if (c == '"') {
          f.get();
          break;
        }
        if (c == '\\') {

          char n = f.get();
          f.get();
          switch (n) {
	    
          case '\"':
            parsed.push_back('\"');
            break;
          case 'n':
            parsed.push_back('\n');
            break;
          case 't':
            parsed.push_back('\t');
            break;
          case 'b':
            parsed.push_back('\b');
            break;
          case 'r':
            parsed.push_back('\r');
            break;
          case 'f':
            parsed.push_back('\f');
            break;
          case '\\':
            parsed.push_back('\\');
            break;
          case '/':
            parsed.push_back('/');
          }
	}
      }
      t.tt = TOKEN_STRING;
      t.tc = parsed;
      tokens.push_back(t);
    }
  }
  return tokens;
}



std::ostream& operator<<(std::ostream& os, const Token& token) {
  switch (token.tt) {
  case TOKEN_BOOL:
    os << "BOOL: ";
    break;
  case TOKEN_COLON:
    os << "COLON";
    break;
  case TOKEN_COMMA:
    os << "COMMA";
    break;
  case TOKEN_LBRAC:
    os << "{";
    break;
  case TOKEN_RBRAC:
    os << "}";
    break;
  case TOKEN_LBRAK:
    os << "[";
    break;
  case TOKEN_RBRAK:
    os << "]";
    break;
  case TOKEN_NULL:
    os << "NULL";
    break;
  case TOKEN_NUMBER:
    os << "NUMBER: ";
    break;
  case TOKEN_STRING:
    os << "STRING: ";
    break;
  }
  if (!std::holds_alternative<std::monostate>(token.tc)) {
    if (token.tt == TOKEN_NUMBER) {
      os << std::get<double>(token.tc);
    }
    if (token.tt == TOKEN_STRING) {
      os << std::get<std::string>(token.tc);
    }
    if (token.tt == TOKEN_BOOL) {
      if (std::get<bool>(token.tc)) {
        os << "true";
      } else {
        os << "false";
      }
    }
  }
  return os;
}


int main(int argc, char** argv) {
  std::ifstream file = std::ifstream("number.json");
  std::vector<Token> tokens = list_tokens(file);
  for (const Token &tok : tokens) {
    std::cout << tok << std::endl;
  }
}
