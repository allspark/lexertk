//
// Created by dboerm on 6/25/21.
//

#ifndef LEXERTK_DETAIL_HPP
#define LEXERTK_DETAIL_HPP

#include <cctype>

namespace lexertk
{
namespace details
{
inline bool is_whitespace(const char c) noexcept
{
  return (' ' == c) || ('\n' == c) ||
      ('\r' == c) || ('\t' == c) ||
      ('\b' == c) || ('\v' == c) ||
      ('\f' == c);
}

inline bool is_operator_char(const char c) noexcept
{
  return ('+' == c) || ('-' == c) ||
      ('*' == c) || ('/' == c) ||
      ('^' == c) || ('<' == c) ||
      ('>' == c) || ('=' == c) ||
      (',' == c) || ('!' == c) ||
      ('(' == c) || (')' == c) ||
      ('[' == c) || (']' == c) ||
      ('{' == c) || ('}' == c) ||
      ('%' == c) || (':' == c) ||
      ('?' == c) || ('&' == c) ||
      ('|' == c) || (';' == c);
}

inline bool is_string_delimiter(const char c) noexcept
{
  return c == '\'' || c == '\"';
}

inline bool is_letter(const char c) noexcept
{
  return (('a' <= c) && (c <= 'z')) || (('A' <= c) && (c <= 'Z'));
}

inline bool is_digit(const char c) noexcept
{
  return ('0' <= c) && (c <= '9');
}

inline bool is_letter_or_digit(const char c) noexcept
{
  return is_letter(c) || is_digit(c);
}

inline bool is_left_bracket(const char c) noexcept
{
  return ('(' == c) || ('[' == c) || ('{' == c);
}

inline bool is_right_bracket(const char c) noexcept
{
  return (')' == c) || (']' == c) || ('}' == c);
}

inline bool is_bracket(const char c) noexcept
{
  return is_left_bracket(c) || is_right_bracket(c);
}

inline bool is_sign(const char c) noexcept
{
  return ('+' == c) || ('-' == c);
}

inline bool is_invalid(const char c) noexcept
{
  return !is_whitespace(c) &&
      !is_operator_char(c) &&
      !is_letter(c) &&
      !is_digit(c) &&
      ('.' != c) &&
      ('_' != c) &&
      ('$' != c) &&
      ('~' != c) &&
      ('\'' != c);
}

inline bool imatch(const char c1, const char c2) noexcept
{
  return std::tolower(c1) == std::tolower(c2);
}

inline bool imatch(std::string_view s1, std::string_view s2) noexcept
{
  if (s1.size() == s2.size())
  {
    for (std::size_t i = 0; i < s1.size(); ++i)
    {
      if (std::tolower(s1[i]) != std::tolower(s2[i]))
      {
        return false;
      }
    }

    return true;
  }

  return false;
}

struct ilesscompare
{
  using is_transparent = void;

  inline bool operator()(std::string_view s1, std::string_view s2) const noexcept
  {
    const std::size_t length = std::min(s1.size(), s2.size());

    for (std::size_t i = 0; i < length; ++i)
    {
      if (std::tolower(s1[i]) > std::tolower(s2[i]))
        return false;
      else if (std::tolower(s1[i]) < std::tolower(s2[i]))
        return true;
    }

    return s1.size() < s2.size();
  }
};

inline std::string cleanup_escapes(std::string_view s)
{
  std::string ret;
  std::size_t removal_count{0};

  for (auto it = s.begin(); it != s.end();)
  {
    if (*it == '\\')
    {
      ++removal_count;
      if (++it == s.end())
      {
        break;
      }
      else if (*it != '\\')
      {
        switch (*it)
        {
          case 'n':
            ret.push_back('\n');
            ++it;
            break;
          case 'r':
            ret.push_back('\r');
            ++it;
            break;
          case 't':
            ret.push_back('\t');
            ++it;
            break;
        }
        continue;
      }
    }
    else
    {
      ret.push_back(*it++);
    }
  }

  return ret;
}
}  // namespace details
}  // namespace lexertk

#endif  //LEXERTK_DETAIL_HPP
