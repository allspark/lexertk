/*
 *****************************************************************
 *                 Simple C++ Lexer Toolkit Library              *
 *                                                               *
 * Lexer Examples                                                *
 * Author: Arash Partow (2001)                                   *
 * URL: http://www.partow.net/programming/lexertk/index.html     *
 *                                                               *
 * Copyright notice:                                             *
 * Free use of the Simple C++ Lexer Toolkit Library is permitted *
 * under the guidelines and in accordance with the MIT License.  *
 * http://www.opensource.org/licenses/MIT                        *
 *                                                               *
 *****************************************************************
*/

#include <iostream>
#include <string>
#include <vector>

#include <lexertk/helper.hpp>
#include <lexertk/lexertk.hpp>

#include <fmt/format.h>

void example01()
{
  std::string expression = "(sin(x/pi)cos(2y) + 1) == (sin(x / pi) * cos(2 * y) + 1)";

  lexertk::generator generator;

  if (!generator.process(expression))
  {
    fmt::print("Example01 - Failed to lex: {}\n", expression);
    return;
  }

  fmt::print("***** Example01 *****\n");
  lexertk::dump(generator.get_token_list());
  fmt::print("*********************\n");
}

void example02()
{
  std::string expression = "(sin(x/pi)123.456cos(2y) + 1) == (sin(x / pi) * 123.456 * cos(2 * y) + 1)";

  lexertk::generator generator;

  if (!generator.process(expression))
  {
    fmt::print("Example02 - Failed to lex: {}\n", expression);
    return;
  }

  lexertk::helper::commutative_inserter ci;
  auto list = std::move(generator).get_token_list();
  ci.process(list);

  fmt::print("***** Example02 *****\n");
  lexertk::dump(list);
  fmt::print("*********************\n");
}

void example03()
{
  std::string expression = "((1.1 > = 2.2) ! = (3.3 < = 4.4)) < > [x_x : = y_y]";

  lexertk::generator generator;

  if (!generator.process(expression))
  {
    fmt::print("Example03 - Failed to lex: {}\n", expression);
    return;
  }

  lexertk::helper::operator_joiner oj;
  auto list = std::move(generator).get_token_list();
  oj.process(list);

  fmt::print("***** Example03 *****\n");
  lexertk::dump(list);
  fmt::print("*********************\n");
}

void example04()
{
  std::string expression = "{a+(b-[c*(e/{f+g}-h)*i]%[j+(k-{l*m}/n)+o]-p)*q}";

  lexertk::generator generator;

  if (!generator.process(expression))
  {
    fmt::print("Example04 - Failed to lex: {}\n", expression);
    return;
  }

  lexertk::helper::bracket_checker bc;
  auto list = std::move(generator).get_token_list();
  bc.process(list);

  if (!bc.result())
  {
    fmt::print("Example04 - Failed Bracket Checker!\n");
    return;
  }

  fmt::print("***** Example04 *****\n");
  lexertk::dump(list);
  fmt::print("*********************\n");
}

void example05()
{
  std::string expression = "(sin(x/pi) * cos(2y) + 1)";

  lexertk::generator generator;

  if (!generator.process(expression))
  {
    fmt::print("Example05 - Failed to lex: {}\n", expression);
    return;
  }

  lexertk::helper::symbol_replacer sr;

  sr.add_replace("sin", "Deg2RadSine");
  sr.add_replace("cos", "Deg2RadCosine");

  auto list = std::move(generator).get_token_list();

  std::size_t change_count = sr.process(list);

  fmt::print("Example05 - Number of changes: {}\n", change_count);

  fmt::print("***** Example05 *****\n");
  lexertk::dump(list);
  fmt::print("*********************\n");
}

struct function_definition
{
  std::string_view name;
  std::string_view body;
  std::vector<std::string_view> var_list;
};

struct parse_function_definition_impl : public lexertk::parser_helper
{
  /*
      Structure: function <name> (v0,v1,...,vn) { expression }
   */
  std::tuple<bool, std::string_view> process(std::string_view func_def, function_definition& fd)
  {
    if (!init(func_def))
      return {false, {}};

    if (!token_is(token_t::token_type::symbol, "function"))
      return {false, {}};

    if (!token_is_then_assign(token_t::token_type::symbol, fd.name))
      return {false, {}};

    if (!token_is(token_t::token_type::lbracket))
      return {false, {}};

    if (!token_is(token_t::token_type::rbracket))
    {
      std::vector<std::string_view> var_list;

      for (;;)
      {
        // (x,y,z,....w)
        if (!token_is_then_assign(token_t::token_type::symbol, var_list))
          return {false, {}};

        if (token_is(token_t::token_type::rbracket))
          break;

        if (!token_is(token_t::token_type::comma))
          return {false, {}};
      }

      var_list.swap(fd.var_list);
    }

    std::size_t body_begin = current_token().get_position().column;
    std::size_t body_end = current_token().get_position().column;

    int bracket_stack = 0;

    if (!token_is(token_t::token_type::lcrlbracket, lexertk::parser_helper::token_advance_mode::hold))
      return {false, {}};

    for (;;)
    {
      body_end = current_token().get_position().column;

      if (token_is(token_t::token_type::lcrlbracket))
        bracket_stack++;
      else if (token_is(token_t::token_type::rcrlbracket))
      {
        if (0 == --bracket_stack)
          break;
      }
      else
      {
        if (m_current_token == m_token_list.end())
          return {false, {}};

        next_token();
      }
    }

    std::size_t size = body_end - body_begin;

    fd.body = func_def.substr(body_begin - 1, size + 1);

    const std::size_t index = body_begin + size;

    if (index < func_def.size())
      return {true, func_def.substr(index)};
    else
      return {true, {}};
  }
};

std::tuple<bool, std::string_view> parse_function_definition(std::string_view func_def, function_definition& fd)
{
  parse_function_definition_impl parser;
  return parser.process(func_def, fd);
}

void example06()
{
  constexpr static std::string_view f =
      "function foo0( ) { if (x < '}}}') { x+y; x+=1;} else {x;} }"
      "function foo1(x) { if (x < '}}}') { x+y; x+=1;} else {x;} }"
        "function foo2(x,y) { if (x < '}}}') { x+y; x+=1;} else {x;} }"
        "function foo3(x,y,z) { if (x < '}}}') { x+y; x+=1;} else {x;} }"
        "function foo4(x,y,z,w) { if (x < '}}}') { x+y; w+=1;} else {x; w;} }"
        "function foo5( ) { if (x < '}}}') { x+y; x+=1;} else {x;} }"
        "function foo6(x) { if (x < '}}}') { x+y; x+=1;} else {x;} }"
        "function foo7(x,y) { if (x < '}}}') { x+y; x+=1;} else {x;} }"
        "function foo8(x,y,z) { if (x < '}}}') { x+y; x+=1;} else {x;} }"
        "function foo9(x,y,z,w) { if (x < '}}}') { x+y; w+=1;} else {x; w;} }"
        "function foo10(x,y,z,w) { if (x < '\\'}\\'}\\'}\\'') { x+y; w+=1;} else {x; w;} }"
        "function foox( )       {  }"
        "function fooy(x)       {  }"
        "function fooz(x,y)     {  }"
        "function foow(x,y,z)   {  }"
        "function foou(x,y,z,w) {  }"
        "{xxx + yyy + zzz {k / l} }";

  function_definition fd;

  std::string_view residual{f};

  fmt::print("***** Example06 *****\n");

  int function_count = 0;

  do
  {
    if (auto [success, next] = parse_function_definition(residual, fd); success)
    {
      fmt::print("Function[{:02}]\n", function_count++);
      fmt::print("Name: {}\n", fd.name);
      fmt::print("Vars: ({})\n", fmt::join(fd.var_list, ", "));
      fmt::print("Body: \n{}\n", fd.body);
      fmt::print("-----------------------------\n\n");

      fd = {};
      residual = next;
    }
    else
      break;
  } while (!residual.empty());

  if (!residual.empty())
  {
    fmt::print("Residual '{}'\n", residual);
  }

  fmt::print("*********************\n");
}

void example07()
{
  constexpr static std::string_view expression = "#include \"lexertk.h\"";

  lexertk::generator generator{{false}};

  if (!generator.process(expression))
  {
    fmt::print("Example07 - Failed to lex: {}\n", expression);
    return;
  }
  auto list = std::move(generator).get_token_list();

  fmt::print("***** Example07 *****\n");
  dump(list);
  fmt::print("*********************\n");
}

int main()
{
  example01();
  example02();
  example03();
  example04();
  example05();
  example06();
  example07();

  return 0;
}
