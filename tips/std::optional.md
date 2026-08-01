# C++ std::optional Tips and Tricks Guide

## Table of Contents
1. [Introduction](#introduction)
2. [Basic Usage](#basic-usage)
3. [Construction and Assignment](#construction-and-assignment)
4. [Value Access](#value-access)
5. [Checking for Values](#checking-for-values)
6. [Modification and Reset](#modification-and-reset)
7. [Monadic Operations (C++23)](#monadic-operations-c23)
8. [Performance Tips](#performance-tips)
9. [Common Patterns](#common-patterns)
10. [Error Handling](#error-handling)
11. [Advanced Techniques](#advanced-techniques)
12. [Best Practices](#best-practices)
13. [Common Pitfalls](#common-pitfalls)

## Introduction

`std::optional<T>` (C++17) is a vocabulary type that represents an optional value - it either contains a value of type `T` or is empty. It's perfect for representing nullable values without using pointers or special sentinel values.

```cpp
#include <optional>
#include <iostream>
#include <string>
```

## Basic Usage

### Declaration and Initialization

```cpp
// Empty optional
std::optional<int> opt1;
std::optional<int> opt2 = std::nullopt;

// With value
std::optional<int> opt3 = 42;
std::optional<int> opt4{42};
std::optional<std::string> opt5 = "Hello";

// Using make_optional
auto opt6 = std::make_optional<int>(42);
auto opt7 = std::make_optional<std::string>("World");
```

### Basic Checking and Access

```cpp
std::optional<int> GetValue(bool condition)
{
    if (condition) return 42;
    return std::nullopt;
}

auto opt = GetValue(true);
if (opt)
{
    std::cout << "Value: " << *opt << std::endl;
}
else
{
    std::cout << "No value" << std::endl;
}
```

## Construction and Assignment

### In-Place Construction

```cpp
// Avoid unnecessary copies/moves
std::optional<std::string> opt;
opt.emplace("Hello World");  // Constructs string in-place

// For complex types
struct Point
{
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
};
std::optional<Point> point;
point.emplace(10, 20);  // Direct construction
```

### Assignment Operators

```cpp
std::optional<int> opt;

opt = 42;              // Assign value
opt = std::nullopt;    // Reset to empty
opt.reset();           // Also resets to empty

// Copy/move from another optional
std::optional<int> other = 100;
opt = other;           // Copy assignment
opt = std::move(other); // Move assignment
```

## Value Access

### Safe Access Methods

```cpp
std::optional<int> opt = 42;

// Method 1: has_value() and value()
if (opt.has_value())
{
    int val = opt.value();  // Throws std::bad_optional_access if empty
}

// Method 2: Boolean conversion and dereference
if (opt)
{
    int val = *opt;  // Undefined behavior if empty, but faster
}

// Method 3: value_or() for defaults
int val = opt.value_or(0);  // Returns 0 if empty

// Method 4: Structured bindings (C++17)
if (auto val = opt; val)
{
    std::cout << *val << std::endl;
}
```

### Pointer-like Access

```cpp
struct Person
{
    std::string name;
    int age;
};

std::optional<Person> person = Person{"Alice", 30};

if (person)
{
    std::cout << person->name << " is " << person->age << std::endl;
    // Equivalent to: (*person).name and (*person).age
}
```

## Checking for Values

### Multiple Ways to Check

```cpp
std::optional<int> opt = 42;

// Boolean conversion (recommended)
if (opt) { /* has value */ }

// Explicit method
if (opt.has_value()) { /* has value */ }

// Comparison with nullopt
if (opt != std::nullopt) { /* has value */ }

// In conditions
while (opt) { /* do something, then maybe reset opt */ }
```

### Pattern Matching Style

```cpp
template<typename T>
void Process(const std::optional<T>& opt)
{
    if (opt)
    {
        // Handle value case
        std::cout << "Value: " << *opt << std::endl;
    }
    else
    {
        // Handle empty case
        std::cout << "No value" << std::endl;
    }
}
```

## Modification and Reset

### Resetting Values

```cpp
std::optional<int> opt = 42;

opt.reset();           // Set to empty
opt = std::nullopt;    // Also set to empty
opt = {};              // Also set to empty (since C++17)
```

### Conditional Assignment

```cpp
std::optional<int> opt;

// Only assign if empty
if (!opt)
{
    opt = 42;
}

// Or use value_or for reading
int value = opt.value_or(42);
```

### Swapping

```cpp
std::optional<int> opt1 = 42;
std::optional<int> opt2 = 100;

opt1.swap(opt2);  // opt1 now has 100, opt2 has 42
std::swap(opt1, opt2);  // Swap back using std::swap
```

## Monadic Operations (C++23)

### Transform and Map

```cpp
#if __cpp_lib_optional >= 202110L  // C++23 feature

std::optional<int> opt = 42;

// Transform - applies function if value exists
auto doubled = opt.transform([](int x) { return x * 2; });
// doubled is std::optional<int> with value 84

// Chain transformations
auto result = opt
    .transform([](int x) { return x * 2; })
    .transform([](int x) { return std::to_string(x); });
// result is std::optional<std::string>

#endif
```

### And Then (Flat Map)

```cpp
#if __cpp_lib_optional >= 202110L

std::optional<int> Divide(int a, int b)
{
    if (b == 0) return std::nullopt;
    return a / b;
}

std::optional<int> opt = 10;
auto result = opt.and_then([](int x) { return Divide(x, 2); });
// result is std::optional<int> with value 5

#endif
```

### Or Else

```cpp
#if __cpp_lib_optional >= 202110L

std::optional<int> opt1;
std::optional<int> opt2 = 42;

auto result = opt1.or_else([&]() { return opt2; });
// result has value 42

#endif
```

## Performance Tips

### Avoid Unnecessary Copies

```cpp
// BAD: Creates temporary string
std::optional<std::string> Bad(const std::string& input)
{
    if (input.empty()) return std::nullopt;
    return std::string(input + " processed");
}

// GOOD: Use emplace or move
std::optional<std::string> Good(std::string input)
{
    if (input.empty()) return std::nullopt;
    input += " processed";
    return input;  // Move happens automatically
}

// BETTER: Construct in-place
std::optional<std::string> Better(const std::string& input)
{
    if (input.empty()) return std::nullopt;
    std::optional<std::string> result;
    result.emplace(input + " processed");
    return result;
}
```

### Use References for Large Objects

```cpp
class LargeObject { /*...*/ };

// Don't copy large objects unnecessarily
void Process(const std::optional<LargeObject>& obj)
{
    if (obj)
    {
        // Use *obj or obj-> to access
        obj->someMethod();
    }
}
```

### Move When Possible

```cpp
std::optional<std::vector<int>> CreateVector(size_t size)
{
    if (size == 0) return std::nullopt;
    
    std::vector<int> vec(size, 42);
    return vec;  // Automatic move
}

// When consuming:
auto opt = CreateVector(1000);
if (opt)
{
    auto vec = std::move(*opt);  // Move out of optional
    // Use vec...
}
```

## Common Patterns

### Factory Functions

```cpp
class Database
{
public:
    std::optional<User> FindUser(int id)
    {
        // Database lookup logic
        if (UserExists(id))
        {
            return User{id, GetName(id), GetEmail(id)};
        }
        return std::nullopt;
    }
    
    std::optional<std::string> GetConfig(const std::string& key)
    {
        auto it = config_.find(key);
        if (it != config_.end())
        {
            return it->second;
        }
        return std::nullopt;
    }
    
private:
    std::map<std::string, std::string> config_;
    bool UserExists(int id) const { /* ... */ return true; }
    std::string GetName(int id) const { return "User" + std::to_string(id); }
    std::string GetEmail(int id) const { return "user" + std::to_string(id) + "@example.com"; }
};
```

### Chaining Operations

```cpp
// Pre-C++23 chaining
std::optional<int> ParseAndDouble(const std::string& str)
{
    if (auto num = ParseInt(str))
    {
        return *num * 2;
    }
    return std::nullopt;
}

std::optional<int> ParseInt(const std::string& str)
{
    try
    {
        return std::stoi(str);
    }
    catch (...)
    {
        return std::nullopt;
    }
}

// Usage
auto result = ParseAndDouble("42");
if (result)
{
    std::cout << "Doubled: " << *result << std::endl;
}
```

### Optional Parameters

```cpp
class HttpClient
{
public:
    struct RequestOptions
    {
        std::optional<int> timeout;
        std::optional<std::string> userAgent;
        std::optional<bool> followRedirects;
    };
    
    Response Get(const std::string& url, const RequestOptions& opts = {})
    {
        int timeout = opts.timeout.value_or(30);  // Default 30 seconds
        std::string ua = opts.userAgent.value_or("DefaultClient/1.0");
        bool follow = opts.followRedirects.value_or(true);
        
        // Make request with these parameters
        return Response{};
    }
};

// Usage
HttpClient client;
client.Get("https://api.example.com");  // Use defaults

RequestOptions opts;
opts.timeout = 60;
opts.userAgent = "MyApp/2.0";
client.Get("https://api.example.com", opts);  // Custom options
```

### Result Type Pattern

```cpp
template<typename T, typename E>
class Result
{
    std::optional<T> value_;
    std::optional<E> error_;
    
public:
    static Result Success(T value)
    {
        Result r;
        r.value_ = std::move(value);
        return r;
    }
    
    static Result Failure(E error)
    {
        Result r;
        r.error_ = std::move(error);
        return r;
    }
    
    bool IsSuccess() const { return value_.has_value(); }
    bool IsError() const { return error_.has_value(); }
    
    const T& GetValue() const { return *value_; }
    const E& GetError() const { return *error_; }
};
```

## Error Handling

### Exception-Safe Access

```cpp
std::optional<int> opt;

// SAFE: Won't throw, returns default
int safe_value = opt.value_or(-1);

// POTENTIALLY UNSAFE: Throws std::bad_optional_access if empty
try
{
    int risky_value = opt.value();
}
catch (const std::bad_optional_access& e)
{
    std::cerr << "Accessed empty optional: " << e.what() << std::endl;
}

// UNDEFINED BEHAVIOR if empty (but fast if you're sure it has value)
if (opt)
{
    int fast_value = *opt;  // Only do this when you've checked!
}
```

### Converting Exceptions to Optional

```cpp
template<typename F>
auto MakeOptionalFromException(F&& func) -> std::optional<decltype(func())>
{
    try
    {
        return func();
    }
    catch (...)
    {
        return std::nullopt;
    }
}

// Usage
auto result = MakeOptionalFromException([]()
{
    return std::stoi("not_a_number");
});

if (!result)
{
    std::cout << "Parsing failed" << std::endl;
}
```

## Advanced Techniques

### Optional with Custom Types

```cpp
class Resource
{
public:
    Resource(const std::string& name) : name_(name)
    {
        std::cout << "Resource " << name_ << " created\n";
    }
    
    ~Resource()
    {
        std::cout << "Resource " << name_ << " destroyed\n";
    }
    
    Resource(const Resource&) = delete;  // Non-copyable
    Resource& operator=(const Resource&) = delete;
    
    Resource(Resource&& other) noexcept : name_(std::move(other.name_))
    {
        std::cout << "Resource " << name_ << " moved\n";
    }
    
private:
    std::string name_;
};

std::optional<Resource> CreateResource(bool shouldCreate)
{
    if (shouldCreate)
    {
        return Resource{"MyResource"};  // Move construction
    }
    return std::nullopt;
}
```

### Optional References (Workaround)

```cpp
// std::optional<T&> doesn't exist, but you can use reference_wrapper
#include <functional>

template<typename T>
using optional_ref = std::optional<std::reference_wrapper<T>>;

void Example()
{
    int x = 42;
    optional_ref<int> opt_ref;
    
    opt_ref = std::ref(x);  // Now holds reference to x
    
    if (opt_ref)
    {
        opt_ref->get() = 100;  // Modify x through reference
        // or: opt_ref.value().get() = 100;
    }
    
    std::cout << x << std::endl;  // Prints 100
}

// Alternative: use pointers
std::optional<int*> opt_ptr;
int x = 42;
opt_ptr = &x;
if (opt_ptr)
{
    **opt_ptr = 100;  // Modify x through pointer
}
```

### Comparison Operations

```cpp
std::optional<int> opt1 = 42;
std::optional<int> opt2 = 43;
std::optional<int> opt3;

// Comparison with values
bool a = (opt1 == 42);        // true
bool b = (opt1 < 43);         // true
bool c = (opt3 == std::nullopt); // true

// Comparison between optionals
bool d = (opt1 < opt2);       // true (42 < 43)
bool e = (opt1 == opt3);      // false (value vs empty)
bool f = (opt3 < opt1);       // true (empty < value)

// Three-way comparison (C++20)
#if __cpp_lib_three_way_comparison
auto cmp = opt1 <=> opt2;  // std::strong_ordering::less
#endif
```

## Best Practices

### 1. Prefer `value_or()` for Defaults

```cpp
// GOOD
int GetValue(const std::optional<int>& opt)
{
    return opt.value_or(42);  // Clear intent, no exceptions
}

// AVOID (unless you want exceptions)
int GetValue(const std::optional<int>& opt)
{
    return opt ? *opt : 42;  // More verbose
}
```

### 2. Use Structured Bindings

```cpp
// C++17 structured binding
if (auto opt = database.FindUser(123); opt)
{
    const auto& user = *opt;
    std::cout << "Found user: " << user.name << std::endl;
}
```

### 3. Make Intent Clear

```cpp
// GOOD: Clear return type indicates optional result
std::optional<User> FindUserByEmail(const std::string& email);

// AVOID: Unclear what nullptr means
User* FindUserByEmail(const std::string& email);  // Returns nullptr if not found?
```

### 4. Use `emplace()` for Complex Construction

```cpp
// GOOD: No temporary objects
std::optional<ComplexObject> opt;
opt.emplace(param1, param2, param3);

// LESS EFFICIENT: Creates temporary
opt = ComplexObject{param1, param2, param3};
```

### 5. Be Consistent with Error Handling

```cpp
class UserService
{
public:
    // Consistent: all lookup methods return optional
    std::optional<User> FindById(int id);
    std::optional<User> FindByEmail(const std::string& email);
    std::optional<User> FindByUsername(const std::string& username);
    
    // Also consistent: exceptions for invalid input, optional for not found
    std::optional<User> FindByIdValidated(int id)
    {
        if (id <= 0) throw std::invalid_argument("ID must be positive");
        return FindById(id);
    }
};
```

## Common Pitfalls

### 1. Don't Check Twice

```cpp
// BAD: Double checking
if (opt.has_value() && opt.value() > 0)
{
    // ...
}

// GOOD: Single check
if (opt && *opt > 0)
{
    // ...
}
```

### 2. Be Careful with Auto

```cpp
std::optional<int> GetNumber();

// BAD: auto hides the optional nature
auto num = GetNumber();
int result = num + 5;  // Compilation error!

// GOOD: Be explicit or check first
if (auto num = GetNumber())
{
    int result = *num + 5;  // OK
}
```

### 3. Don't Store Optional References

```cpp
// WRONG: This doesn't compile
// std::optional<int&> opt_ref;

// CORRECT: Use reference_wrapper or pointers
std::optional<std::reference_wrapper<int>> opt_ref;
// or
std::optional<int*> opt_ptr;
```

### 4. Watch Out for Boolean Conversion

```cpp
std::optional<bool> maybe_flag = false;

// BAD: This checks if optional has value, not the boolean value!
if (maybe_flag)  // true because optional contains false
{
    // This runs even though the contained value is false
}

// GOOD: Access the actual boolean value
if (maybe_flag && *maybe_flag)
{
    // This only runs if optional contains true
}

// BETTER: Be explicit
if (maybe_flag.has_value() && maybe_flag.value())
{
    // Clear intent
}
```

### 5. Move Semantics Awareness

```cpp
std::optional<std::string> opt = "Hello";

std::string s1 = std::move(*opt);  // Move out the value
// opt still has_value(), but *opt is in moved-from state!

if (opt)
{
    std::cout << *opt << std::endl;  // Undefined behavior: moved-from string
}

// CORRECT: Reset after moving
std::string s2 = std::move(*opt);
opt.reset();  // Now it's clearly empty
```

---

## Summary

`std::optional` is a powerful tool for expressing nullable values in a type-safe way. Key takeaways:

- Use it for functions that might not return a value
- Prefer `value_or()` for providing defaults
- Use `emplace()` for in-place construction
- Always check before dereferencing (unless you're certain it has a value)
- Be aware of the difference between checking the optional and checking its contained value
- Consider monadic operations in C++23 for elegant chaining
- Use it consistently across your API for similar use cases

Remember that `std::optional` adds a small overhead (typically one byte for the flag plus alignment), but this is usually negligible compared to the safety and clarity benefits it provides.
