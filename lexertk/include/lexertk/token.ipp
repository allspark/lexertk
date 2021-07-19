//
// Created by allspark on 17/07/2021.
//

#ifndef LEXERTK_TOKEN_IPP
#define LEXERTK_TOKEN_IPP

namespace lexertk
{
token::Position token::Position::IncrementColumn(iterator begin, iterator end) noexcept
{
  std::size_t off = std::distance(begin, end);
  Position tmp{*this};

  column += off;

  return tmp;
}

void token::Position::NextLine() noexcept
{
  ++line;
  column = 1;
}

void token::Position::NextColumn() noexcept
{
  ++column;
}

token::token(token_type tt, iterator begin, iterator end, Position position) noexcept
  : m_type{tt}
  , m_value{begin, static_cast<std::size_t>(std::distance(begin, end))}
  , m_position{position}
{
}

token::token(token_type tt, Position position) noexcept
  : m_type{tt}
  , m_position{position}
{
}

token::token(token_type tt, std::string_view value, Position position) noexcept
  : m_type{tt}
  , m_value{value}
  , m_position{position}
{
}

bool token::is_error() const noexcept
{
  return m_type == token_type::error || m_type == token_type::err_symbol || m_type == token_type::err_number || m_type == token_type::err_string;
}

token::token_type token::get_type() const noexcept
{
  return m_type;
}

std::string_view token::get_value() const noexcept
{
  return m_value;
}

token::Position token::get_position() const noexcept
{
  return m_position;
}

void token::set_type(token_type t) noexcept
{
  m_type = t;
}

void token::set_value(std::string_view value) noexcept
{
  m_value = value;
}

std::string_view to_string(token::token_type t) noexcept
{
  switch (t)
  {
    case token::token_type::none:
      return "NONE";
    case token::token_type::error:
      return "ERROR";
    case token::token_type::err_symbol:
      return "ERROR_SYMBOL";
    case token::token_type::err_number:
      return "ERROR_NUMBER";
    case token::token_type::err_string:
      return "ERROR_STRING";
    case token::token_type::eof:
      return "EOF";
    case token::token_type::number:
      return "NUMBER";
    case token::token_type::symbol:
      return "SYMBOL";
    case token::token_type::string:
      return "STRING";
    case token::token_type::assign:
      return "=";
    case token::token_type::shr:
      return ">>";
    case token::token_type::shl:
      return "<<";
    case token::token_type::lte:
      return "<=";
    case token::token_type::ne:
      return "!=";
    case token::token_type::gte:
      return ">=";
    case token::token_type::eoe:
      return "EOE";
    case token::token_type::lt:
      return "<";
    case token::token_type::gt:
      return ">";
    case token::token_type::eq:
      return "==";
    case token::token_type::rbracket:
      return ")";
    case token::token_type::lbracket:
      return "(";
    case token::token_type::rsqrbracket:
      return "]";
    case token::token_type::lsqrbracket:
      return "[";
    case token::token_type::rcrlbracket:
      return "}";
    case token::token_type::lcrlbracket:
      return "{";
    case token::token_type::comma:
      return ",";
    case token::token_type::add:
      return "+";
    case token::token_type::sub:
      return "-";
    case token::token_type::div:
      return "/";
    case token::token_type::mul:
      return "*";
    case token::token_type::mod:
      return "%";
    case token::token_type::pow:
      return "^";
    case token::token_type::colon:
      return ":";
    case token::token_type::err_sfunc:
      return "ERROR_SFUNC";
    case token::token_type::rebind:
      return ":=";
    case token::token_type::string_with_escapes:
      return "STRING2";
  }
  return "UNKNOWN";
}
}  // namespace lexertk

#endif  //LEXERTK_TOKEN_IPP
