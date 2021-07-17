//
// Created by allspark on 17/07/2021.
//

#ifndef LEXERTK_HELPER_HPP
#define LEXERTK_HELPER_HPP

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
  virtual std::size_t process(generator&)
  {
    return 0;
  }
  virtual ~helper_interface()
  {
  }
};

class token_scanner : public helper_interface
{
public:
  virtual ~token_scanner()
  {
  }

  explicit token_scanner(const std::size_t& stride)
    : stride_(stride)
  {
    if (stride > 4)
    {
      throw std::invalid_argument("token_scanner() - Invalid stride value");
    }
  }

  inline std::size_t process(generator& g)
  {
    if (!g.token_list_.empty())
    {
      for (std::size_t i = 0; i < (g.token_list_.size() - stride_ + 1); ++i)
      {
        token t;
        switch (stride_)
        {
          case 1:
          {
            const token& t0 = g.token_list_[i];

            if (!operator()(t0))
              return i;
          }
          break;

          case 2:
          {
            const token& t0 = g.token_list_[i];
            const token& t1 = g.token_list_[i + 1];

            if (!operator()(t0, t1))
              return i;
          }
          break;

          case 3:
          {
            const token& t0 = g.token_list_[i];
            const token& t1 = g.token_list_[i + 1];
            const token& t2 = g.token_list_[i + 2];

            if (!operator()(t0, t1, t2))
              return i;
          }
          break;

          case 4:
          {
            const token& t0 = g.token_list_[i];
            const token& t1 = g.token_list_[i + 1];
            const token& t2 = g.token_list_[i + 2];
            const token& t3 = g.token_list_[i + 3];

            if (!operator()(t0, t1, t2, t3))
              return i;
          }
          break;
        }
      }
    }

    return (g.token_list_.size() - stride_ + 1);
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
  inline std::size_t process(generator& g)
  {
    std::size_t changes = 0;

    for (std::size_t i = 0; i < g.token_list_.size(); ++i)
    {
      if (modify(g.token_list_[i]))
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

  inline std::size_t process(generator& g)
  {
    if (g.token_list_.empty())
      return 0;

    std::size_t changes = 0;

    for (std::size_t i = 0; i < (g.token_list_.size() - stride_ + 1); ++i)
    {
      token t;
      int insert_index = -1;

      switch (stride_)
      {
        case 1:
          insert_index = insert(g.token_list_[i], t);
          break;

        case 2:
          insert_index = insert(g.token_list_[i], g.token_list_[i + 1], t);
          break;

        case 3:
          insert_index = insert(g.token_list_[i], g.token_list_[i + 1], g.token_list_[i + 2], t);
          break;

        case 4:
          insert_index = insert(g.token_list_[i], g.token_list_[i + 1], g.token_list_[i + 2], g.token_list_[i + 3], t);
          break;

        case 5:
          insert_index = insert(g.token_list_[i], g.token_list_[i + 1], g.token_list_[i + 2], g.token_list_[i + 3], g.token_list_[i + 4], t);
          break;
      }

      if ((insert_index >= 0) && (insert_index <= (static_cast<int>(stride_) + 1)))
      {
        g.token_list_.insert(g.token_list_.begin() + (i + insert_index), t);
        changes++;
      }
    }

    return changes;
  }

  virtual inline int insert(const token&, token&)
  {
    return -1;
  }

  virtual inline int insert(const token&, const token&, token&)
  {
    return -1;
  }

  virtual inline int insert(const token&, const token&, const token&, token&)
  {
    return -1;
  }

  virtual inline int insert(const token&, const token&, const token&, const token&, token&)
  {
    return -1;
  }

  virtual inline int insert(const token&, const token&, const token&, const token&, const token&, token&)
  {
    return -1;
  }

private:
  std::size_t stride_;
};

class token_joiner : public helper_interface
{
public:
  inline std::size_t process(generator& g)
  {
    if (g.token_list_.empty())
      return 0;

    std::size_t changes = 0;

    for (std::size_t i = 0; i < g.token_list_.size() - 1; ++i)
    {
      token t;

      if (join(g.token_list_[i], g.token_list_[i + 1], t))
      {
        g.token_list_[i] = t;
        g.token_list_.erase(g.token_list_.begin() + (i + 1));

        ++changes;
      }
    }

    return changes;
  }

  virtual bool join(const token&, const token&, token&) = 0;
};
namespace helper
{
inline void dump(lexertk::generator& generator)
{
  for (std::size_t i = 0; i < generator.size(); ++i)
  {
    lexertk::token t = generator[i];
    printf("Token[%02d] @ %03d  %6s  -->  '%s'\n",
        static_cast<unsigned int>(i),
        static_cast<unsigned int>(t.position),
        t.to_str(t.type).c_str(),
        t.value.c_str());
  }
}

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

  inline int insert(const lexertk::token& t0, const lexertk::token& t1, lexertk::token& new_token)
  {
    new_token.type = lexertk::token::e_mul;
    new_token.value = "*";
    new_token.position = t1.position;
    bool match = false;

    if (t0.type == lexertk::token::e_symbol)
    {
      if (ignore_set_.end() != ignore_set_.find(t0.value))
      {
        return -1;
      }
      else if (!t0.value.empty() && ('$' == t0.value[0]))
      {
        return -1;
      }
    }

    if (t1.type == lexertk::token::e_symbol)
    {
      if (ignore_set_.end() != ignore_set_.find(t1.value))
      {
        return -1;
      }
    }

    if ((t0.type == lexertk::token::e_number) && (t1.type == lexertk::token::e_symbol))
      match = true;
    else if ((t0.type == lexertk::token::e_number) && (t1.type == lexertk::token::e_lbracket))
      match = true;
    else if ((t0.type == lexertk::token::e_number) && (t1.type == lexertk::token::e_lcrlbracket))
      match = true;
    else if ((t0.type == lexertk::token::e_number) && (t1.type == lexertk::token::e_lsqrbracket))
      match = true;
    else if ((t0.type == lexertk::token::e_symbol) && (t1.type == lexertk::token::e_number))
      match = true;
    else if ((t0.type == lexertk::token::e_rbracket) && (t1.type == lexertk::token::e_number))
      match = true;
    else if ((t0.type == lexertk::token::e_rcrlbracket) && (t1.type == lexertk::token::e_number))
      match = true;
    else if ((t0.type == lexertk::token::e_rsqrbracket) && (t1.type == lexertk::token::e_number))
      match = true;
    else if ((t0.type == lexertk::token::e_rbracket) && (t1.type == lexertk::token::e_symbol))
      match = true;
    else if ((t0.type == lexertk::token::e_rcrlbracket) && (t1.type == lexertk::token::e_symbol))
      match = true;
    else if ((t0.type == lexertk::token::e_rsqrbracket) && (t1.type == lexertk::token::e_symbol))
      match = true;

    return (match) ? 1 : -1;
  }

private:
  std::set<std::string, details::ilesscompare> ignore_set_;
};

class operator_joiner : public token_joiner
{
public:
  inline bool join(const lexertk::token& t0, const lexertk::token& t1, lexertk::token& t)
  {
    //': =' --> ':='
    if ((t0.type == lexertk::token::e_colon) && (t1.type == lexertk::token::e_eq))
    {
      t.type = lexertk::token::e_assign;
      t.value = ":=";
      t.position = t0.position;

      return true;
    }
    //'> =' --> '>='
    else if ((t0.type == lexertk::token::e_gt) && (t1.type == lexertk::token::e_eq))
    {
      t.type = lexertk::token::e_gte;
      t.value = ">=";
      t.position = t0.position;

      return true;
    }
    //'< =' --> '<='
    else if ((t0.type == lexertk::token::e_lt) && (t1.type == lexertk::token::e_eq))
    {
      t.type = lexertk::token::e_lte;
      t.value = "<=";
      t.position = t0.position;

      return true;
    }
    //'= =' --> '=='
    else if ((t0.type == lexertk::token::e_eq) && (t1.type == lexertk::token::e_eq))
    {
      t.type = lexertk::token::e_eq;
      t.value = "==";
      t.position = t0.position;

      return true;
    }
    //'! =' --> '!='
    else if ((static_cast<char>(t0.type) == '!') && (t1.type == lexertk::token::e_eq))
    {
      t.type = lexertk::token::e_ne;
      t.value = "!=";
      t.position = t0.position;

      return true;
    }
    //'< >' --> '<>'
    else if ((t0.type == lexertk::token::e_lt) && (t1.type == lexertk::token::e_gt))
    {
      t.type = lexertk::token::e_ne;
      t.value = "<>";
      t.position = t0.position;

      return true;
    }
    else
      return false;
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

  bool result()
  {
    return state_ && stack_.empty();
  }

  lexertk::token error_token()
  {
    return error_token_;
  }

  void reset()
  {
    //why? because msvc doesn't support swap properly.
    stack_ = std::stack<char>();
    state_ = true;
    error_token_.clear();
  }

  bool operator()(const lexertk::token& t)
  {
    if (
        !t.value.empty() &&
        (lexertk::token::e_string != t.type) &&
        (lexertk::token::e_symbol != t.type) &&
        details::is_bracket(t.value[0]))
    {
      char c = t.value[0];

      if (t.type == lexertk::token::e_lbracket)
        stack_.push(')');
      else if (t.type == lexertk::token::e_lcrlbracket)
        stack_.push('}');
      else if (t.type == lexertk::token::e_lsqrbracket)
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
      const lexertk::token::token_type token_type = lexertk::token::e_symbol)
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
    if (lexertk::token::e_symbol == t.type)
    {
      if (replace_map_.empty())
        return false;

      replace_map_t::iterator itr = replace_map_.find(t.value);

      if (replace_map_.end() != itr)
      {
        t.value = itr->second.first;
        t.type = itr->second.second;

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
    add_invalid(lexertk::token::e_number, lexertk::token::e_number);
    add_invalid(lexertk::token::e_string, lexertk::token::e_string);
    add_invalid(lexertk::token::e_number, lexertk::token::e_string);
    add_invalid(lexertk::token::e_string, lexertk::token::e_number);
    add_invalid(lexertk::token::e_string, lexertk::token::e_colon);
    add_invalid(lexertk::token::e_colon, lexertk::token::e_string);
    add_invalid_set1(lexertk::token::e_assign);
    add_invalid_set1(lexertk::token::e_shr);
    add_invalid_set1(lexertk::token::e_shl);
    add_invalid_set1(lexertk::token::e_lte);
    add_invalid_set1(lexertk::token::e_ne);
    add_invalid_set1(lexertk::token::e_gte);
    add_invalid_set1(lexertk::token::e_lt);
    add_invalid_set1(lexertk::token::e_gt);
    add_invalid_set1(lexertk::token::e_eq);
    add_invalid_set1(lexertk::token::e_comma);
    add_invalid_set1(lexertk::token::e_add);
    add_invalid_set1(lexertk::token::e_sub);
    add_invalid_set1(lexertk::token::e_div);
    add_invalid_set1(lexertk::token::e_mul);
    add_invalid_set1(lexertk::token::e_mod);
    add_invalid_set1(lexertk::token::e_pow);
    add_invalid_set1(lexertk::token::e_colon);
  }

  bool result()
  {
    return error_list_.empty();
  }

  bool operator()(const lexertk::token& t0, const lexertk::token& t1)
  {
    set_t::value_type p = std::make_pair(t0.type, t1.type);

    if (invalid_bracket_check(t0.type, t1.type))
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
    add_invalid(t, lexertk::token::e_assign);
    add_invalid(t, lexertk::token::e_shr);
    add_invalid(t, lexertk::token::e_shl);
    add_invalid(t, lexertk::token::e_lte);
    add_invalid(t, lexertk::token::e_ne);
    add_invalid(t, lexertk::token::e_gte);
    add_invalid(t, lexertk::token::e_lt);
    add_invalid(t, lexertk::token::e_gt);
    add_invalid(t, lexertk::token::e_eq);
    add_invalid(t, lexertk::token::e_comma);
    add_invalid(t, lexertk::token::e_div);
    add_invalid(t, lexertk::token::e_mul);
    add_invalid(t, lexertk::token::e_mod);
    add_invalid(t, lexertk::token::e_pow);
    add_invalid(t, lexertk::token::e_colon);
  }

  bool invalid_bracket_check(lexertk::token::token_type base, lexertk::token::token_type t)
  {
    if (details::is_right_bracket(static_cast<char>(base)))
    {
      switch (t)
      {
        case lexertk::token::e_string:
          return true;
        case lexertk::token::e_assign:
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
          case lexertk::token::e_number:
            return false;
          case lexertk::token::e_symbol:
            return false;
          case lexertk::token::e_string:
            return false;
          case lexertk::token::e_add:
            return false;
          case lexertk::token::e_sub:
            return false;
          case lexertk::token::e_colon:
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
        case lexertk::token::e_number:
          return false;
        case lexertk::token::e_symbol:
          return false;
        case lexertk::token::e_string:
          return false;
        case lexertk::token::e_eof:
          return false;
        case lexertk::token::e_colon:
          return false;
        default:
          return true;
      }
    }
    else if (details::is_left_bracket(static_cast<char>(t)))
    {
      switch (base)
      {
        case lexertk::token::e_rbracket:
          return true;
        case lexertk::token::e_rsqrbracket:
          return true;
        case lexertk::token::e_rcrlbracket:
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

  inline bool run_modifiers(lexertk::generator& g)
  {
    error_token_modifier = reinterpret_cast<lexertk::token_modifier*>(0);

    for (std::size_t i = 0; i < token_modifier_list.size(); ++i)
    {
      lexertk::token_modifier& modifier = (*token_modifier_list[i]);

      modifier.reset();
      modifier.process(g);

      if (!modifier.result())
      {
        error_token_modifier = token_modifier_list[i];

        return false;
      }
    }

    return true;
  }

  inline bool run_joiners(lexertk::generator& g)
  {
    error_token_joiner = reinterpret_cast<lexertk::token_joiner*>(0);

    for (std::size_t i = 0; i < token_joiner_list.size(); ++i)
    {
      lexertk::token_joiner& joiner = (*token_joiner_list[i]);

      joiner.reset();
      joiner.process(g);

      if (!joiner.result())
      {
        error_token_joiner = token_joiner_list[i];

        return false;
      }
    }

    return true;
  }

  inline bool run_inserters(lexertk::generator& g)
  {
    error_token_inserter = reinterpret_cast<lexertk::token_inserter*>(0);

    for (std::size_t i = 0; i < token_inserter_list.size(); ++i)
    {
      lexertk::token_inserter& inserter = (*token_inserter_list[i]);

      inserter.reset();
      inserter.process(g);

      if (!inserter.result())
      {
        error_token_inserter = token_inserter_list[i];

        return false;
      }
    }

    return true;
  }

  inline bool run_scanners(lexertk::generator& g)
  {
    error_token_scanner = reinterpret_cast<lexertk::token_scanner*>(0);

    for (std::size_t i = 0; i < token_scanner_list.size(); ++i)
    {
      lexertk::token_scanner& scanner = (*token_scanner_list[i]);

      scanner.reset();
      scanner.process(g);

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

  inline bool init(const std::string& str)
  {
    if (!lexer_.process(str))
    {
      return false;
    }

    lexer_.begin();

    next_token();

    return true;
  }

  inline generator_t& lexer()
  {
    return lexer_;
  }

  inline const generator_t& lexer() const
  {
    return lexer_;
  }

  inline void next_token()
  {
    current_token_ = lexer_.next_token();
  }

  inline const token_t& current_token() const
  {
    return current_token_;
  }

  enum token_advance_mode
  {
    e_hold = 0,
    e_advance = 1
  };

  inline void advance_token(const token_advance_mode mode)
  {
    if (e_advance == mode)
    {
      next_token();
    }
  }

  inline bool token_is(const token_t::token_type& ttype, const token_advance_mode mode = e_advance)
  {
    if (current_token().type != ttype)
    {
      return false;
    }

    advance_token(mode);

    return true;
  }

  inline bool token_is(const token_t::token_type& ttype,
      const std::string& value,
      const token_advance_mode mode = e_advance)
  {
    if (
        (current_token().type != ttype) ||
        !details::imatch(value, current_token().value))
    {
      return false;
    }

    advance_token(mode);

    return true;
  }

  inline bool token_is_then_assign(const token_t::token_type& ttype,
      std::string& token,
      const token_advance_mode mode = e_advance)
  {
    if (current_token_.type != ttype)
    {
      return false;
    }

    token = current_token_.value;

    advance_token(mode);

    return true;
  }

  template <typename Allocator,
      template <typename, typename> class Container>
  inline bool token_is_then_assign(const token_t::token_type& ttype,
      Container<std::string, Allocator>& token_list,
      const token_advance_mode mode = e_advance)
  {
    if (current_token_.type != ttype)
    {
      return false;
    }

    token_list.push_back(current_token_.value);

    advance_token(mode);

    return true;
  }

  inline bool peek_token_is(const token_t::token_type& ttype)
  {
    return (lexer_.peek_next_token().type == ttype);
  }

  inline bool peek_token_is(const std::string& s)
  {
    return (details::imatch(lexer_.peek_next_token().value, s));
  }

private:
  generator_t lexer_;
  token_t current_token_;
};
}  // namespace lexertk

#endif  //LEXERTK_HELPER_HPP
