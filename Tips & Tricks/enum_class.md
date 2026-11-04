# C++ Enums: `enum class`, Underlying Types, and Doing It Right

> C++ is not a bad language. It gives you sharp tools and trusts you to use them on purpose. Enums are a perfect example: used well they are type-safe, free at runtime, and self-documenting. Used carelessly (casting random ints into them, treating them as bools) they will bite you. This guide shows how to use them well.

> **Language level:** the examples use modern C++: C++20 features (concepts, `std::same_as`, `using enum`, `<=>`, ranges) and C++23's `std::to_underlying`. Compile with `-std=c++23`. A C++20-only fallback for `to_underlying` is in [section 4](#4-getting-the-value-out-casting-properly).

## Table of Contents

1. [What is wrong with plain `enum`](#1-what-is-wrong-with-plain-enum)
2. [What `enum class` solves](#2-what-enum-class-solves)
3. [The underlying type](#3-the-underlying-type)
4. [Getting the value out: casting properly](#4-getting-the-value-out-casting-properly)
5. [`to_underlying` costs nothing at runtime](#5-to_underlying-costs-nothing-at-runtime)
6. [Comparing and switching](#6-comparing-and-switching)
7. [Use case: bit flags](#7-use-case-bit-flags)
8. [Use case: lookup tables and indexing](#8-use-case-lookup-tables-and-indexing)
9. [Enum to string (no reflection yet)](#9-enum-to-string-no-reflection-yet)
10. [Pitfalls and how to avoid them](#10-pitfalls-and-how-to-avoid-them)
11. [Best-practice checklist](#11-best-practice-checklist)
12. [Under the hood: how `enum class` works](#12-under-the-hood-how-enum-class-works)
13. [What to avoid: do not fight the type system](#13-what-to-avoid-do-not-fight-the-type-system)

---

## 1. What is wrong with plain `enum`

```cpp
enum Color { Red, Green, Blue };
enum Fruit { Apple, Orange };
```

Three root problems:

**1. Names leak into the enclosing scope.**

```cpp
enum Light { Red, Yellow };   // error: 'Red' redeclared
int Green = 5;                // error: conflicts with Color::Green
```

**2. Implicit conversion to integers.** The enum is "just a named int" in disguise.

```cpp
int x = Red;                  // compiles
if (Red == Apple) { }         // compiles (!), compares two unrelated concepts
int y = Green + Blue;         // compiles, meaningless
```

**3. The underlying type is implementation-defined and hidden.** You do not know whether it is `int`, `unsigned`, or `char`, and (without a fixed type) you cannot forward-declare it.

---

## 2. What `enum class` solves

```cpp
enum class e_Color { Red, Green, Blue };
enum class e_Fruit { Apple, Orange };
```

| Problem                    | Plain `enum`        | `enum class`                          |
| -------------------------- | ------------------- | ------------------------------------- |
| Name pollution             | Leaks into scope    | Scoped: `e_Color::Red`                |
| Implicit conversion to int | Yes                 | **No**, needs an explicit cast        |
| Comparing different enums  | Compiles            | **Compile error**                     |
| Underlying type            | Implementation-defined | Always fixed (default `int`)       |
| Forward declaration        | Only with fixed type | Always possible                      |

```cpp
e_Color color = e_Color::Red;
// int x = color;                  // error: no implicit conversion
// if (color == e_Fruit::Apple) {} // error: different types
```

The compiler now catches a whole category of bugs for free.

---

## 3. The underlying type

Every `enum class` has an **underlying type**: the integer type used to store its values.

- Default is `int`.
- You can choose any integral type with `: type`.

```cpp
enum class e_Test { First = 20, Second, Third };
// First = 20, Second = 21, Third = 22  (each enumerator is previous + 1)
// underlying type: int  (because we did not specify one)

enum class e_Perm : unsigned
{
    None  = 0,
    Read  = 1,
    Write = 2,
    Exec  = 4
};

enum class e_Small : std::uint8_t { A = 1, B = 2 };   // exactly 1 byte
```

Rules worth knowing:

- Without an initializer, the first enumerator is `0`; every later one is *previous + 1*.
- Every enumerator value must **fit** in the underlying type:

```cpp
enum class e_Bad : std::uint8_t { Big = 300 };   // error: narrowing, 300 > 255
```

- You can check the type at compile time:

```cpp
#include <concepts>

static_assert(std::same_as<std::underlying_type_t<e_Test>, int>);
static_assert(std::same_as<std::underlying_type_t<e_Perm>, unsigned>);
static_assert(sizeof(e_Small) == 1);
```

`std::same_as<A, B>` (C++20) reads like the sentence it expresses and replaces the older `std::is_same_v<A, B>`.

- A fixed underlying type means you can **forward-declare**, which keeps headers light and speeds up builds:

```cpp
// header.h
enum class e_Color : int;          // declaration only
void Paint(e_Color color);
```

**When to choose a type explicitly:**

- Bit flags: use an `unsigned` type (never signed, see [pitfalls](#10-pitfalls-and-how-to-avoid-them)).
- Data sent over the network, stored on disk, or in packed structs: use a fixed-width type such as `std::uint8_t`.
- Otherwise the default `int` is fine.

---

## 4. Getting the value out: casting properly

Because `enum class` refuses implicit conversion, you must ask explicitly. There are three ways, from worst to best.

```cpp
e_Test test = e_Test::First;

// 1. Hard-coded type: works, but fragile
int a = static_cast<int>(test);

// 2. Correct for any underlying type (C++11)
auto b = static_cast<std::underlying_type_t<e_Test>>(test);

// 3. Best: C++23, <utility>
auto c = std::to_underlying(test);
```

**Why `static_cast<int>` is the weakest choice (root cause):** it hard-codes an assumption. If someone later changes the enum to `: std::uint8_t` or `: std::int64_t`, your `static_cast<int>` still compiles and silently truncates or widens. `std::to_underlying` and `std::underlying_type_t` always follow the declared type, so they can never go out of sync.

**C++20 fallback**, a drop-in that behaves identically. It uses a small concept so that passing a non-enum is a clear compile error:

```cpp
#include <type_traits>

template <typename T>
concept Enumeration = std::is_enum_v<T>;

template <Enumeration E>
constexpr std::underlying_type_t<E> ToUnderlying(E value) noexcept
{
    return static_cast<std::underlying_type_t<E>>(value);
}
```

This is literally what the standard library does. `std::to_underlying` is just this function, standardised. The `Enumeration` concept is reused later in this guide.

---

## 5. `to_underlying` costs nothing at runtime

`std::to_underlying(x)` is a `constexpr` function whose entire body is one `static_cast`. An enum and its underlying type have the **same object representation**, so the conversion needs no instruction: no arithmetic, no memory access, no branch.

- With optimization on (`-O1` and above), the function is inlined and disappears. The generated machine code is identical to using the raw integer.
- At `-O0` (no optimization) it may exist as a tiny function call, because nothing gets inlined at that level. That is a debug-build artifact, not a property of the language.
- It is `constexpr`, so it works wherever a compile-time constant is needed:

```cpp
std::array<int, std::to_underlying(e_Test::Third)> table{};   // array size
static_assert(std::to_underlying(e_Perm::Exec) == 4);
```

So there is never a performance reason to prefer a hand-written cast over `std::to_underlying`. The named function is just as fast and communicates intent.

---

## 6. Comparing and switching

You do **not** need to convert to an integer to compare. Compare enumerators directly:

```cpp
e_Test test = e_Test::Third;

if (test == e_Test::Third) { /* ... */ }   // fine
if (test != e_Test::First) { /* ... */ }   // fine
if (test <  e_Test::Third) { /* ... */ }   // fine: same type, compares by value
```

Since C++20, the three-way comparison `<=>` also works between two values of the same enum type:

```cpp
if ((e_Test::First <=> e_Test::Third) == std::strong_ordering::less) { /* ... */ }
```

What does *not* compile (and that is a feature):

```cpp
// if (test == 22) {}               // error: enum vs int
// if (test == e_Perm::Read) {}     // error: different enum types
```

If you really need to compare against a raw number (for example a value from a file), convert the **number** into the enum at the boundary and validate it (see [section 10](#10-pitfalls-and-how-to-avoid-them)), rather than converting the enum down to an int everywhere.

### `switch`: let the compiler check you

```cpp
std::string_view Describe(e_Test test)
{
    switch (test)
    {
        case e_Test::First:  return "first";
        case e_Test::Second: return "second";
        case e_Test::Third:  return "third";
    }
    return "unknown";   // reachable only if test holds an invalid value
}
```

Do **not** add `default:` when you cover every enumerator. Without `default`, `-Wswitch` (included in `-Wall`) warns you the moment someone adds a new enumerator and forgets to handle it. With `default`, the warning is silenced and the bug hides.

### C++20: `using enum` removes the repetition

Inside a small scope you can bring the enumerators in, so a long `switch` stays readable but you keep the type checks:

```cpp
std::string_view Describe(e_Test test)
{
    switch (test)
    {
        using enum e_Test;       // C++20: names are visible only inside this block
        case First:  return "first";
        case Second: return "second";
        case Third:  return "third";
    }
    return "unknown";
}
```

The names are not leaked into the surrounding scope. You are choosing, locally and on purpose, to drop the `e_Test::` prefix.

---

## 7. Use case: bit flags

`enum class` forbids `|` and `&` by default, so you opt in explicitly. This is the intended design: *you* decide which operations make sense.

With C++20 concepts you write the operators **once** and switch them on per enum with a one-line opt-in:

```cpp
enum class e_Perm : unsigned
{
    None  = 0,
    Read  = 1,
    Write = 2,
    Exec  = 4
};

// Opt-in switch: off for every enum unless you specialise it
template <typename E>
inline constexpr bool enable_flags = false;

template <typename E>
concept FlagEnum = Enumeration<E> && enable_flags<E>;      // Enumeration: see section 4

template <FlagEnum E>
constexpr E operator|(E lhs, E rhs)
{
    return static_cast<E>(std::to_underlying(lhs) | std::to_underlying(rhs));
}
template <FlagEnum E>
constexpr E operator&(E lhs, E rhs)
{
    return static_cast<E>(std::to_underlying(lhs) & std::to_underlying(rhs));
}
template <FlagEnum E>
constexpr E& operator|=(E& lhs, E rhs)
{
    return lhs = lhs | rhs;
}
template <FlagEnum E>
constexpr bool HasFlag(E value, E flag)
{
    return (value & flag) == flag;
}

// The one line that turns the operators on for e_Perm
template <>
inline constexpr bool enable_flags<e_Perm> = true;
```

Why a trait and a concept instead of overloads for each enum:

- The operators exist **only** for enums that opt in. An ordinary enum such as `e_Color` still rejects `|`, so you keep the safety where it matters.
- A new flag enum needs one extra line, not four copied functions.
- Constraining with a concept gives a short, readable error ("constraints not satisfied: `FlagEnum`") instead of a wall of template noise.
- Put the specialisation right after the enum, before anything uses the operators.

Usage:

```cpp
e_Perm perm = e_Perm::Read | e_Perm::Write;
perm |= e_Perm::Exec;

if (HasFlag(perm, e_Perm::Write)) { /* can write */ }
if ((perm & e_Perm::Exec) != e_Perm::None) { /* can execute */ }
```

Notes:

- Use an **unsigned** underlying type. Bitwise operations on signed integers have pitfalls (sign bit, implementation behaviour of `~` and shifts).
- Each flag must be a distinct power of two (`1, 2, 4, 8, ...`), or use `1u << n`.
- Never write `if (perm)`. It does not compile, and the right question is *which* flag you are testing.

---

## 8. Use case: lookup tables and indexing

When enumerators are consecutive (`0, 1, 2, ...`), they make a type-safe index:

```cpp
enum class e_Color : std::uint8_t { Red, Green, Blue };

constexpr std::array<std::uint32_t, 3> color_codes{ 0xFF0000, 0x00FF00, 0x0000FF };

constexpr std::uint32_t CodeOf(e_Color color)
{
    return color_codes[std::to_underlying(color)];
}
```

Because `to_underlying` is free, this compiles down to a plain array access.

Other good uses of `enum class`:

- **States**: `e_ConnectionState { Idle, Connecting, Connected, Closed }`
- **Error/result codes**: `e_ParseResult { Ok, Empty, BadFormat }`
- **Options instead of boolean parameters**: `Save(file, e_Overwrite::Yes)` is much clearer than `Save(file, true)`
- **Message / opcode types** in protocols (with a fixed-width underlying type)
- **Keys** for `switch`-based dispatch

---

## 9. Enum to string (no reflection yet)

C++ has no standard `to_string` for enums. The compiler knows the names, but the language cannot give them to your code, because there is no static reflection yet. Reflection is being standardised for C++26; use the technique below until your toolchain supports it.

The cleanest zero-overhead trick is the **X-macro**: list the enumerators **once**, and generate the enum, the string conversion, and more from that one list. It cannot get out of sync.

```cpp
#include <array>
#include <optional>
#include <string_view>

// 1. The single source of truth
#define COLOR_LIST(X) \
    X(Red)            \
    X(Green)          \
    X(Blue)

// 2. Generate the enum
enum class e_Color
{
#define X(name) name,
    COLOR_LIST(X)
#undef X
};

// 3. Generate enum -> string
constexpr std::string_view ToString(e_Color color)
{
    switch (color)
    {
#define X(name) case e_Color::name: return #name;
        COLOR_LIST(X)
#undef X
    }
    return "Unknown";
}

// 4. Generate string -> enum
constexpr std::optional<e_Color> FromString(std::string_view text)
{
#define X(name) if (text == #name) return e_Color::name;
    COLOR_LIST(X)
#undef X
    return std::nullopt;
}

// 5. Generate "all values" for iteration
inline constexpr std::array all_colors
{
#define X(name) e_Color::name,
    COLOR_LIST(X)
#undef X
};
```

How it works, in plain words:

- `COLOR_LIST(X)` calls whatever macro you pass as `X` once per entry.
- `#name` is the preprocessor **stringizing operator**: it turns the token `Red` into the string literal `"Red"`.
- Each time, you `#define X` to mean something different, expand the list, then `#undef X` so it cannot leak.

Usage:

```cpp
e_Color color = e_Color::Green;
std::cout << ToString(color) << '\n';            // Green

if (auto parsed = FromString("Blue"))
{
    // *parsed == e_Color::Blue
}

for (e_Color each : all_colors)
{
    std::cout << ToString(each) << '\n';         // Red, Green, Blue
}
```

**Why there is no overhead:**

- The strings are literals stored in read-only data, created at compile time.
- `ToString` is `constexpr`, so it can be evaluated at compile time. At runtime it is a `switch`, which compilers turn into a jump table or lookup.
- `std::string_view` is a pointer plus a length. No allocation, no copying.

**With explicit values** (flags, protocol codes), carry the value in the list:

```cpp
#define PERM_LIST(X) \
    X(None,  0)      \
    X(Read,  1)      \
    X(Write, 2)      \
    X(Exec,  4)

enum class e_Perm : unsigned
{
#define X(name, value) name = value,
    PERM_LIST(X)
#undef X
};

constexpr std::string_view ToString(e_Perm perm)
{
    switch (perm)
    {
#define X(name, value) case e_Perm::name: return #name;
        PERM_LIST(X)
#undef X
    }
    return "Unknown";
}
```

Macro hygiene rules:

- Always `#undef X` right after use.
- Give list macros specific names (`COLOR_LIST`, not `LIST`).
- Keep the macro in the same header as the enum.

**No macros at all?** A hand-written `switch` (like `Describe` in section 6) is equally fast. It is just more typing, and you must update it when the enum changes. Compile with `-Wall` and the missing-case warning will remind you.

---

## 10. Pitfalls and how to avoid them

### 10.1 Casting an arbitrary integer into an enum

```cpp
auto color = static_cast<e_Color>(42);   // compiles!
```

What actually happens (precise version):

- For an `enum class` (always a fixed underlying type), `42` is a **valid value of the type** even though no enumerator has that value. It is not undefined behaviour, but it is a *bug in waiting*: your `switch` handles no case for it, tables indexed by it go out of bounds, `ToString` returns `"Unknown"`.
- For an old **unscoped enum without a fixed type**, a value outside the enum's range is genuinely **undefined behaviour** (since C++17).

Root cause: the language lets you say "trust me, this int is a Color". The fix is not to cast blindly. **Validate at the boundary**, where untrusted data enters your program (files, network, user input):

```cpp
constexpr bool IsValid(e_Color color)
{
    switch (color)
    {
        case e_Color::Red:
        case e_Color::Green:
        case e_Color::Blue:
            return true;
    }
    return false;
}

std::optional<e_Color> ColorFromInt(int raw)
{
    auto color = static_cast<e_Color>(raw);
    if (!IsValid(color)) return std::nullopt;
    return color;
}
```

If you already have the X-macro list from section 9, C++20 ranges give you the same check without a second `switch` to maintain:

```cpp
constexpr bool IsValid(e_Color color)
{
    return std::ranges::find(all_colors, color) != all_colors.end();
}
```

It is a linear search, which is fine for small enums at a boundary. Use the `switch` version for hot paths.

Inside your program, a valid `e_Color` should be the only kind that exists, so you never need to re-check.

### 10.2 Arithmetic and `++` on enums

```cpp
// auto next = color + 1;     // error (good!)
// ++color;                   // error (good!)
```

Do not "fix" this by casting to int, adding, and casting back. That produces values outside the enum. If you need "the next one", write a function that wraps explicitly:

```cpp
constexpr e_Color Next(e_Color color)
{
    switch (color)
    {
        case e_Color::Red:   return e_Color::Green;
        case e_Color::Green: return e_Color::Blue;
        case e_Color::Blue:  return e_Color::Red;
    }
    return color;
}
```

### 10.3 Letting values drift when they are persisted

Auto-numbered enumerators depend on order. Insert a new one in the middle and every later value shifts, silently corrupting saved files and network protocols.

```cpp
// Anything stored or transmitted: pin the numbers
enum class e_Opcode : std::uint8_t
{
    Ping  = 1,
    Pong  = 2,
    Data  = 3,    // add new ones at the END, never renumber
};
```

### 10.4 `default:` hiding missing cases

Covered in section 6. Prefer no `default` for enums you fully handle, and compile with `-Wall -Wextra` (add `-Werror=switch` to make it a hard error).

### 10.5 Signed underlying types for flags

`~` and shifts on signed integers are a trap. Use `unsigned` (or `std::uint32_t`) as the underlying type for flag enums.

### 10.6 Values that do not fit

```cpp
enum class e_Tiny : std::uint8_t { Max = 256 };   // error
```

The compiler catches this, so pick the underlying type deliberately.

### 10.7 Sentinel `Count` enumerators

Adding `Count` at the end for array sizing is common, but `Count` becomes a fake valid value that every `switch` must handle. Prefer `all_colors.size()` from the X-macro list, or `std::to_underlying(e_Color::Blue) + 1` in one well-named constant.

### 10.8 Brace-initialising from an integer

Since C++17, `e_Color color{42};` also compiles for enums with a fixed underlying type. Same bug as 10.1, same fix: validate first.

---

## 11. Best-practice checklist

- Use `enum class` by default. Reach for plain `enum` only when you need implicit conversion on purpose (rare).
- Specify the underlying type when size or signedness matters (flags, protocols, storage, forward declarations).
- Read the value with `std::to_underlying` (C++23) or `std::underlying_type_t` (older standards). Never hard-code `static_cast<int>`.
- Compare enumerators to enumerators. Do not compare against integer literals.
- Pin explicit values for anything that is saved or sent.
- Validate any integer before turning it into an enum.
- Switch without `default`, with warnings on.
- Opt in to operators (`|`, `&`) only where they make sense: write them once as `constexpr` templates constrained by a `FlagEnum` concept, then enable them per enum.
- Prefer `std::same_as` and other concepts over `std::is_same_v` and `enable_if` for readable constraints and checks.
- Generate names with an X-macro (single source of truth) until reflection arrives.
- Prefer enums over `bool` parameters and magic numbers: `Open(path, e_Mode::ReadOnly)` documents itself.

---

## 12. Under the hood: how `enum class` works

The short version: **an `enum class` is a compile-time idea.** The compiler uses it to check your code, then throws the idea away. By the time machine code exists, an enum value is just an integer. Scoping, "no implicit conversion", and "different enums cannot be mixed" are all rules the compiler enforces and then erases.

> The measurements below come from `g++ -std=c++23 -O2` on x86-64 Linux. Exact instructions vary by compiler and target; the structure does not.

### 12.1 What the compiler does with `e_Color::Green`

```cpp
enum class e_Color { Red, Green, Blue };
```

| Stage                | What happens                                                                                                      |
| -------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Parsing              | `e_Color` is registered as a **new, distinct type**. `Red`, `Green`, `Blue` are registered as named constants *inside that type's scope* (that is why you must write `e_Color::Green`). |
| Semantic analysis    | Values are assigned (`0, 1, 2`), the underlying type is fixed (`int`), and every use is type-checked: scope lookup, conversion rules, overload resolution. |
| Code generation      | `e_Color::Green` is replaced by the literal `1`. A variable of type `e_Color` becomes a plain `int`.              |
| Runtime              | **Nothing.** No enum object, no table, no vtable, no names.                                                       |

Closest mental model: a **strong typedef**, an `int` the type system refuses to mix with other ints, plus named constants that live inside the type's scope. Unlike a hand-written wrapper struct, it is literally just an `int` in the generated code.

### 12.2 What a value looks like in memory

A variable of enum type has exactly the size and alignment of its underlying type. There is no extra header or tag.

```cpp
enum class e_Color { Red, Green, Blue };                  // int
enum class e_Wide  : std::uint64_t { A = 1ull << 40 };    // 8 bytes
enum class e_Tiny  : std::uint8_t  { A };                 // 1 byte
enum OldColor { OldRed, OldGreen };                       // plain enum, no fixed type

sizeof(e_Color)   // 4
sizeof(e_Wide)    // 8
sizeof(e_Tiny)    // 1
sizeof(OldColor)  // 4  (implementation-defined for a plain enum without a fixed type)
```

A flag value is just bits in that integer:

```
e_Perm perm = e_Perm::Read | e_Perm::Write;     // underlying: unsigned (4 bytes)

 bit:   31 ...... 3   2      1       0
              0 ... 0  0      1       1
                      Exec   Write   Read        ->  value 3
```

### 12.3 Enumerators are constants, not objects

- They occupy no storage and have no address: `&e_Color::Red` does not compile.
- They are constant expressions, so they work in `static_assert`, array sizes, template arguments, and `case` labels.
- Duplicate values are allowed (an alias such as `Default = First`).
- Each value must be a constant expression that fits the underlying type. If you give none, it is *previous + 1* (first is `0`).

### 12.4 Distinct type, identical machine code

Take two overloads that differ only by enum type, plus one that takes a plain `int`:

```cpp
int Paint(e_Color color) { return static_cast<int>(color) * 3 + 1; }
int Paint(e_Fruit fruit) { return static_cast<int>(fruit) * 3 + 1; }
int PaintInt(int value)  { return value * 3 + 1; }
```

The compiler keeps them apart using the type name in the **mangled symbol**:

```
_Z5Paint7e_Color     <- Paint(e_Color)
_Z5Paint7e_Fruit     <- Paint(e_Fruit)
_Z8PaintInti         <- PaintInt(int)
```

But the instructions inside `Paint(e_Color)` and `PaintInt(int)` are identical:

```asm
endbr64
leal    1(%rdi,%rdi,2), %eax     ; value * 3 + 1
ret
```

The type exists for the compiler and for the linker's symbol names. The CPU never sees it. This is the whole trick: **type safety is a compile-time layer on top of ordinary integers.**

### 12.5 Why there is no implicit conversion

This is a language rule, not magic:

- A plain `enum` has an *integral promotion* to `int`, so it quietly becomes a number whenever one is expected.
- An `enum class` has **no standard conversion** to any arithmetic type. That one missing rule is the entire difference.

Which operators exist:

- Built-in `==`, `!=`, `<`, `>`, `<=`, `>=` (and `<=>` in C++20) work between two values of the **same** enum type.
- `+`, `-`, `|`, `&` need arithmetic operands, so they do not exist for an `enum class`. That is why section 7 defines them yourself.
- `static_cast<underlying>(value)` is the one door. It is an ordinary integral conversion between types of the same representation, so it emits no instructions (section 5).
- `std::underlying_type_t<E>` asks the compiler directly. It is a compiler built-in, not something you could write in plain C++.

### 12.6 How the underlying type is chosen

| Declaration                          | Underlying type                          | Valid values                                      |
| ------------------------------------ | ---------------------------------------- | ------------------------------------------------- |
| `enum class e_A { ... }`             | `int` (default)                          | **Every** value of `int`                          |
| `enum class e_B : std::uint8_t {...}`| What you wrote                           | **Every** value of `std::uint8_t`                 |
| `enum e_C { ... }` (plain, no type)  | Chosen by the compiler (implementation-defined) | Only the range needed to hold the enumerators |

That last row is why section 10.1 says an out-of-range cast is true undefined behaviour for a plain enum, but only a logic bug for an `enum class`: the `enum class` already *defines* all of its integer's values as valid, even the ones nobody named.

### 12.7 What exists at runtime, and what does not

I searched the compiled program for the enumerator names:

- **Normal build:** zero matches for `Red`, `Green`, `Blue`, or `e_Color`. The names do not exist in the executable.
- **Build with `-g`:** the names appear, but in the debug information (DWARF). That is how a debugger prints `Green` instead of `1`. Those sections are not loaded into memory when the program runs normally.

So the language gives you **no names at runtime**. If you want them, you must supply them yourself, which is exactly what the X-macro in section 9 does, with string literals placed in read-only data.

### 12.8 How a `switch` on an enum compiles

```cpp
int Score(e_Color color)
{
    switch (color)
    {
        case e_Color::Red:   return 10;
        case e_Color::Green: return 25;
        case e_Color::Blue:  return 7;
    }
    return -1;
}
```

GCC turned this into a range check plus one table lookup (no chain of `if`s):

```asm
movl    $-1, %eax            ; default result: -1
cmpl    $2, %edi             ; is value > 2 ?
ja      .L5                  ;   yes -> return -1
movl    %edi, %edi
leaq    CSWTCH.5(%rip), %rax ; address of table {10, 25, 7}
movl    (%rax,%rdi,4), %eax  ; result = table[value]
.L5:
ret
```

Two takeaways:

1. Small, consecutive enumerators make the lookup `O(1)`. Sparse values (like flags `1, 2, 4, 8`) may compile to compares or a binary search; the compiler decides.
2. The `cmpl $2` / `ja` pair is your `return -1` after the `switch`. It is the code that runs when someone cast an invalid integer into the enum (section 10.1). The compiler had to keep it because it cannot prove nobody did that.

### 12.9 The whole picture

```
You write                   Compiler knows (compile time)              CPU sees (runtime)
-------------------------   ----------------------------------------   ------------------
e_Color::Green              constant of type e_Color, value 1          the integer 1
e_Color color;              variable of distinct type, int-sized       4 bytes
color == e_Color::Red       built-in ==, both sides same type          cmp  value, 0
std::to_underlying(color)   static_cast<int>(color)                    (nothing)
color == e_Fruit::Apple     ERROR: different types                     (never generated)
int x = color;              ERROR: no implicit conversion              (never generated)
```

Everything in the middle column is checked and then erased. That is why `enum class` gives you strong safety at zero runtime cost.

---

## 13. What to avoid: do not fight the type system

`enum class` deliberately makes conversions explicit so that the dangerous ones look dangerous. If you find yourself writing any of these, stop and ask what you are really trying to do:

```cpp
// BAD: enum to bool. Which value means "true"? What does "None" mean?
bool is_set = static_cast<bool>(e_Perm::Write);

// BAD: enum to an unrelated type
float ratio = static_cast<float>(e_Color::Green);

// BAD: reinterpreting memory to get around the rules
int& raw = reinterpret_cast<int&>(color);   // undefined behaviour territory

// BAD: casting random numbers into an enum "because it compiles"
auto color = static_cast<e_Color>(user_input);

// BAD: casting everywhere instead of comparing enumerators
if (static_cast<int>(color) == 1) { }
```

And the honest replacements:

```cpp
// GOOD: ask the real question
bool can_write = HasFlag(perm, e_Perm::Write);

// GOOD: compare enumerators
if (color == e_Color::Green) { }

// GOOD: validate, then use
if (auto parsed = ColorFromInt(user_input)) { Use(*parsed); }
```

The pattern behind every item on the bad list is the same: someone silenced the compiler instead of listening to it. C++ never forces you to do this. The explicit-cast syntax is there so that when you genuinely need a conversion (for example `to_underlying` for an array index or a wire format), it is visible, searchable, and intentional.

C++ is not the problem. Intentionally bypassing the type system is. Write the enum, let the compiler guard it, and cast only when you have a reason you could explain to a colleague.
