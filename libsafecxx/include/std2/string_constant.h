// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

/*
  string_constant synopsis

namespace std2
{

template<class CharT>
class string_constant
{
  string_constant() = delete;

public:
  const [CharT; dyn]^/static text(self) noexcept safe;
};

} // namespace std2

*/

namespace std2
{

template<class CharT>
class string_constant
{
  const [CharT; dyn]^/static _text;

  // The compiler will provide this deleted constructor.
  // This class must live in namespace std2.
  string_constant() = delete;

public:
  const [CharT; dyn]^/static text(self) noexcept safe
  {
    return self._text;
  }
};

} // namespace std2
