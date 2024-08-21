#feature on safety

namespace std2 {

interface send { };
interface sync { };

}


struct [[unsafe::send(true),  unsafe::sync(false)]] IsSend { };
struct [[unsafe::send(false), unsafe::sync(false)]] NotSend { };

struct [[unsafe::send(false), unsafe::sync       ]] IsSync { };
struct [[unsafe::send(false), unsafe::sync(false)]] NotSync { };

// Sanity checks on the attributes.
static_assert(impl<IsSend, std2::send>);
static_assert(!impl<NotSend, std2::send>);
static_assert(impl<IsSync, std2::sync>);
static_assert(!impl<NotSync, std2::sync>);

// Builtins are send and sync.
static_assert(impl<int, std2::send>);
static_assert(impl<int, std2::sync>);

// Mutable borrows T^ are send if T is sync.
static_assert(impl<int^, std2::send>);
static_assert(impl<IsSend^, std2::send>);
static_assert(!impl<NotSend^, std2::send>);
static_assert(!impl<IsSync^, std2::send>);
static_assert(!impl<NotSync^, std2::send>);

// Shared borrows const T^ are send if T is sync.
static_assert(impl<const int^, std2::send>);
static_assert(!impl<const IsSend^, std2::send>);
static_assert(!impl<const NotSend^, std2::send>);
static_assert(impl<const IsSync^, std2::send>);
static_assert(!impl<const NotSync^, std2::send>);

// Borrows are sync if T is sync. Even for mutable borrows.
static_assert(impl<const IsSync^, std2::sync>);
static_assert(!impl<const NotSync^, std2::sync>);
static_assert(impl<IsSync^, std2::sync>);
static_assert(!impl<NotSync^, std2::sync>);

// Pointers and legacy references are send if T is a function.
static_assert(impl<void(*)(int)noexcept, std2::send>);
static_assert(impl<void(&)(int)noexcept, std2::send>);
static_assert(impl<void(&&)(int)noexcept, std2::send>);

// Pointers and legacy references are not send for any other T.
static_assert(!impl<int*, std2::send>);
static_assert(!impl<int&, std2::send>);
static_assert(!impl<int&&, std2::send>);

// Array types T[N] and [T; N] are send/sync depending on T.
static_assert(impl<IsSend[5], std2::send>);
static_assert(impl<[IsSend;5], std2::send>);
static_assert(!impl<NotSend[5], std2::send>);
static_assert(!impl<[NotSend;5], std2::send>);

static_assert(impl<IsSync[5], std2::sync>);
static_assert(impl<[IsSync;5], std2::sync>);
static_assert(!impl<NotSync[5], std2::sync>);
static_assert(!impl<[NotSync;5], std2::sync>);

// Slice types [T; dyn] are send/sync depending on T.
static_assert(impl<[IsSync; dyn], std2::sync>);
static_assert(!impl<[NotSync; dyn], std2::sync>);

// Test auto send/sync on classes.
template<typename... Ts>
struct MyClass {
  // Useng member pack expansion, but the send/sync test also looks at
  // base classes.
  Ts... members;
};

// A class is send if all its subobjects are send.
static_assert(impl<MyClass<IsSend, const int^, double>, std2::send>);
static_assert(!impl<MyClass<NotSend, const int^, double>, std2::send>);

// A class is sync if all its subobjects are send.
static_assert(impl<MyClass<IsSync, const int^, double>, std2::sync>);
static_assert(!impl<MyClass<NotSync, const int^, double>, std2::sync>);

int main(){}
