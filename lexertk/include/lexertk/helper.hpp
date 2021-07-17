//
// Created by allspark on 17/07/2021.
//

#ifndef LEXERTK_HELPER_HPP
#define LEXERTK_HELPER_HPP

#include "generator.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <stack>

namespace lexertk
{
class helper_interface
{
public:
  virtual void init()
  {
  }
  virtual void reset()
  {
  }
  virtual bool result()
  {
    return true;
  }
  virtual std::size_t process(generator::token_list_t&)
  {
    return 0;
  }
  virtual ~helper_interface() = default;
};

class token_scanner : public helper_interface
{
public:
  explicit token_scanner(const std::size_t& stride)
    : stride_(stride)
  {
    if (stride > 4)
    {
      throw std::invalid_argument("token_scanner() - Invalid stride value");
    }
  }

  inline std::size_t process(generator::token_list_t& list) override
  {
    if (!list.empty())
    {
      for (std::size_t i = 0; i < (list.size() - stride_ + 1); ++i)
      {
        token t;
        switch (stride_)
        {
          case 1:
          {
            const token& t0 = list[i];

            if (!operator()(t0))
              return i;
          }
          break;

          case 2:
          {
            const token& t0 = list[i];
            const token& t1 = list[i + 1];

            if (!operator()(t0, t1))
              return i;
          }
          break;

          case 3:
          {
            const token& t0 = list[i];
            const token& t1 = list[i + 1];
            const token& t2 = list[i + 2];

            if (!operator()(t0, t1, t2))
              return i;
          }
          break;

          case 4:
          {
            const token& t0 = list[i];
            const token& t1 = list[i + 1];
            const token& t2 = list[i + 2];
            const token& t3 = list[i + 3];

            if (!operator()(t0, t1, t2, t3))
              return i;
          }
          break;
        }
      }
    }

    return list.size() - stride_ + 1;
  }

  virtual bool operator()(const token&)
  {
    return false;
  }

  virtual bool operator()(const token&, const token&)
  {
    return false;
  }

  virtual bool operator()(const token&, const token&, const token&)
  {
    return false;
  }

  virtual bool operator()(const token&, const token&, const token&, const token&)
  {
    return false;
  }

private:
  std::size_t stride_;
};

class token_modifier : public helper_interface
{
public:
  inline std::size_t process(generator::token_list_t& list) override
  {
    std::size_t changes = 0;

    for (std::size_t i = 0; i < list.size(); ++i)
    {
      if (modify(list[i]))
        changes++;
    }

    return changes;
  }

  virtual bool modify(token& t) = 0;
};

class token_inserter : public helper_interface
{
public:
  explicit token_inserter(const std::size_t& stride)
    : stride_(stride)
  {
    if (stride > 5)
    {
      throw std::invalid_argument("token_inserter() - Invalid stride value");
    }
  }

  inline std::size_t process(generator::token_list_t& list) override
  {
    if (list.empty())
      return 0;

    std::size_t changes = 0;

    for (std::size_t i = 0; i < (list.size() - stride_ + 1); ++i)
    {
      auto [insert_index, t] = [&]() -> std::tuple<int, token>
      {
        switch (stride_)
        {
          case 1:
            return insert(list[i]);
          case 2:
            return insert(list[i], list[i + 1]);
          case 3:
            return insert(list[i], list[i + 1], list[i + 2]);
          case 4:
            return insert(list[i], list[i + 1], list[i + 2], list[i + 3]);
          case 5:
            return insert(list[i], list[i + 1], list[i + 2], list[i + 3], list[i + 4]);
        }
        return {-1, {}};
      }();

      if ((insert_index >= 0) && (insert_index <= (static_cast<int>(stride_) + 1)))
      {
        list.insert(list.begin() + (i + insert_index), t);
        changes++;
      }
    }

    return changes;
  }

  virtual inline std::tuple<int, token> insert(const token&)
  {
    return {-1, {}};
  }

  virtual inline std::tuple<int, token> insert(const token&, const token&)
  {
    return {-1, {}};
  }

  virtual inline std::tuple<int, token> insert(const token&, const token&, const token&)
  {
    return {-1, {}};
  }

  virtual inline std::tuple<int, token> insert(const token&, const token&, const token&, const token&)
  {
    return {-1, {}};
  }

  virtual inline std::tuple<int, token> insert(const token&, const token&, const token&, const token&, const token&)
  {
    return {-1, {}};
  }

private:
  std::size_t stride_;
};

class token_joiner : public helper_interface
{
public:
  inline std::size_t process(generator::token_list_t& list) override
  {
    if (list.empty())
      return 0;

    std::size_t changes = 0;

    for (std::size_t i = 0; i < list.size() - 1; ++i)
    {
      if (auto [success, t] = join(list[i], list[i + 1]); success)
      {
        list[i] = t;
        list.erase(list.begin() + (i + 1));

        ++changes;
      }
    }

    return changes;
  }

  virtual std::tuple<bool, token> join(const token&, const token&) = 0;
};
namespace helper
{
class commutative_inserter : public token_inserter
{
public:
  commutative_inserter()
    : lexertk::token_inserter(2)
  {
  }

  inline void ignore_symbol(const std::string& symbol)
  {
    ignore_set_.insert(symbol);
  }

  inline std::tuple<int, token> insert(const lexertk::token& t0, const lexertk::token& t1) override
  {
    bool match = false;

    if (t0.get_type() == lexertk::token::token_type::symbol)
    {
      if (ignore_set_.end() != ignore_set_.find(t0.get_value()))
      {
        return {-1, {}};
      }
      else if (!t0.get_value().empty() && ('$' == t0.get_value()[0]))
      {
        return {-1, {}};
      }
    }

    if (t1.get_type() == lexertk::token::token_type::symbol)
    {
      if (ignore_set_.end() != ignore_set_.find(t1.get_value()))
      {
        return {-1, {}};
      }
    }

    if ((t0.get_type() == lexertk::token::token_type::number) && (t1.get_type() == lexertk::token::token_type::symbol))
      match = true;
    else if ((t0.get_type() == lexertk::token::token_type::number) && (t1.get_type() == lexertk::token::token_type::lbracket))
      match = true;
    else if ((t0.get_type() == lexertk::token::token_type::number) && (t1.get_type() == lexertk::token::token_type::lcrlbracket))
      match = true;
    else if ((t0.get_type() == lexertk::token::token_type::number) && (t1.get_type() == lexertk::token::token_type::lsqrbracket))
      match = true;
    else if ((t0.get_type() == lexertk::token::token_type::symbol) && (t1.get_type() == lexertk::token::token_type::number))
      match = true;
    else if ((t0.get_type() == lexertk::token::token_type::rbracket) && (t1.get_type() == lexertk::token::token_type::number))
      match = true;
    else if ((t0.get_type() == lexertk::token::token_type::rcrlbracket) && (t1.get_type() == lexertk::token::token_type::number))
      match = true;
    else if ((t0.get_type() == lexertk::token::token_type::rsqrbracket) && (t1.get_type() == lexertk::token::token_type::number))
      match = true;
    else if ((t0.get_type() == lexertk::token::token_type::rbracket) && (t1.get_type() == lexertk::token::token_type::symbol))
      match = true;
    else if ((t0.get_type() == lexertk::token::token_type::rcrlbracket) && (t1.get_type() == lexertk::token::token_type::symbol))
      match = true;
    else if ((t0.get_type() == lexertk::token::token_type::rsqrbracket) && (t1.get_type() == lexertk::token::token_type::symbol))
      match = true;

    if (match)
    {
      return {1, {lexertk::token::token_type::mul, "*", t1.get_position()}};
    }
    return {-1, {}};
  }

private:
  std::set<std::string, details::ilesscompare> ignore_set_;
};

class operator_joiner : public token_joiner
{
public:
  inline std::tuple<bool, token> join(const lexertk::token& t0, const lexertk::token& t1) override
  {
    //': =' --> ':='
    if ((t0.get_type() == lexertk::token::token_type::colon) && (t1.get_type() == lexertk::token::token_type::eq))
    {
      return {true, {lexertk::token::token_type::assign, ":=", t0.get_position()}};
    }
    //'> =' --> '>='
    else if ((t0.get_type() == lexertk::token::token_type::gt) && (t1.get_type() == lexertk::token::token_type::eq))
    {
      return {true, {lexertk::token::token_type::gte, ">=", t0.get_position()}};
    }
    //'< =' --> '<='
    else if ((t0.get_type() == lexertk::token::token_type::lt) && (t1.get_type() == lexertk::token::token_type::eq))
    {
      return {true, {lexertk::token::token_type::lte, "<=", t0.get_position()}};
    }
    //'= =' --> '=='
    else if ((t0.get_type() == lexertk::token::token_type::eq) && (t1.get_type() == lexertk::token::token_type::eq))
    {
      return {true, {lexertk::token::token_type::eq, "==", t0.get_position()}};
    }
    //'! =' --> '!='
    else if ((static_cast<char>(t0.get_type()) == '!') && (t1.get_type() == lexertk::token::token_type::eq))
    {
      return {true, {lexertk::token::token_type::ne, "!=", t0.get_position()}};
    }
    //'< >' --> '<>'
    else if ((t0.get_type() == lexertk::token::token_type::lt) && (t1.get_type() == lexertk::token::token_type::gt))
    {
      return {true, {lexertk::token::token_type::ne, "<>", t0.get_position()}};
    }

    return {false, {}};
  }
};

class bracket_checker : public token_scanner
{
public:
  bracket_checker()
    : token_scanner(1)
    , state_(true)
  {
  }

  bool result() override
  {
    return state_ && stack_.empty();
  }

  lexertk::token error_token()
  {
    return error_token_;
  }

  void reset() override
  {
    //why? because msvc doesn't support swap properly.
    stack_ = std::stack<char>();
    state_ = true;
    error_token_ = {};
  }

  bool operator()(const lexertk::token& t) override
  {
    if (
        !t.get_value().empty() &&
        (lexertk::token::token_type::string != t.get_type()) &&
        (lexertk::token::token_type::symbol != t.get_type()) &&
        details::is_bracket(t.get_value()[0]))
    {
      char c = t.get_value()[0];

      if (t.get_type() == lexertk::token::token_type::lbracket)
        stack_.push(')');
      else if (t.get_type() == lexertk::token::token_type::lcrlbracket)
        stack_.push('}');
      else if (t.get_type() == lexertk::token::token_type::lsqrbracket)
        stack_.push(']');
      else if (details::is_right_bracket(c))
      {
        if (stack_.empty())
        {
          state_ = false;
          error_token_ = t;

          return false;
        }
        else if (c != stack_.top())
        {
          state_ = false;
          error_token_ = t;

          return false;
        }
        else
          stack_.pop();
      }
    }

    return true;
  }

private:
  bool state_;
  std::stack<char> stack_;
  lexertk::token error_token_;
};

class symbol_replacer : public token_modifier
{
private:
  typedef std::map<std::string, std::pair<std::string, token::token_type>, details::ilesscompare> replace_map_t;

public:
  bool remove(const std::string& target_symbol)
  {
    replace_map_t::iterator itr = replace_map_.find(target_symbol);

    if (replace_map_.end() == itr)
      return false;

    replace_map_.erase(itr);

    return true;
  }

  bool add_replace(const std::string& target_symbol,
      const std::string& replace_symbol,
      const lexertk::token::token_type token_type = lexertk::token::token_type::symbol)
  {
    replace_map_t::iterator itr = replace_map_.find(target_symbol);

    if (replace_map_.end() != itr)
    {
      return false;
    }

    replace_map_[target_symbol] = std::make_pair(replace_symbol, token_type);

    return true;
  }

  void clear()
  {
    replace_map_.clear();
  }

private:
  bool modify(lexertk::token& t)
  {
    if (lexertk::token::token_type::symbol == t.get_type())
    {
      if (replace_map_.empty())
        return false;

      replace_map_t::iterator itr = replace_map_.find(t.get_value());

      if (replace_map_.end() != itr)
      {
        t.set_value(itr->second.first);
        t.set_type(itr->second.second);

        return true;
      }
    }

    return false;
  }

  replace_map_t replace_map_;
};

class sequence_validator : public token_scanner
{
private:
  typedef std::pair<lexertk::token::token_type, lexertk::token::token_type> token_pair_t;
  typedef std::set<token_pair_t> set_t;

public:
  sequence_validator()
    : lexertk::token_scanner(2)
  {
    add_invalid(lexertk::token::token_type::number, lexertk::token::token_type::number);
    add_invalid(lexertk::token::token_type::string, lexertk::token::token_type::string);
    add_invalid(lexertk::token::token_type::number, lexertk::token::token_type::string);
    add_invalid(lexertk::token::token_type::string, lexertk::token::token_type::number);
    add_invalid(lexertk::token::token_type::string, lexertk::token::token_type::colon);
    add_invalid(lexertk::token::token_type::colon, lexertk::token::token_type::string);
    add_invalid_set1(lexertk::token::token_type::assign);
    add_invalid_set1(lexertk::token::token_type::shr);
    add_invalid_set1(lexertk::token::token_type::shl);
    add_invalid_set1(lexertk::token::token_type::lte);
    add_invalid_set1(lexertk::token::token_type::ne);
    add_invalid_set1(lexertk::token::token_type::gte);
    add_invalid_set1(lexertk::token::token_type::lt);
    add_invalid_set1(lexertk::token::token_type::gt);
    add_invalid_set1(lexertk::token::token_type::eq);
    add_invalid_set1(lexertk::token::token_type::comma);
    add_invalid_set1(lexertk::token::token_type::add);
    add_invalid_set1(lexertk::token::token_type::sub);
    add_invalid_set1(lexertk::token::token_type::div);
    add_invalid_set1(lexertk::token::token_type::mul);
    add_invalid_set1(lexertk::token::token_type::mod);
    add_invalid_set1(lexertk::token::token_type::pow);
    add_invalid_set1(lexertk::token::token_type::colon);
  }

  bool result()
  {
    return error_list_.empty();
  }

  bool operator()(const lexertk::token& t0, const lexertk::token& t1)
  {
    set_t::value_type p = std::make_pair(t0.get_type(), t1.get_type());

    if (invalid_bracket_check(t0.get_type(), t1.get_type()))
    {
      error_list_.push_back(std::make_pair(t0, t1));
    }
    else if (invalid_comb_.find(p) != invalid_comb_.end())
      error_list_.push_back(std::make_pair(t0, t1));

    return true;
  }

  std::size_t error_count()
  {
    return error_list_.size();
  }

  std::pair<lexertk::token, lexertk::token> error(const std::size_t index)
  {
    if (index < error_list_.size())
    {
      return error_list_[index];
    }
    else
    {
      static const lexertk::token error_token;
      return std::make_pair(error_token, error_token);
    }
  }

  void clear_errors()
  {
    error_list_.clear();
  }

private:
  void add_invalid(lexertk::token::token_type base, lexertk::token::token_type t)
  {
    invalid_comb_.insert(std::make_pair(base, t));
  }

  void add_invalid_set1(lexertk::token::token_type t)
  {
    add_invalid(t, lexertk::token::token_type::assign);
    add_invalid(t, lexertk::token::token_type::shr);
    add_invalid(t, lexertk::token::token_type::shl);
    add_invalid(t, lexertk::token::token_type::lte);
    add_invalid(t, lexertk::token::token_type::ne);
    add_invalid(t, lexertk::token::token_type::gte);
    add_invalid(t, lexertk::token::token_type::lt);
    add_invalid(t, lexertk::token::token_type::gt);
    add_invalid(t, lexertk::token::token_type::eq);
    add_invalid(t, lexertk::token::token_type::comma);
    add_invalid(t, lexertk::token::token_type::div);
    add_invalid(t, lexertk::token::token_type::mul);
    add_invalid(t, lexertk::token::token_type::mod);
    add_invalid(t, lexertk::token::token_type::pow);
    add_invalid(t, lexertk::token::token_type::colon);
  }

  bool invalid_bracket_check(lexertk::token::token_type base, lexertk::token::token_type t)
  {
    if (details::is_right_bracket(static_cast<char>(base)))
    {
      switch (t)
      {
        case lexertk::token::token_type::string:
          return true;
        case lexertk::token::token_type::assign:
          return true;
        default:
          return false;
      }
    }
    else if (details::is_left_bracket(static_cast<char>(base)))
    {
      if (details::is_right_bracket(static_cast<char>(t)))
        return false;
      else if (details::is_left_bracket(static_cast<char>(t)))
        return false;
      else
      {
        switch (t)
        {
          case lexertk::token::token_type::number:
            return false;
          case lexertk::token::token_type::symbol:
            return false;
          case lexertk::token::token_type::string:
            return false;
          case lexertk::token::token_type::add:
            return false;
          case lexertk::token::token_type::sub:
            return false;
          case lexertk::token::token_type::colon:
            return false;
          default:
            return true;
        }
      }
    }
    else if (details::is_right_bracket(static_cast<char>(t)))
    {
      switch (base)
      {
        case lexertk::token::token_type::number:
          return false;
        case lexertk::token::token_type::symbol:
          return false;
        case lexertk::token::token_type::string:
          return false;
        case lexertk::token::token_type::eof:
          return false;
        case lexertk::token::token_type::colon:
          return false;
        default:
          return true;
      }
    }
    else if (details::is_left_bracket(static_cast<char>(t)))
    {
      switch (base)
      {
        case lexertk::token::token_type::rbracket:
          return true;
        case lexertk::token::token_type::rsqrbracket:
          return true;
        case lexertk::token::token_type::rcrlbracket:
          return true;
        default:
          return false;
      }
    }

    return false;
  }

  set_t invalid_comb_;
  std::deque<std::pair<lexertk::token, lexertk::token>> error_list_;
};

struct helper_assembly
{
  inline bool register_scanner(lexertk::token_scanner* scanner)
  {
    if (token_scanner_list.end() != std::find(token_scanner_list.begin(), token_scanner_list.end(), scanner))
    {
      return false;
    }

    token_scanner_list.push_back(scanner);

    return true;
  }

  inline bool register_modifier(lexertk::token_modifier* modifier)
  {
    if (token_modifier_list.end() != std::find(token_modifier_list.begin(), token_modifier_list.end(), modifier))
    {
      return false;
    }

    token_modifier_list.push_back(modifier);

    return true;
  }

  inline bool register_joiner(lexertk::token_joiner* joiner)
  {
    if (token_joiner_list.end() != std::find(token_joiner_list.begin(), token_joiner_list.end(), joiner))
    {
      return false;
    }

    token_joiner_list.push_back(joiner);

    return true;
  }

  inline bool register_inserter(lexertk::token_inserter* inserter)
  {
    if (token_inserter_list.end() != std::find(token_inserter_list.begin(), token_inserter_list.end(), inserter))
    {
      return false;
    }

    token_inserter_list.push_back(inserter);

    return true;
  }

  inline bool run_modifiers(lexertk::generator::token_list_t& list)
  {
    error_token_modifier = reinterpret_cast<lexertk::token_modifier*>(0);

    for (std::size_t i = 0; i < token_modifier_list.size(); ++i)
    {
      lexertk::token_modifier& modifier = (*token_modifier_list[i]);

      modifier.reset();
      modifier.process(list);

      if (!modifier.result())
      {
        error_token_modifier = token_modifier_list[i];

        return false;
      }
    }

    return true;
  }

  inline bool run_joiners(lexertk::generator::token_list_t& list)
  {
    error_token_joiner = reinterpret_cast<lexertk::token_joiner*>(0);

    for (std::size_t i = 0; i < token_joiner_list.size(); ++i)
    {
      lexertk::token_joiner& joiner = (*token_joiner_list[i]);

      joiner.reset();
      joiner.process(list);

      if (!joiner.result())
      {
        error_token_joiner = token_joiner_list[i];

        return false;
      }
    }

    return true;
  }

  inline bool run_inserters(lexertk::generator::token_list_t& list)
  {
    error_token_inserter = reinterpret_cast<lexertk::token_inserter*>(0);

    for (std::size_t i = 0; i < token_inserter_list.size(); ++i)
    {
      lexertk::token_inserter& inserter = (*token_inserter_list[i]);

      inserter.reset();
      inserter.process(list);

      if (!inserter.result())
      {
        error_token_inserter = token_inserter_list[i];

        return false;
      }
    }

    return true;
  }

  inline bool run_scanners(lexertk::generator::token_list_t& list)
  {
    error_token_scanner = reinterpret_cast<lexertk::token_scanner*>(0);

    for (std::size_t i = 0; i < token_scanner_list.size(); ++i)
    {
      lexertk::token_scanner& scanner = (*token_scanner_list[i]);

      scanner.reset();
      scanner.process(list);

      if (!scanner.result())
      {
        error_token_scanner = token_scanner_list[i];

        return false;
      }
    }

    return true;
  }

  std::deque<lexertk::token_scanner*> token_scanner_list;
  std::deque<lexertk::token_modifier*> token_modifier_list;
  std::deque<lexertk::token_joiner*> token_joiner_list;
  std::deque<lexertk::token_inserter*> token_inserter_list;

  lexertk::token_scanner* error_token_scanner;
  lexertk::token_modifier* error_token_modifier;
  lexertk::token_joiner* error_token_joiner;
  lexertk::token_inserter* error_token_inserter;
};
}  // namespace helper
class parser_helper
{
public:
  typedef token token_t;
  typedef generator generator_t;

  inline bool init(std::string_view str)
  {
    generator_t lexer;
    if (!lexer.process(str))
    {
      return false;
    }
    m_token_list = std::move(lexer).get_token_list();
    m_current_token = m_token_list.begin();

    return true;
  }

  inline void next_token()
  {
    if (m_current_token != m_token_list.end())
    {
      ++m_current_token;
    }
  }

  enum struct token_advance_mode
  {
    hold = 0,
    advance = 1
  };

  inline void advance_token(const token_advance_mode mode)
  {
    if (token_advance_mode::advance == mode)
    {
      next_token();
    }
  }

  inline bool token_is(const token_t::token_type& ttype, const token_advance_mode mode = token_advance_mode::advance)
  {
    if (current_token().get_type() != ttype)
    {
      return false;
    }

    advance_token(mode);

    return true;
  }

  inline bool token_is(const token_t::token_type& ttype, std::string_view value, const token_advance_mode mode = token_advance_mode::advance)
  {
    if (
        (current_token().get_type() != ttype) ||
        !details::imatch(value, current_token().get_value()))
    {
      return false;
    }

    advance_token(mode);

    return true;
  }

  inline bool token_is_then_assign(const token_t::token_type& ttype, std::string_view& token, const token_advance_mode mode = token_advance_mode::advance)
  {
    if (current_token().get_type() != ttype)
    {
      return false;
    }

    token = current_token().get_value();

    advance_token(mode);

    return true;
  }

  template <typename Allocator, template <typename, typename> class Container>
  inline bool token_is_then_assign(const token_t::token_type& ttype, Container<std::string_view, Allocator>& token_list, const token_advance_mode mode = token_advance_mode::advance)
  {
    if (current_token().get_type() != ttype)
    {
      return false;
    }

    token_list.push_back(current_token().get_value());

    advance_token(mode);

    return true;
  }

  inline bool peek_token_is(const token_t::token_type& ttype)
  {
    return current_token().get_type() == ttype;
  }

  inline bool peek_token_is(std::string_view s)
  {
    return (details::imatch(current_token().get_value(), s));
  }

  token& current_token() noexcept
  {
    if (m_current_token == m_token_list.end())
    {
      return m_eof_token;
    }
    return *m_current_token;
  }

protected:
  generator_t::token_list_t m_token_list;
  generator_t::token_list_t::iterator m_current_token;

  token m_eof_token{token::token_type::eof, token::Position{}};
};
}  // namespace lexertk

#endif  //LEXERTK_HELPER_HPP
