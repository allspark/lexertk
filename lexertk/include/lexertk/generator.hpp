//
// Created by dboerm on 6/25/21.
//

#ifndef LEXERTK_GENERATOR_HPP
#define LEXERTK_GENERATOR_HPP

#include "token.hpp"
#include "detail.hpp"

#include <vector>

namespace lexertk
{
class generator
{
  using iterator = std::string_view::const_iterator;
  struct Range
  {
    iterator begin;
    iterator end;

    inline explicit operator bool() const noexcept;
    inline Range& operator++() noexcept;
    inline Range& operator+=(std::size_t off) noexcept;
  };

public:
  using token_list_t = std::vector<token>;
  using token_list_itr_t = typename token_list_t::const_iterator;

  struct Settings
  {
    bool hash_as_comment{true};
    std::size_t lineOffset{0};
  };

  generator() = default;
  inline explicit generator(Settings settings);
  generator(generator const&) = delete;
  generator(generator&&) = delete;
  generator& operator=(generator const&) = delete;
  generator& operator=(generator&&) = delete;
  ~generator() = default;

  inline bool process(std::string_view line);

  inline token_list_t const& get_token_list() const & noexcept;
  inline token_list_t get_token_list() && noexcept;

private:
  inline Range skip_whitespace(Range) noexcept;
  inline Range skip_comments(Range) noexcept;
  inline Range scan_token(Range) noexcept;
  inline Range scan_operator(Range) noexcept;
  inline Range scan_symbol(Range) noexcept;
  inline Range scan_number(Range) noexcept;
  inline Range scan_string(Range) noexcept;

private:
  token_list_t m_token_list;
  token::Position m_currentPosition{0, 0};
  token m_eof_token{token::token_type::eof, token::Position{}};

  Settings m_settings;
};
inline void dump(generator::token_list_t const& list);
}  // namespace lexertk

#include "generator.ipp"

#endif  //LEXERTK_GENERATOR_HPP
