//
// Created by dboerm on 6/25/21.
//

#ifndef LEXERTK_TOKEN_HPP
#define LEXERTK_TOKEN_HPP

#include <cstdint>
#include <limits>
#include <string_view>

namespace lexertk
{
class token
{
public:
  using iterator = std::string_view::const_iterator;

  enum struct token_type : unsigned char
  {
    none = 0,
    error = 1,
    err_symbol = 2,
    err_number = 3,
    err_string = 4,
    err_sfunc = 5,
    eof = 6,
    number = 7,
    symbol = 8,
    string = 9,
    string_with_escapes = 10,
    boolean = 11,
    shr = 12,
    shl = 13,
    lte = 14,
    ne = 15,
    gte = 16,
    eol = 17,
    rebind = 18,
    eq = 19,
    increment = 20,
    decrement = 21,
    scope = 22,
    logical_and = 23,
    logical_or = 24,
    eoe = ';',
    lt = '<',
    gt = '>',
    assign = '=',
    rbracket = ')',
    lbracket = '(',
    rsqrbracket = ']',
    lsqrbracket = '[',
    rcrlbracket = '}',
    lcrlbracket = '{',
    comma = ',',
    dot = '.',
    add = '+',
    sub = '-',
    div = '/',
    mul = '*',
    mod = '%',
    pow = '^',
    logical_not = '!',
    bit_and = '&',
    bit_or = '|',
    bit_not = '~',
    colon = ':',
    hash = '#'
  };

  struct Position
  {
    using value_type = std::uint16_t;

    value_type line{std::numeric_limits<value_type>::max()};
    value_type column{std::numeric_limits<value_type>::max()};

    inline Position IncrementColumn(iterator begin, iterator end) noexcept;
    inline void NextLine() noexcept;
    inline void NextColumn() noexcept;
  };

  token() = default;

  inline token(token_type tt, iterator begin, iterator end, Position position) noexcept;
  inline token(token_type tt, Position position) noexcept;
  inline token(token_type tt, std::string_view value, Position position) noexcept;

  inline bool is_error() const noexcept;

  inline token_type get_type() const noexcept;
  inline std::string_view get_value() const noexcept;
  inline Position get_position() const noexcept;

  inline void set_type(token_type) noexcept;
  inline void set_value(std::string_view) noexcept;

private:
  Position m_position{};
  token_type m_type{token_type::none};
  std::string_view m_value;
};

inline std::string_view to_string(token::token_type t) noexcept;
}  // namespace lexertk

#include "token.ipp"

#endif  //LEXERTK_TOKEN_HPP
