# C++11: The Complete Guide to Modern C++

> **Standard:** ISO/IEC 14882:2011
> **Predecessor:** C++03 (ISO/IEC 14882:2003)
> **Released:** August 12, 2011

C++11 was the most transformative update to C++ since its inception. Formerly known as **C++0x**, it introduced sweeping changes to both the core language and the Standard Library, fundamentally modernizing how C++ is written.

---

## Table of Contents

1. [Auto Type Deduction](#1-auto-type-deduction)
2. [Range-Based For Loop](#2-range-based-for-loop)
3. [Lambda Expressions](#3-lambda-expressions)
4. [Move Semantics & Rvalue References](#4-move-semantics--rvalue-references)
5. [Smart Pointers](#5-smart-pointers)
6. [nullptr](#6-nullptr)
7. [Uniform Initialization & Initializer Lists](#7-uniform-initialization--initializer-lists)
8. [constexpr](#8-constexpr)
9. [Static Assertions](#9-static-assertions)
10. [Strongly Typed Enums](#10-strongly-typed-enums)
11. [Variadic Templates](#11-variadic-templates)
12. [Alias Templates & Type Aliases](#12-alias-templates--type-aliases)
13. [Delegating Constructors](#13-delegating-constructors)
14. [Inheriting Constructors](#14-inheriting-constructors)
15. [Override & Final](#15-override--final)
16. [Default & Deleted Functions](#16-default--deleted-functions)
17. [Raw String Literals](#17-raw-string-literals)
18. [User-Defined Literals](#18-user-defined-literals)
19. [Threading & Concurrency Library](#19-threading--concurrency-library)
20. [Tuple & New Utility Types](#20-tuple--new-utility-types)
21. [Regular Expressions](#21-regular-expressions)
22. [New Algorithms & Container Features](#22-new-algorithms--container-features)
23. [Miscellaneous Core Language Changes](#23-miscellaneous-core-language-changes)

---

## 1. Auto Type Deduction

### What Changed

In C++03, every variable declaration required an explicit type. C++11 introduced the `auto` keyword to let the compiler deduce the type from the initializer expression.

### Syntax

```cpp
auto variable = expression;
```

### Examples

```cpp
// C++03 — verbose and error-prone
std::map<std::string, std::vector<int>>::iterator it = my_map.begin();

// C++11 — clean and safe
auto it = my_map.begin();

// Other common uses
auto x    = 42;       // int
auto pi   = 3.14;     // double
auto name = "Alice";  // const char*
auto flag = true;     // bool

// Works with function return types
auto result = std::make_pair(1, 2.0);  // std::pair<int, double>
```

### With References and Const

```cpp
const auto& ref = some_expression;  // deduced const reference
auto&&      fwd = some_expression;  // universal/forwarding reference
```

### Trailing Return Type (also C++11)

```cpp
// Useful when the return type depends on template parameters
template<typename T, typename U>
auto Add(T a, U b) -> decltype(a + b)
{
    return a + b;
}
```

> **Note:** `auto` does not deduce `const` or `&` by default. Use `const auto&` or `auto&` explicitly when needed.

---

## 2. Range-Based For Loop

### What Changed

C++11 introduced a cleaner syntax for iterating over any range — arrays, STL containers, or anything that provides `begin()` and `end()`.

### Syntax

```cpp
for (declaration : range) { body }
```

### Examples

```cpp
std::vector<int> nums = {1, 2, 3, 4, 5};

// C++03
for (std::vector<int>::iterator it = nums.begin(); it != nums.end(); ++it)
{
    std::cout << *it << "\n";
}

// C++11 — by value (copy)
for (int n : nums)
{
    std::cout << n << "\n";
}

// C++11 — by const reference (preferred for non-trivial types)
for (const auto& n : nums)
{
    std::cout << n << "\n";
}

// C++11 — by reference (to modify elements)
for (auto& n : nums)
{
    n *= 2;
}

// Works on raw arrays too
int arr[] = {10, 20, 30};
for (int x : arr)
{
    std::cout << x << "\n";
}
```

### Custom Range Support

Any class that provides `begin()` and `end()` member functions (or free functions) works automatically.

---

## 3. Lambda Expressions

### What Changed

C++11 introduced anonymous function objects (lambdas) that can capture local variables from the enclosing scope. This eliminates the need for one-off functor structs.

### Syntax

```
[ capture-list ] ( parameters ) -> return-type { body }
```

### Capture Modes

| Capture | Meaning |
|---------|---------|
| `[]` | Capture nothing |
| `[=]` | Capture all locals by value |
| `[&]` | Capture all locals by reference |
| `[x]` | Capture `x` by value |
| `[&x]` | Capture `x` by reference |
| `[=, &x]` | All by value, `x` by reference |
| `[this]` | Capture the current object |

### Examples

```cpp
// Basic lambda
auto greet = []() { std::cout << "Hello!\n"; };
greet();

// With parameters and return type
auto add = [](int a, int b) -> int { return a + b; };
std::cout << add(3, 4);  // 7

// Capturing by value
int offset     = 10;
auto add_offset = [offset](int x) { return x + offset; };

// Capturing by reference
int count     = 0;
auto increment = [&count]() { count++; };
increment();
increment();
// count == 2

// Used with STL algorithms
std::vector<int> v = {5, 2, 8, 1, 9};
std::sort(v.begin(), v.end(), [](int a, int b) { return a < b; });

// Stored in std::function
std::function<int(int, int)> multiply = [](int a, int b) { return a * b; };
```

### Mutable Lambdas

By default, captured-by-value variables are `const` inside the lambda. Use `mutable` to allow modification:

```cpp
int x   = 0;
auto f  = [x]() mutable { x++; return x; };
f();  // returns 1, but original x is still 0
```

---

## 4. Move Semantics & Rvalue References

### What Changed

This is arguably the most impactful C++11 feature. C++11 introduced **rvalue references** (`T&&`) enabling **move semantics** — transferring ownership of resources instead of copying them, dramatically improving performance.

### Rvalue References

```cpp
int&&        rref = 42;                    // rvalue reference to a temporary
std::string&& s   = std::string("hello");  // rvalue reference
```

### Move Constructor & Move Assignment

```cpp
class Buffer
{
    int*   data;
    size_t size;

public:
    // Move constructor
    Buffer(Buffer&& other) noexcept
        : data(other.data), size(other.size)
    {
        other.data = nullptr;  // Leave source in valid but empty state
        other.size = 0;
    }

    // Move assignment operator
    Buffer& operator=(Buffer&& other) noexcept
    {
        if (this != &other)
        {
            delete[] data;
            data       = other.data;
            size       = other.size;
            other.data = nullptr;
            other.size = 0;
        }
        return *this;
    }
};
```

### std::move

`std::move` casts an lvalue to an rvalue reference, signaling that it can be moved from:

```cpp
std::vector<int> a = {1, 2, 3};
std::vector<int> b = std::move(a);  // b takes ownership; a is now empty
```

### std::forward (Perfect Forwarding)

```cpp
template<typename T>
void Wrapper(T&& arg)
{
    Target(std::forward<T>(arg));  // Preserves value category
}
```

### The Rule of Five (C++11 upgrade from Rule of Three)

If you define any of these, define all five:
1. Destructor
2. Copy constructor
3. Copy assignment operator
4. **Move constructor** *(new)*
5. **Move assignment operator** *(new)*

---

## 5. Smart Pointers

### What Changed

C++11 deprecated `std::auto_ptr` and replaced it with three new smart pointer types in `<memory>`, providing safe, automatic memory management.

### std::unique_ptr

Exclusive ownership. Cannot be copied, only moved.

```cpp
#include <memory>

// Creation
std::unique_ptr<int> p(new int(42));
std::unique_ptr<int> p2(new int(10));

// Access
std::cout << *p << "\n";       // 42
std::cout << p.get() << "\n";  // raw pointer

// Transfer ownership
std::unique_ptr<int> p3 = std::move(p);  // p is now null

// Arrays
std::unique_ptr<int[]> arr(new int[10]);
```

### std::shared_ptr

Shared ownership via reference counting. Destroyed when the last owner goes out of scope.

```cpp
std::shared_ptr<int> a = std::make_shared<int>(100);
std::shared_ptr<int> b = a;  // both share ownership

std::cout << a.use_count();  // 2

b.reset();
std::cout << a.use_count();  // 1
```

### std::weak_ptr

Non-owning observer of a `shared_ptr`. Breaks circular reference cycles.

```cpp
std::shared_ptr<int> sp = std::make_shared<int>(42);
std::weak_ptr<int>   wp = sp;

if (auto locked = wp.lock())  // lock() returns shared_ptr if still alive
{
    std::cout << *locked;
}
```

### Comparison: C++03 vs C++11

| Scenario | C++03 | C++11 |
|----------|-------|-------|
| Single owner | Raw pointer + manual `delete` | `std::unique_ptr` |
| Shared owner | Manual ref counting or `boost::shared_ptr` | `std::shared_ptr` |
| Non-owning observer | Raw pointer (dangling risk) | `std::weak_ptr` |

---

## 6. nullptr

### What Changed

C++03 used `NULL` (typically defined as `0` or `(void*)0`), which caused ambiguity in overloaded functions. C++11 introduced `nullptr` — a keyword of type `std::nullptr_t` that is unambiguously a null pointer.

### Problem in C++03

```cpp
void F(int);
void F(char*);

F(NULL);  // Calls F(int)! Ambiguous and surprising.
```

### Solution in C++11

```cpp
F(nullptr);  // Unambiguously calls F(char*)
```

### Usage

```cpp
int* p = nullptr;          // preferred over NULL or 0
if (p == nullptr) { ... }

// Works with all pointer types
std::shared_ptr<int> sp = nullptr;
int (*fp)()             = nullptr;
```

---

## 7. Uniform Initialization & Initializer Lists

### What Changed

C++11 introduced **brace initialization** `{}` as a universal, consistent syntax for initializing any type — scalars, aggregates, classes, and containers.

### Syntax

```cpp
Type var { value };
Type var = { value };
```

### Examples

```cpp
// Scalars
int    x{42};
double d{3.14};

// Aggregates / structs
struct Point { int x, y; };
Point p{10, 20};

// Containers — no more repetition
std::vector<int>           v{1, 2, 3, 4, 5};
std::map<std::string, int> m{{"one", 1}, {"two", 2}};

// Class constructors
std::string s{"hello"};

// Prevents narrowing conversions (compile error)
int a{3.7};  // ERROR — double cannot be narrowed to int
int b = 3.7; // OK in C++03 (silent truncation)
```

### std::initializer_list

Enables constructors and functions to accept brace-enclosed lists:

```cpp
#include <initializer_list>

class NumberSet
{
public:
    NumberSet(std::initializer_list<int> values)
    {
        for (int v : values)
            data.push_back(v);
    }

private:
    std::vector<int> data;
};

NumberSet ns{1, 2, 3, 4, 5};  // Uses initializer_list constructor
```

---

## 8. constexpr

### What Changed

C++11 introduced `constexpr` to declare that a function or variable can be evaluated at **compile time**, enabling more powerful constant expressions than `const` alone.

### Variables

```cpp
constexpr int    k_size = 256;        // compile-time constant
constexpr double k_pi   = 3.14159265; // replaces #define PI
```

### Functions

```cpp
constexpr int Factorial(int n)
{
    return (n <= 1) ? 1 : n * Factorial(n - 1);
}

constexpr int f120 = Factorial(5);  // computed at compile time: 120
int arr[Factorial(4)];              // valid: array size from constexpr
```

### C++11 Restrictions on constexpr Functions

- Must have exactly **one return statement**
- Must not contain loops, local variables, or multiple statements
- *(These restrictions were relaxed in C++14)*

### Difference from const

| Feature | `const` | `constexpr` |
|---------|---------|-------------|
| Runtime value | ✓ | ✗ |
| Compile-time guarantee | Not always | Always |
| Use as array size | Only if initialized with constant | ✓ |

---

## 9. Static Assertions

### What Changed

C++11 introduced `static_assert` for compile-time assertions, replacing fragile template tricks or `#error` preprocessor directives.

### Syntax

```cpp
static_assert(constant-expression, "error message");
```

### Examples

```cpp
static_assert(sizeof(int)   == 4, "int must be 4 bytes on this platform");
static_assert(sizeof(void*) == 8, "Requires 64-bit platform");

template<typename T>
void Process(T value)
{
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    // ...
}

// Catches bugs at compile time
constexpr int k_buffer_size = -1;
static_assert(k_buffer_size > 0, "k_buffer_size must be positive");  // Compile error!
```

---

## 10. Strongly Typed Enums

### What Changed

C++03 `enum` values leaked into the enclosing scope and implicitly converted to integers. C++11 introduced `enum class` (scoped enums) that fix both problems.

### C++03 Problems

```cpp
enum Color { Red, Green, Blue };
enum Alert { Red, Yellow };  // ERROR: 'Red' redefined!

Color c = Red;  // unscoped
int   n = Red;  // implicit conversion — dangerous
```

### C++11 Solution: enum class

```cpp
enum class Color { Red, Green, Blue };
enum class Alert { Red, Yellow };  // OK — different scope

Color c = Color::Red;                    // must use scope
// int n = Color::Red;                   // ERROR — no implicit conversion
int n = static_cast<int>(Color::Red);    // explicit cast required
```

### Specifying the Underlying Type

```cpp
enum class Status : uint8_t { k_ok = 0, k_error = 1, k_timeout = 2 };
// Default underlying type is int
```

### Forward Declaration of Enums

```cpp
enum class Direction : int;  // forward declare (underlying type required)
```

---

## 11. Variadic Templates

### What Changed

C++11 introduced **parameter packs** — templates that accept zero or more type or non-type parameters. This enables generic functions and classes over arbitrary numbers of arguments.

### Syntax

```cpp
template<typename... Args>  // template parameter pack
void F(Args... args);       // function parameter pack
```

### Simple Example

```cpp
// Base case (empty pack)
void Print() {}

// Recursive variadic template
template<typename T, typename... Rest>
void Print(T first, Rest... rest)
{
    std::cout << first << " ";
    Print(rest...);  // expand the pack recursively
}

Print(1, "hello", 3.14, true);  // "1 hello 3.14 1"
```

### sizeof... Operator

```cpp
template<typename... Args>
void CountArgs(Args... args)
{
    std::cout << sizeof...(args) << "\n";  // number of arguments
}
```

### Real-World Use: Perfect Forwarding Factory

```cpp
template<typename T, typename... Args>
std::unique_ptr<T> Make(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
```

> **Note:** `std::make_shared`, `std::tuple`, and `std::function` all rely on variadic templates internally.

---

## 12. Alias Templates & Type Aliases

### What Changed

C++11 introduced `using` as a replacement for `typedef`, with the added ability to create **alias templates** — something `typedef` cannot do.

### Type Alias (replaces typedef)

```cpp
// C++03
typedef std::vector<int>   IntVec;
typedef unsigned long long u64;

// C++11 — cleaner, reads left-to-right
using IntVec = std::vector<int>;
using u64    = unsigned long long;

// Function pointers — much cleaner with using
typedef void (*FuncPtr)(int);  // C++03
using FuncPtr = void(*)(int);  // C++11
```

### Alias Templates (new capability)

```cpp
// Cannot do this with typedef:
template<typename T>
using Vec = std::vector<T>;

Vec<int>    vi;  // std::vector<int>
Vec<double> vd;  // std::vector<double>

// More complex alias template
template<typename K, typename V>
using HashMap = std::unordered_map<K, V>;

HashMap<std::string, int> scores;
```

---

## 13. Delegating Constructors

### What Changed

In C++03, if multiple constructors shared initialization logic, you had to call a private `Init()` helper function. C++11 allows constructors to call other constructors of the same class.

### C++03 Pattern (verbose)

```cpp
class Widget
{
    void Init(int x, int y) { /* ... */ }

public:
    Widget()             { Init(0, 0); }
    Widget(int x)        { Init(x, 0); }
    Widget(int x, int y) { Init(x, y); }
};
```

### C++11 Delegating Constructors

```cpp
class Widget
{
public:
    Widget(int x, int y) : x_(x), y_(y) { /* main logic */ }
    Widget(int x)        : Widget(x, 0) {}  // delegates to (int, int)
    Widget()             : Widget(0, 0) {}  // delegates to (int, int)

private:
    int x_, y_;
};
```

> **Rule:** Once a constructor delegates, it cannot have an initializer list for members — the target constructor handles all initialization.

---

## 14. Inheriting Constructors

### What Changed

C++11 allows a derived class to inherit the constructors of its base class with a simple `using` declaration.

### Without Inheriting Constructors (C++03)

```cpp
class Base
{
public:
    Base(int x) {}
    Base(int x, int y) {}
};

class Derived : public Base
{
public:
    Derived(int x)        : Base(x)    {}  // boilerplate
    Derived(int x, int y) : Base(x, y) {}  // boilerplate
};
```

### With Inheriting Constructors (C++11)

```cpp
class Derived : public Base
{
public:
    using Base::Base;  // inherits ALL Base constructors
};

Derived d1(5);       // uses Base(int)
Derived d2(5, 10);   // uses Base(int, int)
```

---

## 15. Override & Final

### What Changed

C++03 had no way to explicitly signal intent to override a virtual function. Typos silently created new virtual functions instead. C++11 added `override` and `final` specifiers.

### override

Forces a compile error if the function does not actually override a base class virtual function.

```cpp
class Base
{
public:
    virtual void Draw() const;
    virtual void Resize(int w, int h);
};

class Derived : public Base
{
public:
    void Draw() const override;   // OK
    void draw() const override;   // ERROR — no matching base function (typo)
    void Resize(int w) override;  // ERROR — signature mismatch
};
```

### final

Prevents further overriding or inheritance.

```cpp
class Base
{
    virtual void F() final;  // cannot be overridden in derived classes
};

class Sealed final : public Base  // cannot be subclassed
{
};

class Attempt : public Sealed {};  // ERROR — Sealed is final
```

---

## 16. Default & Deleted Functions

### What Changed

C++11 allows explicitly requesting compiler-generated defaults or completely disabling functions using `= default` and `= delete`.

### = default

Request the compiler to generate the canonical implementation:

```cpp
class MyClass
{
public:
    MyClass() = default;                           // default constructor
    MyClass(const MyClass&) = default;             // copy constructor
    MyClass& operator=(const MyClass&) = default;  // copy assignment
    MyClass(MyClass&&) = default;                  // move constructor
    MyClass& operator=(MyClass&&) = default;       // move assignment
    ~MyClass() = default;                          // destructor
};
```

### = delete

Prevent a function from being called or instantiated:

```cpp
class NonCopyable
{
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;             // no copying
    NonCopyable& operator=(const NonCopyable&) = delete;  // no copy assignment
};

NonCopyable a;
NonCopyable b = a;  // ERROR — use of deleted function
```

```cpp
// Prevent specific template instantiations
template<typename T>
void Process(T);

template<>
void Process<void*>(void*) = delete;  // disallow void* specialization

// Prevent implicit type conversions
void F(int);
void F(double) = delete;
F(3.14);  // ERROR — calls deleted F(double)
```

---

## 17. Raw String Literals

### What Changed

C++11 introduced raw string literals where escape sequences are not processed, making regex patterns, file paths, and multiline strings much easier to write.

### Syntax

```cpp
R"delimiter(content)delimiter"
```

The `delimiter` can be any sequence of up to 16 characters (may be omitted).

### Examples

```cpp
// C++03 — messy escape sequences
std::string path  = "C:\\Users\\Alice\\Documents\\file.txt";
std::string regex = "\\d{3}-\\d{4}";

// C++11 — clean, what you see is what you get
std::string path  = R"(C:\Users\Alice\Documents\file.txt)";
std::string regex = R"(\d{3}-\d{4})";

// Multiline raw string
std::string html = R"(
<html>
  <body>
    <p>Hello, world!</p>
  </body>
</html>
)";

// Custom delimiter (useful when content contains parentheses)
std::string s = R"xyz(content with ) inside)xyz";
```

---

## 18. User-Defined Literals

### What Changed

C++11 allows operators of the form `operator""_suffix` to define custom literal suffixes, enabling expressive domain-specific values.

### Defining Custom Literals

```cpp
// Length unit literals
constexpr long double operator""_km(long double d) { return d * 1000.0; }
constexpr long double operator""_m(long double d)  { return d; }
constexpr long double operator""_cm(long double d) { return d / 100.0; }

auto distance = 5.0_km + 300.0_m;  // 5300.0 meters

// Time literals
constexpr unsigned long long operator""_s(unsigned long long t)  { return t; }
constexpr unsigned long long operator""_ms(unsigned long long t) { return t / 1000; }

auto timeout = 500_ms;

// String literal
std::string operator""_str(const char* s, size_t n)
{
    return std::string(s, n);
}

auto greeting = "hello"_str;  // std::string
```

### Standard Library Literals (added in C++14, defined in C++11 infrastructure)

The `<chrono>`, `<string>`, and `<complex>` headers use this mechanism.

---

## 19. Threading & Concurrency Library

### What Changed

C++11 introduced the first standardized, portable threading library in C++, removing dependence on platform-specific APIs (POSIX threads, Windows threads).

### std::thread

```cpp
#include <thread>

void Task(int id)
{
    std::cout << "Thread " << id << " running\n";
}

std::thread t1(Task, 1);
std::thread t2(Task, 2);

t1.join();  // wait for t1 to finish
t2.join();  // wait for t2 to finish

// Detach (run independently)
std::thread t3(Task, 3);
t3.detach();
```

### std::mutex & std::lock_guard

```cpp
#include <mutex>

std::mutex g_mutex;
int        g_counter = 0;

void Increment()
{
    std::lock_guard<std::mutex> lock(g_mutex);  // RAII lock
    ++g_counter;
    // lock released automatically at end of scope
}
```

### std::condition_variable

```cpp
#include <condition_variable>

std::mutex              g_mutex;
std::condition_variable g_cv;
bool                    g_ready = false;

void Producer()
{
    std::unique_lock<std::mutex> lock(g_mutex);
    g_ready = true;
    g_cv.notify_one();
}

void Consumer()
{
    std::unique_lock<std::mutex> lock(g_mutex);
    g_cv.wait(lock, [] { return g_ready; });  // wait until ready
    // proceed
}
```

### std::future & std::async

```cpp
#include <future>

int Compute(int x) { return x * x; }

// Launch asynchronously
std::future<int> f = std::async(std::launch::async, Compute, 10);

// Do other work...

int result = f.get();  // waits and retrieves result: 100
```

### std::atomic

```cpp
#include <atomic>

std::atomic<int> g_counter{0};

void Increment()
{
    ++g_counter;  // atomic — no mutex needed for simple ops
}
```

### Concurrency Classes Summary

| Header | Class/Function | Purpose |
|--------|----------------|---------|
| `<thread>` | `std::thread` | Create/manage threads |
| `<mutex>` | `std::mutex`, `std::lock_guard`, `std::unique_lock` | Mutual exclusion |
| `<mutex>` | `std::recursive_mutex`, `std::timed_mutex` | Specialized mutexes |
| `<condition_variable>` | `std::condition_variable` | Thread synchronization |
| `<future>` | `std::future`, `std::promise`, `std::async` | Async results |
| `<atomic>` | `std::atomic<T>` | Lock-free atomic operations |

---

## 20. Tuple & New Utility Types

### std::tuple

A fixed-size collection of heterogeneous values — a generalization of `std::pair`.

```cpp
#include <tuple>

auto t = std::make_tuple(1, "hello", 3.14);

// Access by index
std::cout << std::get<0>(t);  // 1
std::cout << std::get<1>(t);  // "hello"
std::cout << std::get<2>(t);  // 3.14

// Unpack with tie
int         a;
std::string b;
double      c;
std::tie(a, b, c) = t;

// Ignore specific elements
std::tie(a, std::ignore, c) = t;

// Tuple of specific type
std::tuple<int, std::string, double> record{42, "Alice", 99.5};
```

### std::array

A fixed-size array with STL container interface:

```cpp
#include <array>

std::array<int, 5> arr{1, 2, 3, 4, 5};
arr.size();   // 5 — unlike raw arrays
arr.front();  // 1
arr.back();   // 5
arr.at(2);    // 3 — bounds-checked

// Supports algorithms
std::sort(arr.begin(), arr.end());
```

### std::unordered_map & std::unordered_set

Hash-based containers with O(1) average lookup (C++03 only had O(log n) tree-based containers):

```cpp
#include <unordered_map>
#include <unordered_set>

std::unordered_map<std::string, int> scores;
scores["Alice"] = 95;
scores["Bob"]   = 87;

std::unordered_set<int> seen{1, 2, 3, 4, 5};
seen.count(3);  // 1 (found)
seen.count(9);  // 0 (not found)
```

---

## 21. Regular Expressions

C++11 added a full regular expressions library in `<regex>`:

```cpp
#include <regex>
#include <string>

std::string text    = "My phone: 123-456-7890";
std::regex  pattern(R"(\d{3}-\d{3}-\d{4})");

// Check if match exists
bool found = std::regex_search(text, pattern);  // true

// Extract match
std::smatch match;
if (std::regex_search(text, match, pattern))
{
    std::cout << match[0];  // "123-456-7890"
}

// Full match
std::string phone = "123-456-7890";
bool        valid = std::regex_match(phone, pattern);  // true

// Replace
std::string result = std::regex_replace(text, pattern, "***-***-****");
```

---

## 22. New Algorithms & Container Features

### New Algorithms in `<algorithm>`

```cpp
// all_of, any_of, none_of
std::vector<int> v{2, 4, 6, 8};

bool all_even = std::all_of(v.begin(), v.end(),
                            [](int x) { return x % 2 == 0; });  // true
bool any_odd  = std::any_of(v.begin(), v.end(),
                            [](int x) { return x % 2 != 0; });  // false

// iota — fill with sequential values
std::vector<int> seq(10);
std::iota(seq.begin(), seq.end(), 0);  // {0,1,2,3,4,5,6,7,8,9}

// is_sorted
bool sorted = std::is_sorted(v.begin(), v.end());

// minmax
std::pair<int, int> mm = std::minmax(3, 7);
int lo = mm.first;   // 3
int hi = mm.second;  // 7

// copy_if
std::vector<int> evens;
std::copy_if(v.begin(), v.end(), std::back_inserter(evens),
             [](int x) { return x % 2 == 0; });

// move algorithm
std::move(src.begin(), src.end(), dst.begin());
```

### Container Updates

```cpp
// emplace_back — constructs in-place, no copy/move
std::vector<std::pair<int, int>> pts;
pts.emplace_back(1, 2);  // faster than push_back(std::make_pair(1, 2))

// emplace for maps
std::map<std::string, int> m;
m.emplace("key", 42);

// cbegin/cend — explicit const iterators
for (auto it = v.cbegin(); it != v.cend(); ++it) { ... }

// shrink_to_fit — hint to release excess capacity
v.shrink_to_fit();
```

---

## 23. Miscellaneous Core Language Changes

### decltype

Deduces the type of an expression without evaluating it:

```cpp
int x = 5;
decltype(x)       y = 10;  // int
decltype(x + 1.0) z = 0;   // double

// Used in trailing return types
template<typename T, typename U>
auto Multiply(T a, U b) -> decltype(a * b) { return a * b; }
```

### noexcept

Declares that a function does not throw exceptions. Enables compiler optimizations:

```cpp
void SafeOp() noexcept { /* guaranteed no exceptions */ }

// Conditional noexcept
template<typename T>
void Swap(T& a, T& b) noexcept(noexcept(T(std::move(a))))
{
    T tmp = std::move(a);
    a     = std::move(b);
    b     = std::move(tmp);
}
```

### Explicit Conversion Operators

C++03 `explicit` only applied to constructors. C++11 extends it to conversion operators:

```cpp
class SafeBool
{
public:
    explicit operator bool() const { return valid_; }

private:
    bool valid_;
};

SafeBool sb;
if (sb) { ... }      // OK — explicit bool in boolean context
bool b  = sb;        // ERROR — explicit prevents implicit conversion
bool b2 = bool(sb);  // OK — explicit cast
```

### In-Class Member Initializers

Members can now have default values directly in the class definition:

```cpp
class Config
{
    int         timeout = 30;
    std::string host    = "localhost";
    bool        verbose = false;

public:
    Config() = default;            // uses all defaults
    Config(int t) : timeout(t) {}  // overrides only timeout
};
```

### Long Long Integer Type

`long long` (minimum 64 bits) is now part of the standard:

```cpp
long long          big  = 9223372036854775807LL;   // LLONG_MAX
unsigned long long ubig = 18446744073709551615ULL;  // ULLONG_MAX
```

### New Character Types

```cpp
char16_t c16 = u'A';           // UTF-16 character
char32_t c32 = U'\U0001F600';  // UTF-32 character

std::u16string s16 = u"Hello";
std::u32string s32 = U"World";
```

---

## Summary: C++03 → C++11 at a Glance

| Area | C++03 | C++11 |
|------|-------|-------|
| Type deduction | Manual | `auto`, `decltype` |
| Loops | Iterator-based | Range-based `for` |
| Anonymous functions | Functor structs | Lambda expressions |
| Memory management | Raw pointers, `auto_ptr` | `unique_ptr`, `shared_ptr`, `weak_ptr` |
| Null pointers | `NULL` / `0` | `nullptr` |
| Initialization | Multiple inconsistent syntaxes | Uniform `{}` initialization |
| Compile-time constants | `const`, `enum` hacks | `constexpr` |
| Enums | Unscoped, implicit-convert | `enum class` — scoped, safe |
| Move/performance | Copy-only | Move semantics, rvalue refs |
| Threading | Platform-specific | `<thread>`, `<mutex>`, `<future>` |
| Templates | Fixed arity | Variadic templates |
| Assertions | `#error`, template tricks | `static_assert` |
| Type aliases | `typedef` | `using` + alias templates |

---

**Compilation flags:**
```bash
g++ -std=c++11 -Wall -Wextra source.cpp -o program
clang++ -std=c++11 -Wall -Wextra source.cpp -o program
```

---

*This document covers the C++11 standard (ISO/IEC 14882:2011) exclusively.*
