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
inline bool is_whitespace(const char c)
{
  return (' ' == c) || ('\n' == c) ||
      ('\r' == c) || ('\t' == c) ||
      ('\b' == c) || ('\v' == c) ||
      ('\f' == c);
}

inline bool is_operator_char(const char c)
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

inline bool is_letter(const char c)
{
  return (('a' <= c) && (c <= 'z')) || (('A' <= c) && (c <= 'Z'));
}

inline bool is_digit(const char c)
{
  return ('0' <= c) && (c <= '9');
}

inline bool is_letter_or_digit(const char c)
{
  return is_letter(c) || is_digit(c);
}

inline bool is_left_bracket(const char c)
{
  return ('(' == c) || ('[' == c) || ('{' == c);
}

inline bool is_right_bracket(const char c)
{
  return (')' == c) || (']' == c) || ('}' == c);
}

inline bool is_bracket(const char c)
{
  return is_left_bracket(c) || is_right_bracket(c);
}

inline bool is_sign(const char c)
{
  return ('+' == c) || ('-' == c);
}

inline bool is_invalid(const char c)
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

inline bool imatch(const char c1, const char c2)
{
  return std::tolower(c1) == std::tolower(c2);
}

inline bool imatch(std::string_view s1, std::string_view s2)
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

  inline bool operator()(std::string_view s1, std::string_view s2) const
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

inline void cleanup_escapes(std::string& s)
{
  typedef std::string::iterator str_itr_t;

  str_itr_t itr1 = s.begin();
  str_itr_t itr2 = s.begin();
  str_itr_t end = s.end();

  std::size_t removal_count = 0;

  while (end != itr1)
  {
    if ('\\' == (*itr1))
    {
      ++removal_count;

      if (end == ++itr1)
        break;
      else if ('\\' != (*itr1))
      {
        switch (*itr1)
        {
          case 'n':
            (*itr1) = '\n';
            break;
          case 'r':
            (*itr1) = '\r';
            break;
          case 't':
            (*itr1) = '\t';
            break;
        }

        continue;
      }
    }

    if (itr1 != itr2)
    {
      (*itr2) = (*itr1);
    }

    ++itr1;
    ++itr2;
  }

  s.resize(s.size() - removal_count);
}
}  // namespace details
}  // namespace lexertk

#endif  //LEXERTK_DETAIL_HPP
