//
// Created by augus on 10/3/2025.
//

#include <stdbool.h>
#include "siew/scanner.h"

#include <ctype.h>
#include <string.h>

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner; // again... we should pass this in the functions as a pointer and should not be global

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

void initScanner(const char* source) {
    // just if we forget, this is a pointer to the start of the char string, not exactly
    // the entire string
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool isAtEnd() {
    return *scanner.current == '\0';
}

static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

// IMPORTANT the match function advances in the character scanner list
static bool match(char expected) {
    if (isAtEnd()) return false; // we evaluate if we are at the end first
    if (*scanner.current != expected) return false;

    // we advance
    scanner.current++;
    return true;
}

// sees the character but it does not consume the character
static char peek() {
    return *scanner.current;
}

// we can see just n + 1 character in this scaner
static char peekNext() {
    if (isAtEnd()) return '\0'; // if end we return null
    return scanner.current[1];
}

// a little scaner to handle spaces and new lines
static void skipWhitespaceAndComments() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                scanner.line++;
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    // A comment goes until the end of the line.
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static Token string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') scanner.line++;
        advance();
    }

    // the while loop above can end if we are in the end of the file
    // if that happens, pretty much we have an unterminated string
    if (isAtEnd()) return errorToken("Unterminated string.");

    // the closing quote.
    advance();

    // the value is going to be the lexeme that is created here when the token is created
    // clever!!
    return makeToken(TOKEN_STRING);
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static Token number() {
   while (isDigit(peek())) advance();

    if (peek() == '.' && isdigit(peekNext())) {
        // we need to consume the dot '.'
        advance();

        while (isDigit(peek())) advance();
    }

    // we do here similar to what we do to the strings
    return makeToken(TOKEN_NUMBER);
}

static bool isAlpha(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type){
   if (scanner.current - scanner.start == start + length &&
       memcmp(scanner.start + start, rest, length) == 0) {
      return type;
  }

  return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
   switch (scanner.start[0]) {
      case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
      case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
      case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
      case 'f':
        // we must verify that there are a second letter before continue
        // since 'f' is still a valid identifier after all
        if (scanner.current - scanner.start > 1) {
          switch (scanner.start[1]) {
              case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
              case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
              case 'n': return TOKEN_FUN;
          }
        }
        break;
      case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
      case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
      case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
      case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
      case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
      case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
      case 't':
           // same as f, we verify if there are a second letter before continue
        if (scanner.current - scanner.start > 1) {
          switch (scanner.start[1]) {
              case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
              case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
          }
        }
      break;
      case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
      case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  }
  return TOKEN_IDENTIFIER;
}

static Token identifier() {
    while (isAlpha(peek()) || isDigit(peek())) advance();
    return makeToken(identifierType());
}

Token scanToken() {
    skipWhitespaceAndComments();
    // we scan tokens on the fly
    // so every time we get here we are about to read a new token, we set the start to the current
    // token that we have in the current pointer.
    scanner.start = scanner.current;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    // this returns the current char to scan
    // and advance
    char c = advance();

    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

    switch (c) {
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-': return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_PLUS);
        case '/': return makeToken(TOKEN_SLASH);
        case '*': return makeToken(TOKEN_STAR);
        // two-character tokens
        case '!':
            return makeToken(
                match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(
                match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(
                match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(
                match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return string();
    }

    // todo literals

    return errorToken("Unexpected character.");
}
