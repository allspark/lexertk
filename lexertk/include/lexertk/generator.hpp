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
public:
  using token_list_t = std::vector<token>;
  using token_list_itr_t = typename token_list_t::const_iterator;
  using iterator = std::string_view::const_iterator;
  struct Range
  {
    iterator begin;
    iterator end;

    inline explicit operator bool() const noexcept;
    inline Range& operator++() noexcept;
    inline Range& operator+=(std::size_t off) noexcept;
  };

  generator() = default;
  generator(generator const&) = delete;
  generator(generator&&) = delete;
  generator& operator=(generator const&) = delete;
  generator& operator=(generator&&) = delete;
  ~generator() = default;

  inline bool process(std::string_view line);

  inline bool empty() const noexcept;

  inline std::size_t size() const noexcept;

  inline token const& operator[](const std::size_t& index) const;

  inline token_list_itr_t begin() const noexcept;
  inline token_list_itr_t end() const noexcept;

  inline token_list_t const& get_token_list() const & noexcept;
  inline token_list_t get_token_list() && noexcept;

private:
  Range skip_whitespace(Range);
  Range skip_comments(Range);
  Range scan_token(Range);
  Range scan_operator(Range);
  Range scan_symbol(Range);
  Range scan_number(Range);
  Range scan_string(Range);

private:
  token_list_t m_token_list;
  token::Position m_currentPosition{0, 0};
  token m_eof_token{token::token_type::eof, token::Position{}};
};
inline void dump(generator::token_list_t const& list);
}  // namespace lexertk

#include "generator.ipp"

#endif  //LEXERTK_GENERATOR_HPP
