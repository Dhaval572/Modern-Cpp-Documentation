# Ultimate Features of C++20

Welcome to the comprehensive guide to **C++20** features! This release is one of the most significant updates to C++ since C++11, introducing powerful abstractions and modernizing the language.

---

## Language Features

### 1. Concepts

```cpp
template<typename T>
concept Integral = std::is_integral_v<T>;

template<Integral T>
T add(T a, T b)
{
    return a + b;
}

// More complex concepts
template<typename T>
concept Addable = requires(T a, T b)
{
    { a + b } -> std::convertible_to<T>;
};
```

**Purpose**: Constraints on template parameters that produce clear, readable requirements and dramatically improved error messages. Instead of cryptic template instantiation errors, you get human-readable constraint violations.

### 2. Ranges

```cpp
#include <ranges>
#include <vector>

std::vector<int> vec = {1, 2, 3, 4, 5, 6};
auto even = [](int i) { return i % 2 == 0; };

// Composable views - no intermediate allocations
auto result = vec | std::views::filter(even) 
                  | std::views::transform([](int i) { return i * 2; });

for (auto i : result)
{ 
    std::cout << i << " "; // 4 8 12
}
```

**Purpose**: Lazy-evaluated, composable algorithms over sequences. Views are non-owning and can be chained with the pipe operator (`|`) for elegant, efficient data processing pipelines.

### 3. Coroutines

```cpp
#include <coroutine>
#include <iostream>

struct Generator
{
    struct promise_type
    {
        int current_value;
        
        Generator get_return_object()
        {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        
        std::suspend_always yield_value(int value)
        {
            current_value = value;
            return {};
        }
        
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
    
    std::coroutine_handle<promise_type> handle;
    
    ~Generator() { if (handle) handle.destroy(); }
    
    bool move_next()
    { 
        handle.resume(); 
        return !handle.done(); 
    }
    
    int current_value() { return handle.promise().current_value; }
};

Generator counter()
{
    for (int i = 0; i < 3; ++i)
    {
        co_yield i;
    }
}
```

**Purpose**: Enable functions that can suspend execution and resume later. Perfect for generators, async operations, and state machines. Uses `co_await`, `co_yield`, and `co_return` keywords.

### 4. Modules

```cpp
// math.ixx (module interface)
export module math;

export int add(int a, int b)
{ 
    return a + b; 
}

export namespace algebra
{
    int multiply(int a, int b) { return a * b; }
}

// main.cpp
import math;
import <iostream>; // Can also import standard headers

int main()
{
    std::cout << add(2, 3); // 5
}
```

**Purpose**: Replace the preprocessor-based `#include` system with true modular compilation. Benefits include faster compilation, better encapsulation, no header guards needed, and protection from macro pollution.

### 5. Three-Way Comparison (Spaceship Operator)

```cpp
#include <compare>

struct Point
{
    int x, y;
    
    // Compiler generates all six comparison operators
    auto operator<=>(const Point&) const = default;
    
    // Equivalently, for custom logic:
    // auto operator<=>(const Point& other) const
    // {
    //     if (auto cmp = x <=> other.x; cmp != 0) return cmp;
    //     return y <=> other.y;
    // }
};

Point p1{1, 2}, p2{1, 3};
bool less = p1 < p2;  // true
bool equal = p1 == p2; // false (also auto-generated)
```

**Purpose**: Single operator (`<=>`) generates all six comparison operators (`<`, `<=`, `>`, `>=`, `==`, `!=`). Returns `std::strong_ordering`, `std::weak_ordering`, or `std::partial_ordering`.

### 6. Designated Initializers

```cpp
struct Config
{
    int width = 800;
    int height = 600;
    bool fullscreen = false;
    const char* title = "App";
};

// Initialize only specific members by name
Config cfg = {
    .width = 1920,
    .height = 1080,
    .fullscreen = true
    // title uses default value
};
```

**Purpose**: Initialize aggregate members by name, making initialization clearer and more maintainable. Order must match declaration order.

### 7. `consteval` - Immediate Functions

```cpp
consteval int square(int n)
{
    return n * n;
}

int main()
{
    constexpr int a = square(5); // OK: compile-time
    int x = 5;
    // int b = square(x);  // ERROR: must be compile-time evaluable
}
```

**Purpose**: `consteval` functions **must** be evaluated at compile-time, unlike `constexpr` which *can* be. Guarantees zero runtime overhead.

### 8. Expanded `constexpr`

```cpp
#include <vector>
#include <algorithm>

constexpr int compute()
{
    std::vector<int> v = {3, 1, 4, 1, 5}; // Allowed in constexpr now!
    std::sort(v.begin(), v.end());
    return v[0];
}

constexpr int min_value = compute(); // Evaluated at compile-time
```

**Purpose**: `constexpr` functions can now use dynamic allocation, virtual functions, `try-catch`, and more. Most standard library algorithms are now `constexpr`.

### 9. Template Lambdas

```cpp
// Explicit template parameters in lambdas
auto add = []<typename T>(T a, T b)
{ 
    return a + b; 
};

auto sum = []<typename... Ts>(Ts... args)
{
    return (args + ...); // Fold expression
};

int i = add(5, 3);           // T = int
double d = add(1.5, 2.5);    // T = double
int total = sum(1, 2, 3, 4); // 10
```

**Purpose**: Explicit template syntax for lambdas enables better type control, perfect forwarding, and variadic templates in lambda expressions.

### 10. Lambdas in Unevaluated Contexts

```cpp
#include <map>

// Lambda can be used in decltype, sizeof, typeid, etc.
auto lambda = [](int x) { return x * 2; };
using LambdaType = decltype(lambda);

std::map<int, decltype(lambda)> map;
```

**Purpose**: Lambdas can appear in type contexts where they're not executed, enabling them as template arguments and in type computations.

### 11. Default Member Initializers for Bit-Fields

```cpp
struct Flags
{
    unsigned int flag1 : 1 = 0;  // Now allowed!
    unsigned int flag2 : 1 = 1;
    unsigned int value : 6 = 42;
};
```

**Purpose**: Bit-fields can have default member initializers, making struct initialization more convenient.

### 12. `constinit`

```cpp
constinit int global = 42; // Guaranteed initialized at compile-time

constinit thread_local int counter = 0; // Thread-local with compile-time init
```

**Purpose**: Guarantees static/thread-local variables are initialized at compile-time, avoiding the "static initialization order fiasco" without making the variable `const`.

### 13. `[[likely]]` and `[[unlikely]]` Attributes

```cpp
int process(int value)
{
    if (value > 0) [[likely]]
    {
        return value * 2;
    }
    else [[unlikely]]
    {
        return expensive_error_handling();
    }
}
```

**Purpose**: Hint to the compiler about branch probability for better optimization and code layout.

### 14. `[[no_unique_address]]` Attribute

```cpp
struct Empty {};

struct Data
{
    [[no_unique_address]] Empty e;
    int value;
};

// sizeof(Data) == sizeof(int), not sizeof(int) + sizeof(Empty)
```

**Purpose**: Allows empty base optimization for members, enabling zero-overhead policy-based design and type traits as members.

### 15. Aggregate Initialization with Parentheses

```cpp
struct Point { int x, y; };

Point p1(1, 2);  // Now valid! Previously only Point p1{1, 2};
```

**Purpose**: Aggregates can be initialized with parentheses, not just braces, providing more uniform initialization syntax.

### 16. Class Types in Non-Type Template Parameters (NTTP)

```cpp
struct Point { int x, y; };

template<Point P>
struct Widget
{
    static constexpr int x = P.x;
    static constexpr int y = P.y;
};

Widget<Point{10, 20}> w;  // Class literal as template parameter!
```

**Purpose**: Classes with simple structure can be used as non-type template parameters, enabling powerful compile-time computations with structured data.

### 17. `using enum` Declarations

```cpp
enum class Color { Red, Green, Blue };

void draw()
{
    using enum Color;  // Bring all enumerators into scope
    
    Color c = Red;   // No need for Color::Red
    if (c == Green) { /* ... */ }
}
```

**Purpose**: Import all enumerators from an enumeration into the current scope, reducing verbosity when working with scoped enums.

### 18. Conditional `explicit`

```cpp
template<typename T>
struct Wrapper
{
    T value;
    
    // Conditionally explicit based on type property
    explicit(!std::is_convertible_v<T, int>) 
    Wrapper(T v) : value(v) {}
};

Wrapper<int> w1 = 5;     // OK: implicit conversion
// Wrapper<std::string> w2 = "hi"; // ERROR: explicit constructor
```

**Purpose**: Make constructors/conversion operators conditionally explicit based on compile-time conditions.

### 19. Signed Integer Type for `char8_t`

```cpp
char8_t utf8_char = u8'A';  // UTF-8 character type
const char8_t* utf8_string = u8"Hello, 世界";
```

**Purpose**: Distinct type for UTF-8 characters, separate from `char` and `unsigned char`, ensuring type safety for UTF-8 text processing.

### 20. Feature Test Macros

```cpp
#ifdef __cpp_concepts
    // Use concepts
#else
    // Fallback implementation
#endif

#if __cpp_lib_ranges >= 201911L
    // Use ranges library
#endif
```

**Purpose**: Standardized macros to detect availability of specific C++20 features, enabling portable feature detection.

---

## Standard Library Features

### 1. `std::span`

```cpp
#include <span>
#include <vector>

void process(std::span<const int> data)
{
    // Works with any contiguous container
    for (int val : data) { /* ... */ }
}

std::vector<int> vec = {1, 2, 3, 4, 5};
int arr[] = {6, 7, 8};

process(vec);           // Works with vector
process(arr);           // Works with array
process({vec.begin(), 3}); // First 3 elements
```

**Purpose**: Non-owning view over contiguous sequences. Replaces pointer+size pairs with a safer, more convenient abstraction. No allocation or copying.

### 2. `std::format`

```cpp
#include <format>
#include <string>

std::string msg = std::format("Hello, {}!", "World");
std::string precise = std::format("Pi: {:.2f}", 3.14159); // "Pi: 3.14"
std::string hex = std::format("0x{:08X}", 255); // "0x000000FF"

// Custom types
struct Point { int x, y; };

template<>
struct std::formatter<Point>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    
    auto format(const Point& p, format_context& ctx) const
    {
        return format_to(ctx.out(), "({}, {})", p.x, p.y);
    }
};
```

**Purpose**: Type-safe, extensible string formatting similar to Python's format strings. Replaces `printf` and iostreams for formatting, with compile-time format string checking.

### 3. `std::ranges` Library

```cpp
#include <ranges>
#include <algorithm>
#include <vector>

std::vector<int> vec = {5, 2, 8, 1, 9};

// Algorithm works directly with container
std::ranges::sort(vec);

// Projections
struct Person { std::string name; int age; };
std::vector<Person> people = {{"Alice", 30}, {"Bob", 25}};
std::ranges::sort(people, {}, &Person::age); // Sort by age
```

**Purpose**: Range-based versions of all standard algorithms. Support projections, work directly with containers, and integrate seamlessly with range views.

### 4. `std::chrono` Extensions (Calendars and Time Zones)

```cpp
#include <chrono>
using namespace std::chrono;

// Calendar operations
auto today = year_month_day{floor<days>(system_clock::now())};
auto birthday = 2024y/March/15;
auto days_until = sys_days{birthday} - sys_days{today};

// Time zones
auto now = system_clock::now();
auto london = zoned_time{"Europe/London", now};
auto tokyo = zoned_time{"Asia/Tokyo", now};

std::cout << format("{:%Y-%m-%d %H:%M:%S %Z}\n", london);
```

**Purpose**: Complete calendar and time zone support. Type-safe date arithmetic, time zone conversions, and formatting.

### 5. `std::jthread` and `std::stop_token`

```cpp
#include <thread>
#include <stop_token>
#include <chrono>

void worker(std::stop_token stoken)
{
    while (!stoken.stop_requested())
    {
        // Do work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main()
{
    std::jthread thread(worker); // Automatically joins
    std::this_thread::sleep_for(std::chrono::seconds(1));
    thread.request_stop(); // Cooperative cancellation
    // thread auto-joins on destruction
}
```

**Purpose**: `std::jthread` automatically joins on destruction. `std::stop_token` provides cooperative cancellation mechanism for threads.

### 6. `std::bit_cast`

```cpp
#include <bit>
#include <cstdint>

float f = 3.14f;
uint32_t bits = std::bit_cast<uint32_t>(f); // Safe reinterpret

// Can cast between types of same size
struct RGB { uint8_t r, g, b, a; };
uint32_t color = 0xFF00FFAA;
RGB rgb = std::bit_cast<RGB>(color);
```

**Purpose**: Safe, constexpr-capable reinterpretation of bit patterns. Replaces `reinterpret_cast` and `memcpy` for type punning.

### 7. `std::source_location`

```cpp
#include <source_location>

void log(std::string_view message,
         const std::source_location location = 
             std::source_location::current())
{
    std::cout << location.file_name() << ':'
              << location.line() << " - "
              << message << '\n';
}

log("Error occurred"); // Prints: file.cpp:42 - Error occurred
```

**Purpose**: Captures caller's source location (file, line, column, function). Better than `__FILE__` and `__LINE__` macros because it works correctly with default arguments.

### 8. Math Constants

```cpp
#include <numbers>

double circle_area = std::numbers::pi * radius * radius;
double e_value = std::numbers::e;
double phi = std::numbers::phi; // Golden ratio
double sqrt2 = std::numbers::sqrt2;
```

**Purpose**: Standard mathematical constants with full precision as constexpr values. Available for any floating-point type.

### 9. `std::atomic_ref`

```cpp
#include <atomic>

void increment(int& value)
{
    std::atomic_ref<int> atomic_value(value);
    atomic_value.fetch_add(1, std::memory_order_relaxed);
}

int counter = 0;
increment(counter); // Thread-safe increment of non-atomic variable
```

**Purpose**: Provides atomic operations on non-atomic objects. Useful when you can't change the type but need atomic access.

### 10. `std::atomic` Enhancements

```cpp
#include <atomic>

std::atomic<float> atomic_float = 1.0f; // Now supported!
std::atomic<double> atomic_double = 2.0;

atomic_float.fetch_add(0.5f);

// Waiting operations
std::atomic<int> flag = 0;
flag.wait(0); // Block until value changes from 0
flag.store(1);
flag.notify_one(); // Wake one waiting thread
```

**Purpose**: Atomic operations on floating-point types and new wait/notify operations for efficient thread synchronization without spinning.

### 11. `std::barrier` and `std::latch`

```cpp
#include <barrier>
#include <latch>

// Latch - single-use counter
std::latch work_done(3);
// ... threads call work_done.count_down() ...
work_done.wait(); // Wait for count to reach 0

// Barrier - reusable synchronization point
std::barrier sync_point(3, []() noexcept
{
    std::cout << "All threads reached barrier\n";
});

// In thread:
sync_point.arrive_and_wait(); // Synchronize multiple times
```

**Purpose**: `std::latch` is a single-use countdown synchronization. `std::barrier` is a reusable thread rendezvous point with optional completion function.

### 12. `std::counting_semaphore` and `std::binary_semaphore`

```cpp
#include <semaphore>

std::counting_semaphore<10> pool_slots(10); // Max 10 resources

void use_resource()
{
    pool_slots.acquire(); // Wait for available slot
    // Use resource
    pool_slots.release(); // Return slot
}

std::binary_semaphore signal(0); // Like a simplified mutex
signal.release(); // Signal
signal.acquire(); // Wait
```

**Purpose**: Standard semaphore primitives for resource counting and signaling between threads.

### 13. `std::make_shared` for Arrays

```cpp
#include <memory>

// Create shared array
auto arr = std::make_shared<int[]>(10);
arr[0] = 42;

auto arr2 = std::make_shared<int[10]>(); // Fixed size
```

**Purpose**: `std::make_shared` now supports array types with proper array deletion.

### 14. String Prefix/Suffix Operations

```cpp
#include <string>

std::string str = "Hello, World!";

bool starts = str.starts_with("Hello");  // true
bool ends = str.ends_with("World!");     // true
bool has_sub = str.contains("lo, Wo");   // true (C++23 actually)

// Also works with string_view
std::string_view sv = "test.txt";
bool is_txt = sv.ends_with(".txt");
```

**Purpose**: Convenient member functions for common string operations without manual substring comparisons.

### 15. `std::ssize` and Related

```cpp
#include <vector>
#include <iterator>

std::vector<int> vec = {1, 2, 3};

auto signed_size = std::ssize(vec); // Returns ptrdiff_t (signed)
// Useful for loops: for (auto i = std::ssize(vec) - 1; i >= 0; --i)
```

**Purpose**: Returns signed size, avoiding unsigned integer underflow issues in loop conditions.

### 16. `std::to_array`

```cpp
#include <array>

auto arr = std::to_array({1, 2, 3, 4, 5}); // std::array<int, 5>
auto carr = std::to_array("hello"); // std::array<char, 6> including '\0'

int raw[] = {1, 2, 3};
auto std_arr = std::to_array(raw); // Convert C array to std::array
```

**Purpose**: Create `std::array` from braced initializer or C-style array with automatic size and type deduction.

### 17. `std::erase` and `std::erase_if`

```cpp
#include <vector>
#include <algorithm>

std::vector<int> vec = {1, 2, 3, 2, 4, 2, 5};

std::erase(vec, 2);  // Remove all 2s
// vec is now {1, 3, 4, 5}

std::erase_if(vec, [](int x) { return x % 2 == 0; }); // Remove evens
// vec is now {1, 3, 5}
```

**Purpose**: Simplified element removal combining erase-remove idiom into single function call. Works with all standard containers.

### 18. Constexpr Containers and Algorithms

```cpp
#include <vector>
#include <algorithm>

constexpr int compile_time_sort()
{
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6};
    std::sort(v.begin(), v.end());
    return v[0]; // Returns 1
}

constexpr int min_val = compile_time_sort(); // All at compile-time!
```

**Purpose**: Most standard containers (`vector`, `string`) and algorithms can now be used in `constexpr` contexts for compile-time computations.

---

## Utility Enhancements

### 1. `std::midpoint` and `std::lerp`

```cpp
#include <numeric>

int mid = std::midpoint(10, 20);      // 15 (overflow-safe)
double interpolated = std::lerp(0.0, 10.0, 0.5); // 5.0 (linear interpolation)
```

**Purpose**: Safe midpoint calculation (avoids overflow) and linear interpolation for numeric types.

### 2. Bit Manipulation Functions

```cpp
#include <bit>

int count = std::popcount(0b1011);      // 3 (number of 1 bits)
bool power2 = std::has_single_bit(8);   // true
int bits = std::bit_width(7);           // 3 (bits needed)
int next = std::bit_ceil(7);            // 8 (next power of 2)
int rotated = std::rotl(0b1100, 1);     // 0b1001 (rotate left)

if constexpr (std::endian::native == std::endian::little)
{
    // Platform is little-endian
}
```

**Purpose**: Efficient bit manipulation operations and endianness detection.

### 3. Improved `std::bind_front`

```cpp
#include <functional>

auto add = [](int a, int b, int c) { return a + b + c; };
auto add5 = std::bind_front(add, 5);
int result = add5(10, 20); // 35
```

**Purpose**: Partial application of function arguments from the front. Simpler and more efficient than `std::bind`.

### 4. `std::remove_cvref`

```cpp
#include <type_traits>

template<typename T>
void process(T&& value)
{
    using Pure = std::remove_cvref_t<T>; // Removes const, volatile, and reference
    Pure copy = value;
}
```

**Purpose**: Single trait to remove const, volatile, and reference qualifiers. Combines `remove_cv` and `remove_reference`.

---

## Summary

C++20 is a landmark release that represents the biggest evolution of C++ since C++11. It delivers on long-awaited features while maintaining backward compatibility:

### Major Themes:
- **Compile-time programming**: Expanded `constexpr`, `consteval`, `constinit`, compile-time containers
- **Generic programming**: Concepts revolutionize templates with readable constraints
- **Asynchronous programming**: Coroutines enable elegant async code and generators
- **Build system improvements**: Modules eliminate header pain and speed compilation
- **Modern convenience**: Ranges, formatting, calendar/timezone support
- **Thread safety**: Enhanced atomics, cooperative cancellation, new synchronization primitives
- **Type safety**: Three-way comparison, `std::span`, designated initializers

### Performance Benefits:
- Modules dramatically improve compilation times
- Ranges provide zero-cost abstractions
- Concepts enable better optimizations with clearer intent
- Constexpr extensions move more work to compile-time

### Code Quality:
- Concepts provide clear, readable error messages
- Ranges make algorithms more expressive
- Modules prevent macro pollution and improve encapsulation
- Standard formatting and source_location aid debugging

**C++20 truly modernizes C++ for the next decade of systems programming!**
