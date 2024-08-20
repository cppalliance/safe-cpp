#pragma once
#feature on safety

#include <std2/box.h>
#include <std2/optional.h>

namespace std2
{

template<class T+>
using unique_ptr = optional<box<T>>;

} // namespace std2
