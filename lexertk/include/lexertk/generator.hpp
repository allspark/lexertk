//
// Created by dboerm on 6/25/21.
//

#ifndef LEXERTK_GENERATOR_HPP
#define LEXERTK_GENERATOR_HPP

#include "token.hpp"
#include "detail.hpp"

#include <deque>

namespace lexertk
{
class generator
{
public:

    typedef token token_t;
    typedef std::deque<token_t> token_list_t;
    typedef std::deque<token_t>::iterator token_list_itr_t;

    generator()
      : base_itr_(0),
      s_itr_(0),
      s_end_(0)
    {
      clear();
    }

    inline void clear()
    {
      base_itr_ = 0;
      s_itr_    = 0;
      s_end_    = 0;
      token_list_.clear();
      token_itr_ = token_list_.end();
      store_token_itr_ = token_list_.end();
    }

    inline bool process(const std::string& str)
    {
      base_itr_ = str.data();
      s_itr_    = str.data();
      s_end_    = str.data() + str.size();

      eof_token_.set_operator(token_t::e_eof,s_end_,s_end_,base_itr_);
      token_list_.clear();

      while (!is_end(s_itr_))
      {
        scan_token();

        if (token_list_.empty())
          return true;
        else if (token_list_.back().is_error())
        {
          return false;
        }
      }
      return true;
    }

    inline bool empty() const
    {
      return token_list_.empty();
    }

    inline std::size_t size() const
    {
      return token_list_.size();
    }

    inline void begin()
    {
      token_itr_ = token_list_.begin();
      store_token_itr_ = token_list_.begin();
    }

    inline void store()
    {
      store_token_itr_ = token_itr_;
    }

    inline void restore()
    {
      token_itr_ = store_token_itr_;
    }

    inline token_t& next_token()
    {
      if (token_list_.end() != token_itr_)
      {
        return *token_itr_++;
      }
      else
        return eof_token_;
    }

    inline token_t& peek_next_token()
    {
      if (token_list_.end() != token_itr_)
      {
        return *token_itr_;
      }
      else
        return eof_token_;
    }

    inline token_t& operator[](const std::size_t& index)
    {
      if (index < token_list_.size())
        return token_list_[index];
      else
        return eof_token_;
    }

    inline token_t operator[](const std::size_t& index) const
    {
      if (index < token_list_.size())
        return token_list_[index];
      else
        return eof_token_;
    }

    inline bool finished() const
    {
      return (token_list_.end() == token_itr_);
    }

    inline std::string remaining() const
    {
      if (finished())
        return "";
      else if (token_list_.begin() != token_itr_)
        return std::string(base_itr_ + (token_itr_ - 1)->position,s_end_);
      else
        return std::string(base_itr_ + token_itr_->position,s_end_);
    }

private:

    inline bool is_end(const char* itr)
    {
      return (s_end_ == itr);
    }

    inline void skip_whitespace()
    {
      while (!is_end(s_itr_) && details::is_whitespace(*s_itr_))
      {
        ++s_itr_;
      }
    }

    inline void skip_comments()
    {
      //The following comment styles are supported:
      // 1. // .... \n
      // 2. #  .... \n
      // 3. /* .... */
      struct test
      {
        static inline bool comment_start(const char c0, const char c1, int& mode, int& incr)
        {
          mode = 0;
          if ('#' == c0)    { mode = 1; incr = 1; }
          else if ('/' == c0)
          {
            if ('/' == c1) { mode = 1; incr = 2; }
            else if ('*' == c1) { mode = 2; incr = 2; }
          }
          return (mode != 0);
        }

        static inline bool comment_end(const char c0, const char c1, const int mode)
        {
          return ((1 == mode) && ('\n' == c0)) ||
                 ((2 == mode) && ( '*' == c0) && ('/' == c1));
        }
      };

      int mode = 0;
      int increment = 0;

      if (is_end(s_itr_) || is_end((s_itr_ + 1)))
        return;
      else if (!test::comment_start(*s_itr_,*(s_itr_ + 1),mode,increment))
        return;

      s_itr_ += increment;

      while (!is_end(s_itr_) && !test::comment_end(*s_itr_,*(s_itr_ + 1),mode))
      {
        ++s_itr_;
      }

      if (!is_end(s_itr_))
      {
        s_itr_ += mode;
        skip_whitespace();
        skip_comments();
      }
    }

    inline void scan_token()
    {
      skip_whitespace();

      skip_comments();

      if (is_end(s_itr_))
      {
        return;
      }
      else if (details::is_operator_char(*s_itr_))
      {
        scan_operator();
        return;
      }
      else if (details::is_letter(*s_itr_))
      {
        scan_symbol();
        return;
      }
      else if (details::is_digit((*s_itr_)) || ('.' == (*s_itr_)))
      {
        scan_number();
        return;
      }
      else if ('\'' == (*s_itr_))
      {
        scan_string();
        return;
      }
      else
      {
        token_t t;
        t.set_error(token::e_error,s_itr_,s_itr_ + 2,base_itr_);
        token_list_.push_back(t);
        ++s_itr_;
      }
    }

    inline void scan_operator()
    {
      token_t t;

      if (!is_end(s_itr_ + 1))
      {
        token_t::token_type ttype = token_t::e_none;

        char c0 = s_itr_[0];
        char c1 = s_itr_[1];

        if ((c0 == '<') && (c1 == '=')) ttype = token_t::e_lte;
        else if ((c0 == '>') && (c1 == '=')) ttype = token_t::e_gte;
        else if ((c0 == '<') && (c1 == '>')) ttype = token_t::e_ne;
        else if ((c0 == '!') && (c1 == '=')) ttype = token_t::e_ne;
        else if ((c0 == '=') && (c1 == '=')) ttype = token_t::e_eq;
        else if ((c0 == ':') && (c1 == '=')) ttype = token_t::e_assign;
        else if ((c0 == '<') && (c1 == '<')) ttype = token_t::e_shl;
        else if ((c0 == '>') && (c1 == '>')) ttype = token_t::e_shr;

        if (token_t::e_none != ttype)
        {
          t.set_operator(ttype,s_itr_,s_itr_ + 2,base_itr_);
          token_list_.push_back(t);
          s_itr_ += 2;
          return;
        }
      }

      if ('<' == *s_itr_)
        t.set_operator(token_t::e_lt ,s_itr_,s_itr_ + 1,base_itr_);
      else if ('>' == *s_itr_)
        t.set_operator(token_t::e_gt ,s_itr_,s_itr_ + 1,base_itr_);
      else if (';' == *s_itr_)
        t.set_operator(token_t::e_eof,s_itr_,s_itr_ + 1,base_itr_);
      else if ('&' == *s_itr_)
        t.set_symbol(s_itr_,s_itr_ + 1,base_itr_);
      else if ('|' == *s_itr_)
        t.set_symbol(s_itr_,s_itr_ + 1,base_itr_);
      else
        t.set_operator(token_t::token_type(*s_itr_),s_itr_,s_itr_ + 1,base_itr_);

      token_list_.push_back(t);

      ++s_itr_;
    }

    inline void scan_symbol()
    {
      const char* begin = s_itr_;
      while (
        (!is_end(s_itr_)) &&
        (details::is_letter_or_digit(*s_itr_) || ((*s_itr_) == '_'))
        )
      {
        ++s_itr_;
      }
      token_t t;
      t.set_symbol(begin,s_itr_,base_itr_);
      token_list_.push_back(t);
    }

    inline void scan_number()
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
      const char* begin       = s_itr_;
      bool dot_found          = false;
      bool e_found            = false;
      bool post_e_sign_found  = false;
      bool post_e_digit_found = false;
      token_t t;

      while (!is_end(s_itr_))
      {
        if ('.' == (*s_itr_))
        {
          if (dot_found)
          {
            t.set_error(token::e_err_number,begin,s_itr_,base_itr_);
            token_list_.push_back(t);

            return;
          }

          dot_found = true;
          ++s_itr_;

          continue;
        }
        else if (details::imatch('e',(*s_itr_)))
        {
          const char& c = *(s_itr_ + 1);

          if (is_end(s_itr_ + 1))
          {
            t.set_error(token::e_err_number,begin,s_itr_,base_itr_);
            token_list_.push_back(t);

            return;
          }
          else if (
            ('+' != c) &&
            ('-' != c) &&
            !details::is_digit(c)
            )
          {
            t.set_error(token::e_err_number,begin,s_itr_,base_itr_);
            token_list_.push_back(t);

            return;
          }

          e_found = true;
          ++s_itr_;

          continue;
        }
        else if (e_found && details::is_sign(*s_itr_) && !post_e_digit_found)
        {
          if (post_e_sign_found)
          {
            t.set_error(token::e_err_number,begin,s_itr_,base_itr_);
            token_list_.push_back(t);

            return;
          }

          post_e_sign_found = true;
          ++s_itr_;

          continue;
        }
        else if (e_found && details::is_digit(*s_itr_))
        {
          post_e_digit_found = true;
          ++s_itr_;

          continue;
        }
        else if (('.' != (*s_itr_)) && !details::is_digit(*s_itr_))
          break;
        else
          ++s_itr_;
      }

      t.set_numeric(begin,s_itr_,base_itr_);

      token_list_.push_back(t);

      return;
    }

    inline void scan_string()
    {
      const char* begin = s_itr_ + 1;

      token_t t;

      if (std::distance(s_itr_,s_end_) < 2)
      {
        t.set_error(token::e_err_string,s_itr_,s_end_,base_itr_);
        token_list_.push_back(t);

        return;
      }

      ++s_itr_;

      bool escaped_found = false;
      bool escaped       = false;

      while (!is_end(s_itr_))
      {
        if (!escaped && ('\\' == *s_itr_))
        {
          escaped_found = true;
          escaped       = true;
          ++s_itr_;

          continue;
        }
        else if (!escaped)
        {
          if ('\'' == *s_itr_)
            break;
        }
        else if (escaped)
          escaped = false;

        ++s_itr_;
      }

      if (is_end(s_itr_))
      {
        t.set_error(token::e_err_string,begin,s_itr_,base_itr_);
        token_list_.push_back(t);

        return;
      }

      if (!escaped_found)
        t.set_string(begin,s_itr_,base_itr_);
      else
      {
        std::string parsed_string(begin,s_itr_);
        details::cleanup_escapes(parsed_string);
        t.set_string(parsed_string, std::distance(base_itr_,begin));
      }

      token_list_.push_back(t);
      ++s_itr_;

      return;
    }

private:

    token_list_t token_list_;
    token_list_itr_t token_itr_;
    token_list_itr_t store_token_itr_;
    token_t eof_token_;
    const char* base_itr_;
    const char* s_itr_;
    const char* s_end_;

    friend class token_scanner;
    friend class token_modifier;
    friend class token_inserter;
    friend class token_joiner;
};
}  // namespace lexertk

#endif  //LEXERTK_GENERATOR_HPP
