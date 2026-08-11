# Ultimate C++ Performance & Safety Guide: C-Fast, Rust-Safe

Professional tips and tricks for writing blazingly fast and memory-safe C++ using modern features through C++23.

---

## Table of Contents
1. [Memory Safety](#memory-safety)
2. [Performance Optimization](#performance-optimization)
3. [Compile-Time Programming](#compile-time-programming)
4. [Concurrency & Parallelism](#concurrency--parallelism)
5. [Error Handling](#error-handling)
6. [Advanced Features](#advanced-features)
7. [Professional Compiler Flags Guide](#professional-compiler-flags-guide)

---

## Memory Safety

### Use Smart Pointers, Never Raw Pointers for Ownership

```cpp
// ❌ BAD: Raw pointer ownership
Widget* widget = new Widget();
delete widget; // Easy to forget, double-delete, or leak

// ✅ GOOD: Unique ownership
auto widget = std::make_unique<Widget>();
// Automatically deleted, no leaks

// ✅ GOOD: Shared ownership
auto shared = std::make_shared<Widget>();
```

**Rules:**
- Use `unique_ptr` by default for single ownership
- Use `shared_ptr` only when you need shared ownership
- Use `weak_ptr` to break circular references
- Raw pointers only for non-owning references

### Leverage std::span (C++20) for Safe Array Access

```cpp
// ❌ BAD: Pointer decay loses size info
void process(int* data, size_t size)
{
    data[100] = 0; // Potential buffer overflow
}

// ✅ GOOD: Bounds-aware view
void process(std::span<int> data)
{
    if (data.size() > 100)
    {
        data[100] = 0; // Size is known
    }
}

// Usage
std::vector<int> vec(50);
process(vec); // Automatic conversion, size preserved
```

### Use std::string_view (C++17) Instead of const char*

```cpp
// ❌ BAD: Null-terminated strings are error-prone
void print(const char* str)
{
    if (str == nullptr) return;
    std::cout << str;
}

// ✅ GOOD: Length-aware, no null termination required
void print(std::string_view str)
{
    std::cout << str; // Size always known
}

// Zero-copy substring
std::string s = "Hello World";
std::string_view view = s;
auto hello = view.substr(0, 5); // No allocation!
```

### Avoid Buffer Overflows with Standard Containers

```cpp
// ❌ BAD: Fixed arrays with manual bounds checking
char buffer[256];
strncpy(buffer, input.c_str(), 256);

// ✅ GOOD: Automatic memory management
std::string buffer = input;
std::vector<char> buffer(input.begin(), input.end());
```

### Use std::array Instead of C-Arrays

```cpp
// ❌ BAD: C-array decays to pointer
int arr[10];
void func(int* a) { /* size lost */ }

// ✅ GOOD: Size preserved, bounds checking available
std::array<int, 10> arr;
arr.at(5) = 42; // Bounds checked
arr[5] = 42;    // Fast, unchecked
```

---

## Performance Optimization

### Move Semantics: Eliminate Unnecessary Copies

```cpp
// ✅ GOOD: Move automatically applied (RVO/NRVO)
std::vector<int> getData()
{
    std::vector<int> result(1000000);
    return result; // Move or elision, never copy
}

// Explicit move when needed
std::vector<int> data = getData();
myObject.setData(std::move(data)); // data is now empty

// Custom type with move semantics
class BigData
{
    int* data;
    size_t size;
public:
    // Move constructor
    BigData(BigData&& other) noexcept 
        : data(other.data), size(other.size)
    {
        other.data = nullptr;
        other.size = 0;
    }
    
    // Move assignment
    BigData& operator=(BigData&& other) noexcept
    {
        if (this != &other)
        {
            delete[] data;
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
        }
        return *this;
    }
};
```

**Key Rules:**
- Return by value, compiler will optimize
- Use `std::move()` for local variables passed to functions
- Mark move operations `noexcept` for better performance

### Perfect Forwarding with Universal References

```cpp
// ✅ Zero-overhead wrapper functions
template<typename T>
void wrapper(T&& arg)
{
    actualFunction(std::forward<T>(arg));
}

// Factory function example
template<typename T, typename... Args>
std::unique_ptr<T> make_unique_cached(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}
```

### Reserve Memory for Containers

```cpp
// ❌ BAD: Multiple reallocations
std::vector<int> vec;
for (int i = 0; i < 10000; ++i)
{
    vec.push_back(i); // Reallocates ~log(n) times
}

// ✅ GOOD: Single allocation
std::vector<int> vec;
vec.reserve(10000);
for (int i = 0; i < 10000; ++i)
{
    vec.push_back(i);
}
```

### Emplace Instead of Push/Insert

```cpp
struct Widget
{
    Widget(int x, std::string s) : x(x), s(std::move(s)) {}
    int x;
    std::string s;
};

// ❌ BAD: Temporary object created then moved
std::vector<Widget> vec;
vec.push_back(Widget(42, "hello"));

// ✅ GOOD: Construct in-place
vec.emplace_back(42, "hello");
```

### Small String Optimization (SSO) Awareness

```cpp
// Strings < ~15 chars (implementation-dependent) don't allocate
std::string small = "hello";      // Stack only, no heap
std::string large = "very long string that exceeds SSO buffer"; // Heap

// Design with SSO in mind
void processMany()
{
    std::vector<std::string> names;
    names.reserve(1000);
    for (int i = 0; i < 1000; ++i)
    {
        names.emplace_back("User" + std::to_string(i)); // SSO-friendly
    }
}
```

### Use constexpr for Compile-Time Computation

```cpp
// ✅ GOOD: Compile-time when possible
constexpr int factorial(int n)
{
    return n <= 1 ? 1 : n * factorial(n - 1);
}

constexpr int result = factorial(5); // Computed at compile time
int runtime = factorial(getUserInput()); // Computed at runtime

// constexpr with containers (C++20)
constexpr auto buildLookupTable()
{
    std::array<int, 256> table{};
    for (int i = 0; i < 256; ++i)
    {
        table[i] = i * i;
    }
    return table;
}

constexpr auto lookup = buildLookupTable(); // Compile-time!
```

### Branch Prediction Hints (C++20)

```cpp
int process(int value)
{
    if (value > 0) [[likely]]
    {
        return fastPath(value);
    }
    else [[unlikely]]
    {
        return slowPath(value);
    }
}

// Use in loops
for (int i = 0; i < 1000000; ++i)
{
    if (data[i] > threshold) [[unlikely]]
    {
        handleRareCase();
    }
    else [[likely]]
    {
        normalProcessing();
    }
}
```

### Avoid Virtual Functions in Hot Paths

```cpp
// ❌ BAD: Virtual call overhead
class Base
{
    virtual void process() = 0;
};

// ✅ GOOD: Template-based polymorphism (static dispatch)
template<typename T>
void process(T& obj)
{
    obj.process(); // Inlined, no vtable lookup
}

// Or use std::variant for type-safe unions
using Shape = std::variant<Circle, Square, Triangle>;

void draw(const Shape& shape)
{
    std::visit([](const auto& s) { s.draw(); }, shape);
}
```

### Cache-Friendly Data Structures

```cpp
// ❌ BAD: Poor cache locality
struct Node
{
    int data;
    Node* next; // Scattered in memory
};

// ✅ GOOD: Contiguous memory
std::vector<int> data; // Cache-friendly

// Structure of Arrays (SoA) for better vectorization
struct ParticlesAoS
{
    struct Particle { float x, y, z; };
    std::vector<Particle> particles; // x,y,z,x,y,z...
};

struct ParticlesSoA
{
    std::vector<float> x; // x,x,x,x...
    std::vector<float> y; // y,y,y,y...
    std::vector<float> z; // z,z,z,z...
}; // SIMD-friendly, better cache usage
```

### String Concatenation Performance

```cpp
// ❌ BAD: Multiple allocations
std::string result = str1 + str2 + str3 + str4;

// ✅ GOOD: Pre-allocate
std::string result;
result.reserve(str1.size() + str2.size() + str3.size() + str4.size());
result += str1;
result += str2;
result += str3;
result += str4;

// ✅ GOOD: Use std::format (C++20)
std::string result = std::format("{}{}{}{}", str1, str2, str3, str4);
```

### Avoid Unnecessary Copies with const&

```cpp
// ❌ BAD: Copy on every call
void process(std::vector<int> data) { /* ... */ }

// ✅ GOOD: Pass by const reference
void process(const std::vector<int>& data) { /* ... */ }

// ✅ GOOD: Pass by value when you need a copy anyway
void store(std::vector<int> data)
{
    member = std::move(data);
}
```

---

## Modern C++ Best Practices

### Structured Bindings (C++17)

```cpp
// ✅ Clean unpacking of tuples, pairs, structs
std::map<std::string, int> map;
for (const auto& [key, value] : map)
{
    std::cout << key << ": " << value << "\n";
}

struct Point { int x, y; };
Point p{10, 20};
auto [x, y] = p;

// Works with arrays too
int arr[] = {1, 2, 3};
auto [a, b, c] = arr;
```

### if/switch with Initializers (C++17)

```cpp
// ✅ Limit variable scope
if (auto it = map.find(key); it != map.end())
{
    use(it->second);
} // it destroyed here

if (auto [iter, inserted] = set.insert(value); inserted)
{
    std::cout << "Inserted: " << *iter << "\n";
}

// switch with initializer
switch (auto val = getValue(); val)
{
    case 1: handle1(); break;
    case 2: handle2(); break;
    default: handleDefault();
}
```

### std::optional for Optional Values (C++17)

```cpp
// ❌ BAD: Magic values
int find(const std::vector<int>& vec, int value)
{
    auto it = std::find(vec.begin(), vec.end(), value);
    return it != vec.end() ? *it : -1; // -1 is magic value
}

// ✅ GOOD: Explicit optional
std::optional<int> find(const std::vector<int>& vec, int value)
{
    auto it = std::find(vec.begin(), vec.end(), value);
    return it != vec.end() ? std::optional(*it) : std::nullopt;
}

// Usage patterns
if (auto result = find(vec, 42))
{
    std::cout << "Found: " << *result << "\n";
}

int value = find(vec, 42).value_or(0); // With default

// Transform optional values
std::optional<int> opt = 42;
auto doubled = opt.transform([](int x) { return x * 2; }); // C++23
```

### std::variant for Type-Safe Unions (C++17)

```cpp
// ✅ Type-safe discriminated union
using Value = std::variant<int, double, std::string>;

Value v = 42;
v = 3.14;
v = "hello";

// Type-safe visitation
std::visit([](auto&& arg)
{
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, int>)
    {
        std::cout << "int: " << arg << "\n";
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        std::cout << "double: " << arg << "\n";
    }
    else
    {
        std::cout << "string: " << arg << "\n";
    }
}, v);

// Get value safely
if (auto* pval = std::get_if<int>(&v))
{
    std::cout << "int: " << *pval << "\n";
}
```

### Ranges and Views (C++20)

```cpp
#include <ranges>

std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// ✅ Composable, lazy operations
auto even = vec 
    | std::views::filter([](int n) { return n % 2 == 0; })
    | std::views::transform([](int n) { return n * n; });

for (int n : even) // Computed on iteration
{
    std::cout << n << " "; // 4 16 36 64 100
}

// More views
auto first5 = vec | std::views::take(5);
auto skip2 = vec | std::views::drop(2);
auto reversed = vec | std::views::reverse;

// Generate ranges
for (int i : std::views::iota(0, 10))
{
    std::cout << i << " "; // 0 1 2 3 4 5 6 7 8 9
}
```

### Concepts (C++20) for Better Templates

```cpp
// ❌ BAD: Unclear template requirements
template<typename T>
T add(T a, T b)
{
    return a + b; // Cryptic error if T doesn't support +
}

// ✅ GOOD: Explicit requirements
template<typename T>
concept Addable = requires(T a, T b)
{
    { a + b } -> std::convertible_to<T>;
};

template<Addable T>
T add(T a, T b)
{
    return a + b; // Clear error messages
}

// Standard concepts
template<std::integral T>
T increment(T value)
{
    return value + 1;
}

// Custom concepts
template<typename T>
concept Drawable = requires(T obj)
{
    { obj.draw() } -> std::same_as<void>;
    { obj.getPosition() } -> std::convertible_to<Point>;
};

template<Drawable T>
void render(const T& object)
{
    object.draw();
}
```

### std::expected (C++23) for Error Handling

```cpp
// ✅ Rust-like Result type
std::expected<int, std::string> divide(int a, int b)
{
    if (b == 0)
    {
        return std::unexpected("Division by zero");
    }
    return a / b;
}

// Usage
auto result = divide(10, 2);
if (result)
{
    std::cout << "Result: " << *result << "\n";
}
else
{
    std::cerr << "Error: " << result.error() << "\n";
}

// Chain operations
auto final = divide(10, 2)
    .and_then([](int x) { return divide(x, 2); })
    .transform([](int x) { return x * 2; });
```

### Designated Initializers (C++20)

```cpp
struct Config
{
    int timeout = 30;
    bool verbose = false;
    std::string path = "/default";
};

// ✅ Clear initialization
Config cfg
{
    .timeout = 60,
    .verbose = true
    // path uses default
};

// Nested structs
struct Server
{
    std::string host;
    int port;
    Config config;
};

Server server
{
    .host = "localhost",
    .port = 8080,
    .config = {.timeout = 120, .verbose = true}
};
```

### std::format (C++20) - Type-Safe Formatting

```cpp
#include <format>

// ✅ Type-safe, fast string formatting
int value = 42;
std::string result = std::format("The answer is {}", value);

// Width and alignment
std::string s = std::format("{:>10}", "right"); // Right-align
std::string s = std::format("{:<10}", "left");  // Left-align
std::string s = std::format("{:^10}", "center"); // Center

// Numbers
std::format("{:d}", 42);      // Decimal: 42
std::format("{:x}", 255);     // Hex: ff
std::format("{:b}", 5);       // Binary: 101
std::format("{:.2f}", 3.14159); // Float: 3.14

// Multiple arguments
std::format("Name: {}, Age: {}, Score: {:.1f}", name, age, score);
```

---

## Compile-Time Programming

### constexpr and consteval (C++20)

```cpp
// constexpr: May run at compile-time
constexpr int square(int x)
{
    return x * x;
}

// consteval: MUST run at compile-time (C++20)
consteval int compiletime_square(int x)
{
    return x * x;
}

constexpr int a = square(5);           // Compile-time
int b = square(getUserInput());        // Runtime OK

constexpr int c = compiletime_square(5); // Compile-time
// int d = compiletime_square(getUserInput()); // ERROR!

// Complex compile-time computation
consteval auto generateTable()
{
    std::array<int, 100> table{};
    for (int i = 0; i < 100; ++i)
    {
        table[i] = i * i;
    }
    return table;
}

constexpr auto squares = generateTable(); // All at compile-time!
```

### constexpr Containers (C++20)

```cpp
constexpr auto compute()
{
    std::vector<int> vec = {1, 2, 3};
    vec.push_back(4);
    return vec[2]; // Returns 3
}

constexpr int result = compute(); // All at compile-time!

// Complex compile-time data structures
constexpr auto buildMap()
{
    std::map<int, std::string> m;
    m[1] = "one";
    m[2] = "two";
    m[3] = "three";
    return m;
}
```

### constinit (C++20) - Safe Static Initialization

```cpp
// ✅ Ensure compile-time initialization
constexpr int computeValue()
{
    return 42 * 2;
}

constinit int global = computeValue(); // Must be compile-time

// Prevents static initialization order fiasco
constinit std::atomic<int> counter{0}; // Safe global
```

### if constexpr for Compile-Time Branching

```cpp
template<typename T>
auto process(T value)
{
    if constexpr (std::is_integral_v<T>)
    {
        return value * 2;
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        return value * 2.0;
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        return value + value;
    }
    else
    {
        return value;
    }
}

// Type-specific optimization
template<typename T>
void optimizedSort(std::vector<T>& vec)
{
    if constexpr (sizeof(T) <= 8 && std::is_trivially_copyable_v<T>)
    {
        // Use fast radix sort for small trivial types
        radixSort(vec);
    }
    else
    {
        // Use std::sort for complex types
        std::sort(vec.begin(), vec.end());
    }
}
```

---

## Concurrency & Parallelism

### std::jthread (C++20): RAII Threads

```cpp
// ❌ BAD: Must manually join
std::thread t([]{ doWork(); });
t.join();

// ✅ GOOD: Auto-joining thread
{
    std::jthread t([]{ doWork(); });
} // Automatically joins here

// ✅ GOOD: Cancellation support
std::jthread t([](std::stop_token stoken)
{
    while (!stoken.stop_requested())
    {
        doWork();
        std::this_thread::sleep_for(100ms);
    }
});

t.request_stop(); // Cooperative cancellation
// Automatically joins on destruction
```

### Atomic Operations

```cpp
#include <atomic>

// ✅ Lock-free counter
std::atomic<int> counter{0};

void increment()
{
    counter.fetch_add(1, std::memory_order_relaxed);
}

// Different memory orders
counter.store(42, std::memory_order_release);
int value = counter.load(std::memory_order_acquire);

// Compare and swap
int expected = 10;
counter.compare_exchange_strong(expected, 20);

// ✅ Wait/notify (C++20) - Efficient blocking
std::atomic<bool> ready{false};

// Thread 1
ready.store(true);
ready.notify_one();

// Thread 2
ready.wait(false); // Efficient blocking wait
```

### std::latch and std::barrier (C++20)

```cpp
// ✅ One-time synchronization point
void parallelProcess(const std::vector<Task>& tasks)
{
    std::latch done{tasks.size()};
    
    for (const auto& task : tasks)
    {
        std::jthread([&task, &done]
        {
            task.execute();
            done.count_down();
        });
    }
    
    done.wait(); // Wait for all threads
}

// ✅ Reusable synchronization barrier
void phaseProcessing()
{
    std::barrier sync_point{4}; // 4 threads
    
    for (int phase = 0; phase < 10; ++phase)
    {
        doWork(phase);
        sync_point.arrive_and_wait(); // Synchronize between phases
    }
}

// Barrier with completion function
std::barrier sync{4, []() noexcept
{
    std::cout << "All threads synchronized!\n";
}};
```

### Parallel STL Algorithms (C++17)

```cpp
#include <execution>

std::vector<int> vec(1000000);

// ✅ Parallel execution
std::sort(std::execution::par, vec.begin(), vec.end());

// ✅ Parallel transformation
std::transform(std::execution::par_unseq, 
    vec.begin(), vec.end(), vec.begin(),
    [](int x) { return x * x; });

// Parallel accumulation
int sum = std::reduce(std::execution::par, vec.begin(), vec.end());

// Execution policies:
// seq        - Sequential (normal)
// par        - Parallel (uses threads)
// par_unseq  - Parallel + vectorized (SIMD)
```

### Thread-Safe Initialization

```cpp
// ✅ Thread-safe lazy initialization (Magic Static)
Widget& getWidget()
{
    static Widget instance; // Initialized exactly once
    return instance;
}

// ✅ std::call_once for explicit control
std::once_flag flag;
Resource* resource = nullptr;

void initResource()
{
    std::call_once(flag, []
    {
        resource = new Resource();
    });
}

// Thread-safe static initialization (C++20)
std::atomic<Config*> config{nullptr};

Config& getConfig()
{
    if (auto* p = config.load(std::memory_order_acquire))
    {
        return *p;
    }
    std::call_once(flag, []
    {
        config.store(new Config(), std::memory_order_release);
    });
    return *config.load();
}
```

---

## Error Handling

### RAII for Resource Management

```cpp
// ✅ Exception-safe resource handling
class FileHandle
{
    FILE* file;
public:
    explicit FileHandle(const char* name) 
        : file(fopen(name, "r"))
    {
        if (!file)
        {
            throw std::runtime_error("Failed to open");
        }
    }
    
    ~FileHandle()
    {
        if (file)
        {
            fclose(file);
        }
    }
    
    // Delete copy, allow move
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    
    FileHandle(FileHandle&& other) noexcept 
        : file(std::exchange(other.file, nullptr)) {}
    
    FileHandle& operator=(FileHandle&& other) noexcept
    {
        if (this != &other)
        {
            if (file) fclose(file);
            file = std::exchange(other.file, nullptr);
        }
        return *this;
    }
    
    FILE* get() const { return file; }
};
```

### Scope Guards

```cpp
// Manual RAII guard
template<typename F>
struct ScopeExit
{
    F func;
    bool active = true;
    
    explicit ScopeExit(F f) : func(std::move(f)) {}
    
    ~ScopeExit()
    {
        if (active) func();
    }
    
    void dismiss() { active = false; }
    
    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
};

void process()
{
    void* buffer = malloc(1024);
    ScopeExit cleanup{[&] { free(buffer); }};
    
    // Any return or exception -> buffer freed
    if (error) return;
    doWork(buffer);
    
    // cleanup.dismiss(); // Optional: cancel cleanup
}
```

### Exception Safety Guarantees

```cpp
// ✅ Strong exception safety
class Container
{
    std::vector<int> data;
public:
    void add(int value)
    {
        data.push_back(value); // If throws, nothing changes
    }
    
    // No-throw swap for strong guarantee
    void swap(Container& other) noexcept
    {
        data.swap(other.data);
    }
    
    // Copy-and-swap idiom for strong guarantee
    Container& operator=(const Container& other)
    {
        Container temp(other); // Copy
        swap(temp);            // No-throw swap
        return *this;          // Old data destroyed
    }
};
```

### noexcept Specification

```cpp
// ✅ Mark non-throwing functions
void criticalFunction() noexcept
{
    // Compiler can optimize better
    // std::terminate called if exception thrown
}

// Conditional noexcept
template<typename T>
void move_if_noexcept_helper(T& obj) 
    noexcept(std::is_nothrow_move_constructible_v<T>)
{
    // ...
}

// Move operations should be noexcept
class MyClass
{
    std::vector<int> data;
public:
    MyClass(MyClass&&) noexcept = default;
    MyClass& operator=(MyClass&&) noexcept = default;
};
```

---

## Advanced Features

### std::source_location (C++20) - Better Debug Info

```cpp
#include <source_location>

void log(std::string_view message,
         std::source_location loc = std::source_location::current())
{
    std::cout << "File: " << loc.file_name() << "\n"
              << "Line: " << loc.line() << "\n"
              << "Function: " << loc.function_name() << "\n"
              << "Message: " << message << "\n";
}

// Usage - automatically captures call site
log("Something happened"); // No __FILE__, __LINE__ needed!
```

### std::bit_cast (C++20) - Safe Type Punning

```cpp
#include <bit>

// ❌ BAD: Undefined behavior
float f = 3.14f;
uint32_t i = *reinterpret_cast<uint32_t*>(&f);

// ✅ GOOD: Safe type punning
float f = 3.14f;
uint32_t i = std::bit_cast<uint32_t>(f);

// Only works for same-sized trivially copyable types
static_assert(sizeof(float) == sizeof(uint32_t));
```

### std::byteswap (C++23) - Efficient Byte Swapping

```cpp
#include <bit>

uint32_t value = 0x12345678;
uint32_t swapped = std::byteswap(value); // 0x78563412

// Endianness detection
if constexpr (std::endian::native == std::endian::little)
{
    // Little endian system
}
```

### std::unreachable (C++23) - Optimization Hint

```cpp
#include <utility>

int getCategory(int value)
{
    switch (value)
    {
        case 0: return 1;
        case 1: return 2;
        case 2: return 3;
        default: 
            std::unreachable(); // Tells compiler this never happens
    }
}
```

### std::assume (C++23) - Compiler Assumptions

```cpp
void process(int* ptr, size_t size)
{
    [[assume(ptr != nullptr)]];
    [[assume(size > 0)]];
    [[assume(size % 4 == 0)]]; // Alignment hint
    
    // Compiler can optimize based on assumptions
    for (size_t i = 0; i < size; ++i)
    {
        ptr[i] *= 2;
    }
}
```

### Coroutines (C++20) - Async Programming

```cpp
#include <coroutine>

// Simple generator
generator<int> fibonacci()
{
    int a = 0, b = 1;
    while (true)
    {
        co_yield a;
        auto tmp = a;
        a = b;
        b = tmp + b;
    }
}

// Usage
for (int num : fibonacci())
{
    if (num > 100) break;
    std::cout << num << " ";
}

// Async task
task<int> asyncCompute()
{
    co_await startAsyncOperation();
    int result = co_await getResult();
    co_return result;
}
```

### Three-Way Comparison (C++20) - Spaceship Operator

```cpp
#include <compare>

struct Point
{
    int x, y;
    
    // ✅ One operator generates all comparisons
    auto operator<=>(const Point& other) const = default;
    
    // Generates: ==, !=, <, <=, >, >=
};

// Custom three-way comparison
struct Custom
{
    int value;
    
    std::strong_ordering operator<=>(const Custom& other) const
    {
        return value <=> other.value;
    }
};
```

### std::mdspan (C++23) - Multidimensional Array View

```cpp
#include <mdspan>

std::vector<int> data(100);
std::mdspan<int, std::extents<size_t, 10, 10>> matrix(data.data());

// Natural 2D indexing
matrix[3, 4] = 42;
int value = matrix[5, 7];

// Works with any layout
std::mdspan<int, std::dextents<size_t, 3>> tensor(data.data(), 5, 4, 5);
```

---

## Professional Compiler Flags Guide

### Understanding Optimization Levels

**What are optimization levels?**
- They tell the compiler how much effort to spend making your code faster
- Higher levels = faster code but slower compilation
- Always use optimization in release builds

```bash
-O0  # No optimization (default)
     # Best for debugging
     # Fastest compilation
     # Slowest execution

-O1  # Basic optimization
     # Good balance for development
     # Still debuggable

-O2  # Recommended for release
     # Good performance
     # Reasonable compile time
     # Most commonly used

-O3  # Maximum optimization
     # Best performance
     # May increase code size
     # Longer compile time

-Os  # Optimize for size
     # Smaller binaries
     # Good for embedded systems

-Ofast  # -O3 + aggressive math optimizations
        # Fastest but may break IEEE compliance
```

### GCC/Clang Development Build (Maximum Safety)

```bash
# Compile command for development/debugging
g++ -std=c++23 \
    -O0 \                          # No optimization - fast compilation
    -g3 \                          # Maximum debug info
    -Wall \                        # Enable all common warnings
    -Wextra \                      # Enable extra warnings
    -Wpedantic \                   # Strict ISO C++ warnings
    -Werror \                      # Treat warnings as errors
    -Wshadow \                     # Warn if variable shadows another
    -Wconversion \                 # Warn on implicit type conversions
    -Wsign-conversion \            # Warn on signed/unsigned conversions
    -Wold-style-cast \             # Warn about C-style casts
    -Wunused \                     # Warn about unused variables
    -Wnull-dereference \           # Warn about null pointer dereferences
    -Wdouble-promotion \           # Warn about float to double promotion
    -Wformat=2 \                   # Extra printf format checking
    -fsanitize=address \           # Detect memory errors (use-after-free, leaks)
    -fsanitize=undefined \         # Detect undefined behavior
    -fno-omit-frame-pointer \      # Better stack traces
    -D_GLIBCXX_DEBUG \             # Enable STL debug mode (bounds checking)
    -D_GLIBCXX_DEBUG_PEDANTIC \    # Extra STL debugging
    main.cpp -o main_debug
```

**What each flag does:**

- **`-std=c++23`**: Use C++23 standard features
- **`-O0`**: No optimization - keeps code structure intact for debugging
- **`-g3`**: Include ALL debug symbols (macros, inline functions, etc.)
- **`-Wall -Wextra -Wpedantic`**: Catch common programming errors
- **`-Werror`**: STOP compilation on ANY warning - forces clean code
- **`-Wshadow`**: Prevents bugs where inner variable hides outer variable
- **`-fsanitize=address`**: Runtime memory error detector (VERY IMPORTANT!)
  - Detects: buffer overflow, use-after-free, memory leaks
  - Adds ~2x slowdown but catches bugs immediately
- **`-fsanitize=undefined`**: Catches undefined behavior at runtime
  - Detects: integer overflow, null pointer access, division by zero
- **`-D_GLIBCXX_DEBUG`**: Enables bounds checking in STL containers
  - `vector[100]` will crash if vector has < 100 elements
  - Slower but catches bugs early

### GCC/Clang Release Build (Maximum Performance)

```bash
# Compile command for production/release
g++ -std=c++23 \
    -O3 \                          # Maximum optimization
    -DNDEBUG \                     # Disable asserts
    -march=native \                # Optimize for YOUR CPU (use -march=x86-64 for portable)
    -mtune=native \                # Tune for YOUR CPU
    -flto \                        # Link-time optimization (whole program optimization)
    -ffast-math \                  # Aggressive math optimizations (USE WITH CAUTION)
    -funroll-loops \               # Unroll loops for speed
    -fomit-frame-pointer \         # Slightly faster code
    -fno-exceptions \              # Disable exceptions (if you don't use them)
    -fno-rtti \                    # Disable runtime type info (if you don't use dynamic_cast)
    -fvisibility=hidden \          # Hide symbols by default (smaller binary)
    -fvisibility-inlines-hidden \  # Hide inline functions
    -s \                           # Strip debug symbols (smaller binary)
    main.cpp -o main_release
```

**What each flag does:**

- **`-O3`**: Maximum optimization - makes code MUCH faster
- **`-DNDEBUG`**: Disables `assert()` statements (faster execution)
- **`-march=native`**: Compiler generates code for YOUR specific CPU
  - Uses AVX2, AVX512, etc. if your CPU supports it
  - WARNING: Binary won't work on older CPUs! Use `-march=x86-64` for compatibility
- **`-flto`**: Looks at entire program and optimizes across files
  - Can inline functions across files
  - Removes unused code
  - Makes code 10-30% faster but slows compilation
- **`-ffast-math`**: Breaks strict IEEE floating point for speed
  - Allows: reordering operations, assuming no NaN/Inf
  - DON'T use if you need exact math results
  - CAN break numerical code!
- **`-fno-exceptions`**: Remove exception handling overhead (~5% faster)
  - Only use if you don't throw/catch exceptions
- **`-fno-rtti`**: Remove runtime type information (~1-2% faster)
  - Can't use `dynamic_cast` or `typeid`
- **`-s`**: Removes debug symbols - makes binary much smaller

### Profile-Guided Optimization (PGO) - Ultimate Performance

**What is PGO?**
- Run your program with typical data
- Compiler sees which code runs most often
- Optimizes hot paths aggressively
- Can give 10-20% extra speed!

```bash
# Step 1: Compile with instrumentation (adds profiling code)
g++ -std=c++23 -O2 -fprofile-generate main.cpp -o app_instrumented

# Step 2: Run with TYPICAL workload (important!)
./app_instrumented < typical_input.txt
./app_instrumented --process data.csv
# This creates .gcda profiling files

# Step 3: Recompile using profile data
g++ -std=c++23 -O3 -fprofile-use -march=native -flto main.cpp -o app_optimized

# Result: app_optimized is 10-20% faster than -O3 alone!
```

**When to use PGO:**
- Performance-critical applications
- Programs with hot loops
- Server applications with clear workload patterns

### MSVC (Windows) Compiler Flags

```bash
# Development Build (Visual Studio)
cl /std:c++latest \      # Latest C++ standard
   /Od \                 # No optimization
   /Zi \                 # Full debug info
   /W4 \                 # Warning level 4 (maximum)
   /WX \                 # Treat warnings as errors
   /analyze \            # Static analysis
   /fsanitize=address \  # Address sanitizer
   /RTC1 \               # Runtime checks
   main.cpp

# Release Build (Visual Studio)
cl /std:c++latest \      # Latest C++ standard
   /O2 \                 # Optimize for speed
   /GL \                 # Whole program optimization (like -flto)
   /LTCG \               # Link-time code generation
   /arch:AVX2 \          # Use AVX2 instructions
   /fp:fast \            # Fast floating point (like -ffast-math)
   /GR- \                # Disable RTTI (like -fno-rtti)
   /EHs-c- \             # Disable exceptions (like -fno-exceptions)
   main.cpp

# Profile-Guided Optimization (MSVC)
# Step 1: Instrument
cl /O2 /GL /LTCG:PGI main.cpp

# Step 2: Run
app.exe < typical_input.txt

# Step 3: Optimize
cl /O2 /GL /LTCG:PGO main.cpp
```

### Sanitizers - Runtime Bug Detection

**Address Sanitizer (ASan)** - Detects Memory Errors
```bash
g++ -fsanitize=address -g main.cpp

# Detects:
# - Buffer overflow (out-of-bounds access)
# - Use-after-free
# - Use-after-return
# - Memory leaks
# Overhead: ~2x slowdown
```

**Undefined Behavior Sanitizer (UBSan)** - Detects UB
```bash
g++ -fsanitize=undefined -g main.cpp

# Detects:
# - Integer overflow
# - Division by zero
# - Null pointer dereference
# - Unaligned pointer access
# Overhead: ~1.2x slowdown
```

**Thread Sanitizer (TSan)** - Detects Race Conditions
```bash
g++ -fsanitize=thread -g main.cpp

# Detects:
# - Data races
# - Thread ordering issues
# Overhead: ~5-15x slowdown
# CAN'T use with ASan at same time!
```

**Memory Sanitizer (MSan)** - Detects Uninitialized Reads
```bash
clang++ -fsanitize=memory -g main.cpp

# Detects:
# - Reading uninitialized memory
# Overhead: ~3x slowdown
# Clang only!
```

### Important Flag Combinations

**Best Debugging Experience:**
```bash
g++ -std=c++23 -O0 -g3 -Wall -Wextra -Wpedantic -Werror \
    -fsanitize=address,undefined \
    -D_GLIBCXX_DEBUG \
    main.cpp
```

**Best Release Performance:**
```bash
g++ -std=c++23 -O3 -DNDEBUG -march=native -flto \
    -ffast-math -funroll-loops \
    main.cpp
```

**Safe Release Build (no -ffast-math):**
```bash
g++ -std=c++23 -O3 -DNDEBUG -march=native -flto \
    main.cpp
```

**Cross-Platform Release (works on any x86-64 CPU):**
```bash
g++ -std=c++23 -O3 -DNDEBUG -march=x86-64 -flto \
    main.cpp
```

### Common Pitfalls

**1. Using -ffast-math incorrectly:**
```cpp
// This WILL break with -ffast-math!
if (x != x)  // NaN check
{
    // Won't work with -ffast-math
}

// Use this instead:
if (std::isnan(x))
{
    // Always works
}
```

**2. Forgetting NDEBUG in release:**
```cpp
assert(expensive_check()); // Runs in debug, removed with -DNDEBUG
```

**3. Using -march=native for distributed binaries:**
```bash
# DON'T do this for software you distribute!
g++ -march=native main.cpp  # Won't work on older CPUs

# DO this instead:
g++ -march=x86-64 main.cpp  # Works everywhere
```

### Quick Reference Card

```bash
# Development
-O0 -g3 -Wall -Wextra -Werror -fsanitize=address,undefined

# Testing
-O2 -g -Wall -Wextra -fsanitize=thread  # For race detection

# Release
-O3 -DNDEBUG -march=native -flto

# Maximum Performance
-O3 -DNDEBUG -march=native -flto -ffast-math -fprofile-use

# Small Binary
-Os -DNDEBUG -flto -s
```

---

## Key Principles Summary

### Memory Safety Checklist
- ✅ No raw `new`/`delete` (use smart pointers)
- ✅ No raw arrays (use `std::array` or `std::vector`)
- ✅ Prefer `std::span` over pointer + size
- ✅ Use `std::string_view` for read-only strings
- ✅ Mark move constructors `noexcept`
- ✅ Enable sanitizers in testing
- ✅ Use `std::optional` instead of nullptr
- ✅ Use `std::variant` instead of unions

### Performance Checklist
- ✅ Profile before optimizing (use `perf`, `vtune`, or `gprof`)
- ✅ Return by value (trust RVO/NRVO)
- ✅ Use `std::move` for local rvalues
- ✅ Reserve container capacity when size known
- ✅ Use `emplace` instead of `push`/`insert`
- ✅ Mark functions `constexpr` when possible
- ✅ Use `[[likely]]`/`[[unlikely]]` for hot paths
- ✅ Enable LTO and PGO for release builds
- ✅ Use parallel algorithms where applicable
- ✅ Prefer `std::array` over `std::vector` for fixed size
- ✅ Use Structure of Arrays (SoA) for vectorization

### Code Quality Checklist
- ✅ Use concepts to constrain templates
- ✅ Prefer `std::expected` for error handling
- ✅ Use `std::variant` for type-safe unions
- ✅ Use ranges for composable operations
- ✅ Mark constructors `explicit`
- ✅ Enable all compiler warnings (`-Wall -Wextra`)
- ✅ Run static analyzers regularly
- ✅ Write exception-safe code (RAII)
- ✅ Use `std::format` instead of `sprintf`
- ✅ Use `noexcept` for move operations

### Build Configuration Checklist
- ✅ Debug build: `-O0 -g3 -fsanitize=address,undefined`
- ✅ Release build: `-O3 -DNDEBUG -march=native -flto`
- ✅ Test with all sanitizers (ASan, UBSan, TSan)
- ✅ Use PGO for maximum performance
- ✅ Enable static analysis (`-analyze` or `clang-tidy`)

---

**Remember:** Profile first, optimize second. Safety and performance are not mutually exclusive in modern C++. Use the right tools and flags for each phase of development.
