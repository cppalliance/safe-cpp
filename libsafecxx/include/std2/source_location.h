// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

/*

  source_location synopsis

namespace std2
{

class source_location
{
public:
  static constexpr
  source_location current(
    const char* file_name     = __builtin_FILE(),
    const char* function_name = __builtin_FUNCTION(),
    uint32_t    line          = __builtin_LINE(),
    uint32_t    column        = __builtin_COLUMN()) noexcept safe;

  constexpr const char* file_name(const self^) noexcept safe;
  constexpr const char* function_name(const self^) noexcept safe;
  constexpr uint32_t    line(const self^) noexcept safe;
  constexpr uint32_t    column(const self^) noexcept safe;
};

} // namespace std2

*/

namespace std2
{

class source_location
{
public:
  static constexpr
  source_location
  current(
    char const* file_name     = __builtin_FILE(),
    char const* function_name = __builtin_FUNCTION(),
    uint32_t    line          = __builtin_LINE(),
    uint32_t    column        = __builtin_COLUMN()) noexcept safe
  {
    source_location loc{};
    loc._file_name     = file_name;
    loc._function_name = function_name;
    loc._line          = line;
    loc._column        = column;
    return loc;
  }

  constexpr const char* file_name(const self^) noexcept safe {
    return self->_file_name;
  }

  constexpr const char* function_name(const self^) noexcept safe {
    return self->_function_name;
  }

  constexpr uint32_t line(const self^) noexcept safe {
    return self->_line;
  }

  constexpr uint32_t column(const self^) noexcept safe {
    return self->_column;
  }

private:
  char const* unsafe _file_name;
  char const* unsafe _function_name;
  uint32_t _line;
  uint32_t _column;
};

} // namespace std2
