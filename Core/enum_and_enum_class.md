# C++ `enum class`: The Complete Guide
## From Plain `enum` Problems to Modern C++23 Techniques

> **Why this matters:** Plain `enum` has been in C since the 1970s. It carries decades of design baggage that causes real bugs in real codebases. `enum class`, introduced in C++11, fixes every one of those problems — and modern C++ keeps building on top of it.

---

## Table of Contents

1. [The Problem: Plain `enum`](#part-1-the-problem-plain-enum)
   - Implicit conversion to `int`
   - Name leaking (scope pollution)
   - No type safety
   - Unclear underlying type
   - Comparison across unrelated enums
2. [The Solution: `enum class`](#part-2-the-solution-enum-class)
   - How it works under the hood
   - Scoped names
   - Strong typing
   - Explicit underlying type
3. [C++11 Foundations](#part-3-c11-foundations)
4. [C++14 and C++17 Additions](#part-4-c14-and-c17-additions)
5. [C++20 Features](#part-5-c20-features)
6. [C++23 Features](#part-6-c23-features)
7. [Practical Patterns](#part-7-practical-patterns)
8. [Common Mistakes](#part-8-common-mistakes)
9. [Quick Reference](#quick-reference)

---

## Part 1: The Problem — Plain `enum`

Before learning `enum class`, you must understand *why* it was invented. Every feature of `enum class` is a direct fix for a specific plain `enum` failure.

---

### Problem 1: Names Leak into the Enclosing Scope

```cpp
enum color   { red, green, blue };
enum traffic { red, yellow, green };  // ❌ Compile error!
                                      // 'red' and 'green' are already declared
```

Plain `enum` injects all its enumerator names directly into the surrounding scope. This is called **namespace pollution**. If two enums in the same scope use the same name, you get a compile error — even if the enums represent completely different things.

```cpp
// Real-world collision example
enum http_status { ok, not_found, error };
enum parse_result { ok, partial, error };   // ❌ 'ok' and 'error' already defined!
```

This forced developers into ugly workarounds:

```cpp
// Workaround 1: prefix every name (verbose, error-prone)
enum color   { COLOR_RED,   COLOR_GREEN,   COLOR_BLUE  };
enum traffic { TRAFFIC_RED, TRAFFIC_YELLOW, TRAFFIC_GREEN };

// Workaround 2: wrap in a namespace (extra boilerplate)
namespace color   { enum type { red, green, blue }; }
namespace traffic { enum type { red, yellow, green }; }
color::type   c = color::red;    // works, but 'type' is awkward
traffic::type t = traffic::red;
```

Both workarounds are manual and inconsistent. `enum class` makes this automatic and clean.

---

### Problem 2: Implicit Conversion to Integer

Plain `enum` values silently convert to `int` without any cast. This is the most dangerous property of plain enums.

```cpp
enum direction { north, south, east, west };

direction d = north;
int n = d;            // ✅ Compiles silently — d becomes 0
n = d + 10;           // ✅ Compiles silently — arithmetic on enum values
d = 42;               // ✅ Compiles silently in C (warning in C++)
```

This leads to bugs that compile without a single warning:

```cpp
enum button  { ok, cancel, yes, no };
enum weekday { mon, tue, wed, thu, fri, sat, sun };

button  b = ok;
weekday w = mon;

if (b == w)           // ✅ Compiles! Both are 0, comparison "succeeds"
    launch_missile();  // Runs because ok(0) == mon(0) — completely wrong!
```

The compiler silently compares two completely unrelated enums because both convert to `int`. There is no warning by default. This is a real class of bug.

```cpp
// Another real-world danger: function overloading
void process(int code);
void process(bool flag);

enum status { ready, busy, error };

process(ready);   // Calls process(int) — int conversion happens silently
                  // Was this intentional? The compiler doesn't care.
```

---

### Problem 3: Unclear Underlying Type

What type does a plain enum use? The answer is: **it depends, and the standard doesn't fully define it.**

```cpp
enum small_flags { flag_a = 1, flag_b = 2 };   // Could be int, char, or short
enum large_value { max_id = 2147483648 };        // Could be unsigned int or long

// Serializing to binary — what size do you write?
void save(std::ostream& out, small_flags f)
{
    out.write(reinterpret_cast<const char*>(&f), sizeof(f));
    // sizeof(f) is implementation-defined — 1, 2, or 4 bytes?
    // Your binary format is now non-portable
}
```

The underlying type of a plain `enum` is implementation-defined as long as it can hold all the values. Different compilers may choose different types. This makes plain enums problematic for:

- Binary serialization / network protocols
- Bit flags requiring a specific width
- Interface with hardware registers
- `static_assert` size guarantees

---

### Problem 4: Forward Declaration is Impossible (Pre-C++11)

In C++03, you cannot forward-declare a plain `enum` because the compiler doesn't know its size without seeing its definition:

```cpp
// file_a.h
enum status;               // ❌ Error in C++03 — size unknown

void process(status s);    // Can't use it before the full definition
```

This forced the full enum definition into every header that needed to use the enum, increasing compile times and creating tight coupling.

---

### Problem 5: All Values are `int`-Compatible — No Semantic Barrier

```cpp
enum permission { read_perm = 4, write_perm = 2, exec_perm = 1 };

int file_mode = read_perm | write_perm;  // Fine if intentional
int nonsense   = read_perm * 999;        // Also compiles — is this a bug or intentional?

void open_file(int mode);   // Takes int — no way to enforce only enum values
open_file(42);              // Compiles — 42 is not a valid permission, but no error
```

---

## Part 2: The Solution — `enum class`

`enum class` (also written `enum struct` — identical behavior) was introduced in C++11 as a **scoped, strongly-typed enumeration**. It fixes every problem above.

```cpp
enum class color  { red, green, blue };
enum class traffic { red, yellow, green };  // ✅ No conflict — names are scoped
```

---

### How `enum class` Works Under the Hood

The compiler generates a new distinct type for each `enum class`. This type:

1. Has a fixed underlying type (default: `int`)
2. Has enumerators accessible only through the scope operator `::`
3. Does **not** implicitly convert to any other type
4. Does **not** accept assignment from integers without an explicit cast

At the machine level, `enum class` values are stored exactly like the underlying integer type. There is **zero runtime overhead**. The entire safety mechanism is enforced at compile time.

```
enum class direction : uint8_t { north, south, east, west };

Memory layout: 1 byte (same as uint8_t)
Values:        north = 0x00, south = 0x01, east = 0x02, west = 0x03
Type:          direction (not uint8_t — they are distinct types)
```

---

### Fix 1: Scoped Names — No More Pollution

```cpp
enum class color   { red, green, blue };
enum class traffic { red, yellow, green };  // ✅ Both compile fine

color   c = color::red;      // Must use scope
traffic t = traffic::red;    // Completely separate — no conflict

// c == t would be a compile error — they are different types
```

---

### Fix 2: No Implicit Conversion — Bugs Become Errors

```cpp
enum class button  { ok, cancel };
enum class weekday { mon, tue };

button  b = button::ok;
weekday w = weekday::mon;

if (b == w)          // ❌ Compile error — different types, cannot compare
    launch_missile();

int n = b;           // ❌ Compile error — no implicit conversion to int
n = b + 1;           // ❌ Compile error — no arithmetic on enum class directly

// To get the integer value, you must be explicit
int n = static_cast<int>(b);   // ✅ Intentional conversion — clearly stated
```

The principle: **if a bug compiles, it will eventually reach production. If it doesn't compile, it never will.**

---

### Fix 3: Explicit Underlying Type — Portable and Predictable

```cpp
enum class status : uint8_t  { ok, error, pending };   // Always 1 byte
enum class flags  : uint32_t { none = 0, read = 1, write = 2, exec = 4 };
enum class id     : uint64_t { invalid = 0 };           // Full 64-bit range

static_assert(sizeof(status) == 1);   // ✅ Guaranteed
static_assert(sizeof(flags)  == 4);   // ✅ Guaranteed
```

---

### Fix 4: Forward Declaration Works

```cpp
// file_a.h — forward declare with the underlying type
enum class status : int;   // ✅ Size is known (it's int)

void process(status s);    // Can declare functions using it

// file_b.cpp — full definition elsewhere
enum class status : int { ok = 0, error = 1, pending = 2 };
```

This reduces compile-time dependencies and circular include issues.

---

## Part 3: C++11 Foundations

### Basic Syntax

```cpp
// Basic enum class
enum class direction { north, south, east, west };

// With explicit underlying type
enum class http_code : uint16_t
{
    ok            = 200,
    not_found     = 404,
    server_error  = 500
};

// Access always requires scope
direction d    = direction::north;
http_code code = http_code::ok;
```

### Default Values

Enumerators start at 0 and increment by 1, just like plain `enum`:

```cpp
enum class weekday { mon, tue, wed, thu, fri, sat, sun };
//                    0    1    2    3    4    5    6

enum class priority { low = 1, medium, high, critical };
//                      1        2      3       4
```

### Explicit Cast Required

```cpp
enum class color : int { red = 0, green = 1, blue = 2 };

// From enum class to int
int value = static_cast<int>(color::green);   // value = 1

// From int to enum class
color c = static_cast<color>(2);   // c = color::blue

// Danger: casting out-of-range value is implementation-defined
color bad = static_cast<color>(99);   // Compiles, but undefined if value not in enum
```

### `switch` with `enum class`

```cpp
enum class season { spring, summer, autumn, winter };

void describe(season s)
{
    switch (s)
    {
        case season::spring: std::cout << "Warm\n";  break;
        case season::summer: std::cout << "Hot\n";   break;
        case season::autumn: std::cout << "Cool\n";  break;
        case season::winter: std::cout << "Cold\n";  break;
    }
    // No default needed — compiler warns about missing cases with -Wswitch
}
```

**Tip:** Omitting `default` and enabling `-Wswitch` (GCC/Clang) or `/W4` (MSVC) means the compiler warns you whenever you add a new enumerator but forget to handle it. This is valuable.

### Bit Flags with `enum class`

Plain `enum` was often used for bit flags. `enum class` doesn't allow arithmetic by default, so you write explicit operators:

```cpp
enum class permission : uint8_t
{
    none    = 0b0000,
    read    = 0b0001,
    write   = 0b0010,
    exec    = 0b0100,
    all     = 0b0111
};

// Define operators for flag usage
constexpr permission operator|(permission a, permission b)
{
    return static_cast<permission>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
    );
}

constexpr permission operator&(permission a, permission b)
{
    return static_cast<permission>(
        static_cast<uint8_t>(a) & static_cast<uint8_t>(b)
    );
}

constexpr bool has_flag(permission set, permission flag)
{
    return (set & flag) == flag;
}

// Usage
permission file_perms = permission::read | permission::write;
bool can_read = has_flag(file_perms, permission::read);   // true
bool can_exec = has_flag(file_perms, permission::exec);   // false
```

This is more verbose than plain `enum` bit flags, but the verbosity is the point — every flag operation is explicit and intentional.

---

## Part 4: C++14 and C++17 Additions

### C++14: `constexpr` Functions with `enum class`

C++14 relaxed `constexpr` rules significantly, enabling real logic in constant expressions involving `enum class`:

```cpp
enum class log_level : int { trace = 0, debug = 1, info = 2, warn = 3, error = 4 };

// C++14: constexpr function with local variables and if/else
constexpr const char* to_string(log_level level)
{
    switch (level)
    {
        case log_level::trace: return "TRACE";
        case log_level::debug: return "DEBUG";
        case log_level::info:  return "INFO";
        case log_level::warn:  return "WARN";
        case log_level::error: return "ERROR";
    }
    return "UNKNOWN";
}

// Evaluated entirely at compile time
constexpr const char* level_name = to_string(log_level::warn);  // "WARN" at compile time
```

### C++17: `if constexpr` with `enum class`

```cpp
enum class output_format { json, xml, csv };

template <output_format fmt>
std::string serialize(const record& r)
{
    if constexpr (fmt == output_format::json)
        return to_json(r);
    else if constexpr (fmt == output_format::xml)
        return to_xml(r);
    else
        return to_csv(r);
    // Unreachable branches are discarded at compile time — not just optimized away,
    // but never compiled. Type errors in dead branches don't matter.
}

// Usage — the format is a compile-time constant
auto json_output = serialize<output_format::json>(my_record);
```

### C++17: Structured Bindings and `enum class` in Maps

```cpp
enum class error_code : int { ok = 0, timeout = 1, refused = 2, unknown = 99 };

std::map<error_code, std::string_view> error_messages
{
    { error_code::ok,      "Success"          },
    { error_code::timeout, "Connection timed out" },
    { error_code::refused, "Connection refused"   }
};

for (const auto& [code, message] : error_messages)
    std::cout << static_cast<int>(code) << ": " << message << '\n';
```

---

## Part 5: C++20 Features

### `using enum` — Scoped Access Without Repetition

C++20 introduces `using enum` to import enumerator names into a local scope. This eliminates repetitive `enum_name::` prefixes without losing type safety.

```cpp
enum class card_suit { clubs, diamonds, hearts, spades };

// Without using enum
std::string suit_name(card_suit suit)
{
    switch (suit)
    {
        case card_suit::clubs:    return "Clubs";
        case card_suit::diamonds: return "Diamonds";
        case card_suit::hearts:   return "Hearts";
        case card_suit::spades:   return "Spades";
    }
}

// With using enum — much cleaner inside the switch
std::string suit_name(card_suit suit)
{
    using enum card_suit;
    switch (suit)
    {
        case clubs:    return "Clubs";
        case diamonds: return "Diamonds";
        case hearts:   return "Hearts";
        case spades:   return "Spades";
    }
}
```

`using enum` is **scoped** — the names are only available inside the block where it appears. The type safety is preserved; you haven't made them global.

```cpp
// using enum in class scope (clean interface design)
class traffic_controller
{
public:
    enum class state { idle, running, paused, stopped };
    using enum state;   // Members can use idle, running, etc. directly

    void start()   { current_state_ = running; }
    void pause()   { current_state_ = paused;  }
    void stop()    { current_state_ = stopped; }

    bool is_active() const { return current_state_ == running; }

private:
    state current_state_ = idle;
};
```

### Concepts Constraining `enum class` Types

```cpp
#include <type_traits>

// Concept: accepts only enum types
template <typename T>
concept enumeration = std::is_enum_v<T>;

// Concept: accepts only scoped enums (enum class)
template <typename T>
concept scoped_enum = std::is_scoped_enum_v<T>;   // C++23, but approximated in C++20:

template <typename T>
concept scoped_enum_20 = std::is_enum_v<T> &&
                         !std::is_convertible_v<T, std::underlying_type_t<T>>;

// Usage: function that only accepts enum class types
template <scoped_enum_20 E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

enum class color : uint8_t { red, green, blue };
enum plain_dir { north, south };  // plain enum

auto v1 = to_underlying(color::red);   // ✅ Works
auto v2 = to_underlying(north);        // ❌ Compile error — not a scoped enum
```

### `constexpr` Everywhere

In C++20, `constexpr` is much more powerful. Enums inside `constexpr` contexts work seamlessly:

```cpp
enum class direction : int { north = 0, east = 1, south = 2, west = 3 };

constexpr direction rotate_clockwise(direction d)
{
    return static_cast<direction>((static_cast<int>(d) + 1) % 4);
}

constexpr direction turn_around(direction d)
{
    return static_cast<direction>((static_cast<int>(d) + 2) % 4);
}

static_assert(rotate_clockwise(direction::north) == direction::east);
static_assert(turn_around(direction::north)      == direction::south);
// Both assertions checked at compile time — zero runtime cost
```

---

## Part 6: C++23 Features

### `std::to_underlying()` — Official Conversion Helper

C++23 adds a standard function to replace the verbose `static_cast<std::underlying_type_t<E>>(e)` pattern:

```cpp
#include <utility>

enum class color : uint8_t { red = 0, green = 1, blue = 2 };

// C++11 / C++17 way — verbose
uint8_t v1 = static_cast<std::underlying_type_t<color>>(color::green);  // 1

// C++23 way — clean
uint8_t v2 = std::to_underlying(color::green);  // 1
```

This is one of those small additions that makes code significantly cleaner in practice. Any time you need the raw integer value of an `enum class`, `std::to_underlying` is the idiomatic C++23 way.

```cpp
// Practical use: serialization
void write_enum(std::ostream& out, color c)
{
    auto raw = std::to_underlying(c);   // uint8_t
    out.write(reinterpret_cast<const char*>(&raw), sizeof(raw));
}

// Practical use: array indexing
constexpr std::string_view color_names[] = { "red", "green", "blue" };

std::string_view name_of(color c)
{
    return color_names[std::to_underlying(c)];
}
```

### `std::is_scoped_enum` — Type Trait

C++23 adds `std::is_scoped_enum<T>` to distinguish `enum class` from plain `enum`:

```cpp
#include <type_traits>

enum class scoped_color { red, green, blue };
enum plain_dir { north, south };

static_assert(std::is_scoped_enum_v<scoped_color>);   // ✅ true
static_assert(!std::is_scoped_enum_v<plain_dir>);     // ✅ true — plain enum is NOT scoped

// Useful in template metaprogramming
template <typename T>
    requires std::is_scoped_enum_v<T>
void safe_serialize(T value)
{
    auto raw = std::to_underlying(value);
    // ...
}
```

---

## Part 7: Practical Patterns

### Pattern 1: `to_string` and `from_string` for `enum class`

A common need: convert between `enum class` values and human-readable strings.

```cpp
enum class log_level : int { trace, debug, info, warn, error };

// To string
constexpr std::string_view to_string(log_level level) noexcept
{
    using enum log_level;
    switch (level)
    {
        case trace: return "trace";
        case debug: return "debug";
        case info:  return "info";
        case warn:  return "warn";
        case error: return "error";
    }
    return "unknown";
}

// From string — returns optional to handle invalid input cleanly
std::optional<log_level> from_string(std::string_view str) noexcept
{
    if (str == "trace") return log_level::trace;
    if (str == "debug") return log_level::debug;
    if (str == "info")  return log_level::info;
    if (str == "warn")  return log_level::warn;
    if (str == "error") return log_level::error;
    return std::nullopt;   // Unknown string
}

// Usage
auto level = from_string("warn");
if (level)
    std::cout << "Parsed: " << to_string(*level) << '\n';  // "warn"
```

### Pattern 2: `enum class` as Strong Typedef

Use `enum class` with a single value to create a distinct type that wraps a primitive — preventing mixing up semantically different integers.

```cpp
// Problem: both are int — easy to mix up
void create_user(int user_id, int company_id, int role_id);

create_user(company_id, user_id, role_id);   // ❌ Wrong order — compiles!

// Solution: strong typedefs via enum class
enum class user_id    : int {};
enum class company_id : int {};
enum class role_id    : int {};

void create_user(user_id uid, company_id cid, role_id rid);

create_user(company_id{1}, user_id{2}, role_id{3});   // ❌ Compile error — type mismatch
create_user(user_id{2}, company_id{1}, role_id{3});   // ✅ Correct order enforced
```

This pattern uses `enum class` with no enumerators — just as a type wrapper. Values are created by direct initialization: `user_id{42}`.

### Pattern 3: Sentinel Values with `enum class`

```cpp
enum class search_result : int
{
    not_found = -1,   // Explicit sentinel
    // Actual results are non-negative indices
};

search_result find_item(const std::vector<std::string>& items, std::string_view target)
{
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
        if (items[i] == target)
            return static_cast<search_result>(i);
    return search_result::not_found;
}

auto result = find_item(items, "banana");
if (result == search_result::not_found)
    std::cerr << "Not found\n";
else
    std::cout << "Index: " << std::to_underlying(result) << '\n';
```

### Pattern 4: Compile-Time Enum Validation

```cpp
enum class http_method : uint8_t
{
    get     = 0,
    post    = 1,
    put     = 2,
    del     = 3,
    patch   = 4,
    count_  = 5    // Sentinel: one past the last valid value
};

constexpr bool is_valid_method(http_method m)
{
    return std::to_underlying(m) < std::to_underlying(http_method::count_);
}

static_assert(is_valid_method(http_method::get));    // ✅
static_assert(!is_valid_method(static_cast<http_method>(99)));  // ✅
```

### Pattern 5: Enum-Indexed Arrays

```cpp
enum class planet : size_t
{
    mercury = 0, venus, earth, mars,
    jupiter, saturn, uranus, neptune,
    count_
};

constexpr double orbital_period_days[] =
{
    88.0,    // mercury
    224.7,   // venus
    365.25,  // earth
    687.0,   // mars
    4331.0,  // jupiter
    10747.0, // saturn
    30589.0, // uranus
    59800.0  // neptune
};

static_assert(std::size(orbital_period_days) == std::to_underlying(planet::count_));

constexpr double get_period(planet p)
{
    return orbital_period_days[std::to_underlying(p)];
}

static_assert(get_period(planet::earth) == 365.25);   // Compile-time check
```

### Pattern 6: State Machines

`enum class` is ideal for state machine states — the type safety prevents accidentally assigning an unrelated value as the current state:

```cpp
enum class connection_state
{
    disconnected,
    connecting,
    connected,
    reconnecting,
    error
};

class tcp_connection
{
public:
    void connect()
    {
        if (state_ != connection_state::disconnected)
            return;
        state_ = connection_state::connecting;
        do_connect();
    }

    void on_connected()   { state_ = connection_state::connected;    }
    void on_error()       { state_ = connection_state::error;        }
    void disconnect()     { state_ = connection_state::disconnected; }

    bool is_connected() const
    {
        return state_ == connection_state::connected;
    }

private:
    connection_state state_ = connection_state::disconnected;
    void do_connect() { /* ... */ }
};
```

---

## Part 8: Common Mistakes

### Mistake 1: Forgetting the Scope Prefix

```cpp
enum class color { red, green, blue };

color c = red;          // ❌ Compile error — 'red' is not in scope
color c = color::red;   // ✅
```

### Mistake 2: Arithmetic Without Cast

```cpp
enum class score : int { low = 1, mid = 5, high = 10 };

int total = score::high + score::mid;   // ❌ Compile error — no operator+
int total = static_cast<int>(score::high) + static_cast<int>(score::mid);  // ✅
// Or in C++23:
int total = std::to_underlying(score::high) + std::to_underlying(score::mid);  // ✅
```

### Mistake 3: Comparing `enum class` with Raw Integer

```cpp
enum class priority : int { low = 0, medium = 1, high = 2 };

priority p = priority::high;

if (p == 2)                          // ❌ Compile error
if (p == static_cast<priority>(2))   // ✅ But fragile — use named value
if (p == priority::high)             // ✅ Best — semantic and type-safe
```

### Mistake 4: Using `enum class` for Bit Flags Without Operators

```cpp
enum class flag : uint8_t { a = 1, b = 2, c = 4 };

// ❌ Doesn't compile — no operator| defined
flag combined = flag::a | flag::b;

// ✅ Define operators, or use a flag wrapper library
constexpr flag operator|(flag x, flag y)
{
    return static_cast<flag>(
        static_cast<uint8_t>(x) | static_cast<uint8_t>(y)
    );
}
flag combined = flag::a | flag::b;   // ✅ Now works
```

### Mistake 5: Casting Arbitrary Integers to `enum class`

```cpp
enum class direction : int { north = 0, south = 1, east = 2, west = 3 };

int user_input = get_user_input();

// Dangerous: value might be 5, 99, -1...
direction d = static_cast<direction>(user_input);   // ❌ No validation

// Safe: validate before casting
direction parse_direction(int value)
{
    if (value < 0 || value > 3)
        throw std::invalid_argument("Invalid direction");
    return static_cast<direction>(value);
}
```

### Mistake 6: Plain `enum` vs `enum class` Confusion

```cpp
// Plain enum — names are in the enclosing scope
enum plain { alpha, beta };
auto x = alpha;   // ✅ Works — name is in global scope

// enum class — names are scoped
enum class scoped { alpha, beta };
auto y = alpha;          // ❌ 'alpha' not found
auto y = scoped::alpha;  // ✅
```

---

## Quick Reference

### Syntax Comparison

```cpp
// Plain enum (C++03 and earlier)
enum color { red, green, blue };
int value = red;       // Implicit conversion — compiles
color c = 0;           // Allowed in C, warned in C++

// enum class (C++11+) — scoped and strongly typed
enum class color { red, green, blue };
color c   = color::red;                  // Must use scope
int value = static_cast<int>(color::red); // Must cast explicitly
int value = std::to_underlying(color::red); // C++23 idiom
```

### Feature Timeline

| Feature | Standard | Example |
|---|---|---|
| `enum class` basic | C++11 | `enum class color { red };` |
| Explicit underlying type | C++11 | `enum class flags : uint8_t { ... };` |
| Forward declaration | C++11 | `enum class status : int;` |
| `constexpr` enums | C++11/14 | `constexpr color c = color::red;` |
| `if constexpr` with enums | C++17 | `if constexpr (fmt == format::json)` |
| `using enum` | C++20 | `using enum card_suit;` |
| `std::is_scoped_enum` | C++23 | `std::is_scoped_enum_v<color>` |
| `std::to_underlying` | C++23 | `std::to_underlying(color::red)` |

### When to Use What

| Situation | Recommendation |
|---|---|
| Simple named constants, no arithmetic | `enum class` |
| Bit flags | `enum class` + explicit bitwise operators |
| State machine states | `enum class` |
| Strong typedef for int parameters | `enum class` with no enumerators |
| C API interop requiring int | Plain `enum` or `enum class` + cast |
| Need implicit int conversion | Plain `enum` (use rarely, document why) |
| Compile-time dispatch | `enum class` + `if constexpr` |

### The Conversion Cheat Sheet

```cpp
enum class color : uint8_t { red = 0, green = 1, blue = 2 };

// enum class → underlying type
uint8_t v = static_cast<uint8_t>(color::red);      // C++11
uint8_t v = std::to_underlying(color::red);         // C++23 ✅

// underlying type → enum class (validate first!)
color c = static_cast<color>(1u);                   // color::green

// enum class → string (you write this)
constexpr std::string_view to_string(color c);

// string → enum class (you write this)
std::optional<color> from_string(std::string_view s);

// enum class in switch
switch (c) { case color::red: ... }

// enum class with using enum (C++20)
using enum color;
switch (c) { case red: ... }    // No prefix needed inside this scope
```

---

## Summary

Plain `enum` was a convenience from C that brought five serious problems into C++: name pollution, implicit integer conversion, unspecified underlying type, impossible forward declarations, and no semantic barrier between unrelated values.

`enum class` solves all five problems with a single design decision: **make the type a first-class named type with explicit, opt-in conversions**.

The cost is slightly more typing (you must write `color::red` instead of `red`, and `static_cast` when you need the integer). The benefit is an entire class of bugs that simply cannot compile.

Modern C++ keeps building on `enum class`:

- **C++14/17** made `constexpr` powerful enough to do real work with enum values at compile time
- **C++20's `using enum`** eliminated the repetitive prefix in contexts where it was just noise
- **C++23's `std::to_underlying` and `std::is_scoped_enum`** made the remaining boilerplate disappear

```
The rule is simple:
  Default to enum class.
  Use plain enum only when interfacing with C APIs that require it,
  and document that choice explicitly.
```

---

*Based on the ISO C++ Standard (C++11 through C++23), cppreference.com, and the C++ Core Guidelines (Enum.3, Enum.5, Enum.7, Enum.8).*
