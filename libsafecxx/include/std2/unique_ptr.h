// Copyright 2024 Sean Baxter
// Copyright 2024 Christian Mazakas
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#feature on safety

#include <std2/box.h>
#include <std2/optional.h>

namespace std2
{

template<class T+>
using unique_ptr = optional<box<T>>;

} // namespace std2
