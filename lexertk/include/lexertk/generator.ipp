//
// Created by allspark on 17/07/2021.
//

#ifndef LEXERTK_GENERATOR_IPP
#define LEXERTK_GENERATOR_IPP

#include <fmt/core.h>

#include <tuple>

namespace lexertk
{
generator::Range::operator bool() const noexcept
{
  return begin != end;
}

generator::Range& generator::Range::operator++() noexcept
{
  ++begin;
  return *this;
}

generator::Range& generator::Range::operator+=(std::size_t off) noexcept
{
  begin += off;
  return *this;
}

bool generator::process(std::string_view line)
{
  m_currentPosition.NextLine();
  Range range = {line.begin(), line.end()};

  while (range)
  {
    range = scan_token(range);

    if (!m_token_list.empty() && m_token_list.back().is_error())
    {
      return false;
    }
  }

  return true;
}

bool generator::empty() const noexcept
{
  return m_token_list.empty();
}

std::size_t generator::size() const noexcept
{
  return m_token_list.size();
}

token const& generator::operator[](const std::size_t& index) const
{
  if (index < m_token_list.size())
    return m_token_list[index];
  else
    return m_eof_token;
}

generator::token_list_itr_t generator::begin() const noexcept
{
  return m_token_list.begin();
}

generator::token_list_itr_t generator::end() const noexcept
{
  return m_token_list.end();
}

generator::token_list_t const& generator::get_token_list() const & noexcept
{
  return m_token_list;
}

generator::token_list_t generator::get_token_list() && noexcept
{
  return std::move(m_token_list);
}

generator::Range generator::skip_whitespace(Range range)
{
  while (range && details::is_whitespace(*range.begin))
  {
    if (*range.begin == '\n')
    {
      m_currentPosition.NextLine();
    }
    else
    {
      m_currentPosition.NextColumn();
    }
    ++range;
  }
  return range;
}

namespace
{
enum struct CommentMode
{
  NONE,
  COMMENT_START,
  STAR
};
enum struct CommentIncrement
{
  NONE = 0,
  ONE = 1,
  TWO = 2
};
std::tuple<CommentMode, CommentIncrement> comment_start(const char c0, const char c1)
{
  if ('#' == c0)
  {
    return {CommentMode::COMMENT_START, CommentIncrement::ONE};
  }
  else if ('/' == c0)
  {
    if ('/' == c1)
    {
      return {CommentMode::COMMENT_START, CommentIncrement::TWO};
    }
    else if ('*' == c1)
    {
      return {CommentMode::STAR, CommentIncrement::TWO};
    }
  }
  return {CommentMode::NONE, CommentIncrement::NONE};
}

bool comment_end(const char c0, const char c1, CommentMode mode)
{
  return ((mode == CommentMode::COMMENT_START) && ('\n' == c0)) || ((mode == CommentMode::STAR) && ('*' == c0) && ('/' == c1));
}
}  // namespace

generator::Range generator::skip_comments(Range range)
{
  //The following comment styles are supported:
  // 1. // .... \n
  // 2. #  .... \n
  // 3. /* .... */

  if (!range || range.begin + 1 == range.end)
    return range;

  auto [mode, increment] = comment_start(*range.begin, *(range.begin + 1));
  if (mode == CommentMode::NONE)
  {
    return range;
  }

  range.begin += static_cast<std::size_t>(increment);

  while (range && !comment_end(*range.begin, *(range.begin + 1), mode))
  {
    ++range;
  }

  if (range)
  {
    range += static_cast<std::size_t>(mode);
    range = skip_whitespace(range);
    range = skip_comments(range);
  }

  return range;
}

generator::Range generator::scan_token(Range range)
{
  range = skip_whitespace(range);
  range = skip_comments(range);

  if (!range)
  {
    return range;
  }
  else if (details::is_operator_char(*range.begin))
  {
    return scan_operator(range);
  }
  else if (details::is_letter(*range.begin))
  {
    return scan_symbol(range);
  }
  else if (details::is_digit(*range.begin) || ('.' == *range.begin))
  {
    return scan_number(range);
  }
  else if ('\'' == *range.begin)
  {
    return scan_string(range);
  }
  else
  {
    auto end = range.begin + 2;
    m_token_list.emplace_back(token::token_type::error, range.begin, end, m_currentPosition.IncrementColumn(range.begin, end));
    ++range;
  }
  return range;
}

generator::Range generator::scan_operator(Range range)
{
  if (range.begin + 1 != range.end)
  {
    token::token_type ttype = [](const unsigned char c0, const unsigned char c1)
    {
      if ((c0 == '<') && (c1 == '='))
        return token::token_type::lte;
      if ((c0 == '>') && (c1 == '='))
        return token::token_type::gte;
      if ((c0 == '<') && (c1 == '>'))
        return token::token_type::ne;
      if ((c0 == '!') && (c1 == '='))
        return token::token_type::ne;
      if ((c0 == '=') && (c1 == '='))
        return token::token_type::eq;
      if ((c0 == ':') && (c1 == '='))
        return token::token_type::assign;
      if ((c0 == '<') && (c1 == '<'))
        return token::token_type::shl;
      if ((c0 == '>') && (c1 == '>'))
        return token::token_type::shr;

      return token::token_type::none;
    }(*range.begin, *(range.begin + 1));

    if (token::token_type::none != ttype)
    {
      auto end = range.begin + 2;
      m_token_list.emplace_back(ttype, range.begin, end, m_currentPosition.IncrementColumn(range.begin, end));
      range += 2;
      return range;
    }
  }

  auto end = range.begin + 1;
  auto pos = m_currentPosition.IncrementColumn(range.begin, end);

  if ('<' == *range.begin)
    m_token_list.emplace_back(token::token_type::lt, range.begin, end, pos);
  else if ('>' == *range.begin)
    m_token_list.emplace_back(token::token_type::gt, range.begin, end, pos);
  else if (';' == *range.begin)
    m_token_list.emplace_back(token::token_type::eoe, range.begin, end, pos);
  else if ('&' == *range.begin)
    m_token_list.emplace_back(token::token_type::symbol, range.begin, end, pos);
  else if ('|' == *range.begin)
    m_token_list.emplace_back(token::token_type::symbol, range.begin, end, pos);
  else
    m_token_list.emplace_back(static_cast<token::token_type>(*range.begin), range.begin, end, pos);

  return ++range;
}

generator::Range generator::scan_symbol(Range range)
{
  auto begin = range.begin;
  while (range && (details::is_letter_or_digit(*range.begin) || ((*range.begin) == '_')))
  {
    ++range;
  }
  m_token_list.emplace_back(token::token_type::symbol, begin, range.begin, m_currentPosition.IncrementColumn(begin, range.begin));

  return range;
}

generator::Range generator::scan_number(Range range)
{
  /*
       Attempt to match a valid numeric value in one of the following formats:
       01. 123456
       02. 123.456
       03. 123.456e3
       04. 123.456E3
       05. 123.456e+3
       06. 123.456E+3
       07. 123.456e-3
       08. 123.456E-3
       09. .1234
       10. .1234e3
       11. .1234E+3
       12. .1234e+3
       13. .1234E-3
       14. .1234e-3
    */
  auto begin = range.begin;
  bool dot_found = false;
  bool e_found = false;
  bool post_e_sign_found = false;
  bool post_e_digit_found = false;

  while (range)
  {
    if ('.' == *range.begin)
    {
      if (dot_found)
      {
        m_token_list.emplace_back(token::token_type::err_number, begin, range.begin, m_currentPosition.IncrementColumn(begin, range.begin));
        return range;
      }

      dot_found = true;
      ++range;
      continue;
    }
    else if (details::imatch('e', (*range.begin)))
    {
      if (range.begin + 1 == range.end)
      {
        m_token_list.emplace_back(token::token_type::err_number, begin, range.begin, m_currentPosition.IncrementColumn(begin, range.begin));
        return range;
      }
      else if (auto c = *(range.begin + 1); ('+' != c) && ('-' != c) && !details::is_digit(c))
      {
        m_token_list.emplace_back(token::token_type::err_number, begin, range.begin, m_currentPosition.IncrementColumn(begin, range.begin));
        return range;
      }

      e_found = true;
      ++range;

      continue;
    }
    else if (e_found && details::is_sign(*range.begin) && !post_e_digit_found)
    {
      if (post_e_sign_found)
      {
        m_token_list.emplace_back(token::token_type::err_number, begin, range.begin, m_currentPosition.IncrementColumn(begin, range.begin));
        return range;
      }

      post_e_sign_found = true;
      ++range;

      continue;
    }
    else if (e_found && details::is_digit(*range.begin))
    {
      post_e_digit_found = true;
      ++range;

      continue;
    }
    else if (('.' != (*range.begin)) && !details::is_digit(*range.begin))
      break;
    else
      ++range;
  }

  m_token_list.emplace_back(token::token_type::number, begin, range.begin, m_currentPosition.IncrementColumn(begin, range.begin));
  return range;
}

generator::Range generator::scan_string(Range range)
{
  auto begin = range.begin + 1;
  m_currentPosition.NextColumn();
  if (std::distance(range.begin, range.end) < 2)
  {
    m_token_list.emplace_back(token::token_type::err_string, range.begin, range.end, m_currentPosition.IncrementColumn(range.begin, range.end));
    return range;
  }

  ++range;

  bool escaped_found = false;
  bool escaped = false;

  while (range)
  {
    if (!escaped && ('\\' == *range.begin))
    {
      escaped_found = true;
      escaped = true;
      ++range;
      continue;
    }
    else if (!escaped)
    {
      if ('\'' == *range.begin)
        break;
    }
    else if (escaped)
      escaped = false;

    ++range;
  }

  if (!range)
  {
    m_token_list.emplace_back(token::token_type::err_string, begin, range.begin, m_currentPosition.IncrementColumn(begin, range.begin));
    return range;
  }

  //      if (!escaped_found)
  //      t.set_string(begin, m_s_itr, m_base_itr);
  //      else
  //      {
  //        std::string parsed_string(begin, s_itr_);
  //        details::cleanup_escapes(parsed_string);
  //        t.set_string(parsed_string, std::distance(base_itr_, begin));
  //      }

  m_token_list.emplace_back(token::token_type::string, begin, range.begin, m_currentPosition.IncrementColumn(begin, range.begin));
  ++range;
  m_currentPosition.NextColumn();

  return range;
}

void dump(generator::token_list_t const& list)
{
  for (std::size_t i = 0; i < list.size(); ++i)
  {
    const auto& t = list[i];
    fmt::print("Token[{:2}] @ ({:3},{:3}) {:6} -> {}\n", i, t.get_position().line, t.get_position().column, to_string(t.get_type()), t.get_value());
  }
}
}  // namespace lexertk

#endif  //LEXERTK_GENERATOR_IPP
