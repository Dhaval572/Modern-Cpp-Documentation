# C++23 Features Guide

A Comprehensive Guide To The Most Important Features Introduced In C++23, Focusing On Practical Improvements That Make Modern C++ More Expressive And Easier To Use.

## Table Of Contents
- [Range Algorithms Enhancements](#range-algorithms-enhancements)
- [std::expected - Better Error Handling](#stdexpected---better-error-handling)
- [std::mdspan - Multidimensional Arrays](#stdmdspan---multidimensional-arrays)
- [Deducing This](#deducing-this)
- [if consteval](#if-consteval)
- [std::print And std::println](#stdprint-and-stdprintln)
- [Monadic Operations For std::optional](#monadic-operations-for-stdoptional)
- [Stack Trace Library](#stack-trace-library)
- [constexpr Improvements](#constexpr-improvements)
- [std::flat_map And std::flat_set](#stdflat_map-and-stdflat_set)
- [Additional Quality Of Life Improvements](#additional-quality-of-life-improvements)

## Range Algorithms Enhancements

C++23 Adds Several New Algorithms To The Ranges Library That Make Common Operations Simpler And More Expressive.

### ranges::contains()

Check If A Range Contains A Value Without Using `Find` And Comparing To `End()`.

```cpp
#include <vector>
#include <algorithm>

std::vector<int> number_list = {1, 2, 3, 4, 5};

// C++20 Way
bool is_found = std::ranges::find(number_list, 3) != number_list.end();

// C++23 Way - Much Cleaner!
bool is_found = std::ranges::contains(number_list, 3);
```

### ranges::contains_subrange()

Check If A Range Contains A Specific Subrange.

```cpp
std::vector<int> data_sequence = {1, 2, 3, 4, 5, 6};
std::vector<int> pattern_to_find = {3, 4, 5};

bool has_pattern = std::ranges::contains_subrange(data_sequence, pattern_to_find);
// True - The Pattern Exists In data_sequence
```

### ranges::fold_left() And Friends

Functional-Style Folding Operations For Ranges.

```cpp
std::vector<int> number_list = {1, 2, 3, 4, 5};

// Sum All Elements
int total_sum = std::ranges::fold_left(number_list, 0, std::plus{});
// Result: 15

// Product Of All Elements
int total_product = std::ranges::fold_left(number_list, 1, std::multiplies{});
// Result: 120
```

### ranges::to() - Convert Ranges To Containers

Convert Any Range Directly To A Container Without Manual Iteration.

```cpp
#include <ranges>
#include <vector>
#include <set>

auto squared_numbers = std::views::iota(1, 6) 
                     | std::views::transform([](int x) 
                         { 
                             return x * x; 
                         });

// Convert Range To Vector
std::vector<int> number_vector = std::ranges::to<std::vector>(squared_numbers);
// number_vector = {1, 4, 9, 16, 25}

// Convert To Set
std::set<int> number_set = std::ranges::to<std::set>(squared_numbers);
```

### ranges::starts_with() And ranges::ends_with()

Check Prefix And Suffix Without Manual Slicing.

```cpp
std::vector<int> data_sequence = {1, 2, 3, 4, 5};
std::vector<int> prefix_to_check = {1, 2, 3};

bool starts_correctly = std::ranges::starts_with(data_sequence, prefix_to_check);  // True
bool ends_correctly = std::ranges::ends_with(data_sequence, {4, 5});              // True
```

## std::expected - Better Error Handling

`std::expected<T, E>` Provides A Type-Safe Way To Return Either A Value Or An Error, Similar To Rust's `Result` Type.

```cpp
#include <expected>
#include <string>
#include <print>

std::expected<int, std::string> Divide(int a, int b)
{
    if (b == 0)
    {
        return std::unexpected("Division by zero");
    }
    return a / b;
}

int main()
{
    auto result = Divide(10, 2);

    if (result.has_value())
    {
        std::println("Result: {}", *result);
    }
    else
    {
        std::println("Error: {}", result.error());
    }

    int safe_result = Divide(10, 0).value_or(-1);
    std::println("Safe result: {}", safe_result);
    return 0;
}
```

### Monadic Operations With std::expected

```cpp
auto SafeDivideAndDouble(int a, int b)
{
    return Divide(a, b)
        .and_then([](int x) -> std::expected<int, std::string>
            {
                return x * 2;
            })
        .or_else([](const std::string& error_message) -> std::expected<int, std::string>
            {
                std::println("Handling error: {}", error_message);
                return 0;
            });
}
```

## std::mdspan - Multidimensional Arrays

`std::mdspan` Provides A Non-Owning View Over Multidimensional Data With Zero Overhead.

```cpp
#include <mdspan>
#include <vector>

std::vector<int> matrix_data(12);
std::iota(matrix_data.begin(), matrix_data.end(), 0);

// Create A 3x4 View Over The Data
std::mdspan<int, std::extents<size_t, 3, 4>> matrix_view(matrix_data.data());

// Access Elements Naturally
matrix_view[1, 2] = 42;
int cell_value = matrix_view[2, 3];

// Iterate Over Rows
for (size_t row_index = 0; row_index < matrix_view.extent(0); ++row_index)
{
    for (size_t column_index = 0; column_index < matrix_view.extent(1); ++column_index)
    {
        std::println("{} ", matrix_view[row_index, column_index]);
    }
}
```

### Dynamic Extents

```cpp
// Runtime Dimensions
std::mdspan<double, std::dextents<size_t, 2>> dynamic_matrix_view(
    matrix_data.data(), 3, 4
);

// Mix Static And Dynamic
std::mdspan<int, std::extents<size_t, std::dynamic_extent, 4>> 
    semi_dynamic_view(matrix_data.data(), 3);
```

## Deducing This

Explicit Object Parameters Eliminate Code Duplication For Const/Non-Const And Lvalue/Rvalue Overloads.

```cpp
struct Container
{
    std::vector<int> Data;
    
    // Old Way: Need 4 Overloads
    // auto& Get() &;
    // const auto& Get() const &;
    // auto&& Get() &&;
    // const auto&& Get() const &&;
    
    // C++23 Way: One Function Handles All Cases
    template<typename Self>
    auto&& Get(this Self&& self)
    {
        return std::forward<Self>(self).Data;
    }
};

void UseContainer()
{
    Container container_instance;
    const Container const_container_instance;

    auto& reference = container_instance.Get();        // Lvalue Reference
    auto& const_reference = const_container_instance.Get();      // Const Lvalue Reference
    auto&& rvalue_reference = Container{}.Get();  // Rvalue Reference
}
```

### CRTP Simplification

```cpp
// Old CRTP Pattern
template<typename Derived>
struct Base
{
    void Interface()
    {
        static_cast<Derived*>(this)->Implementation();
    }
};

// C++23 With Deducing This
struct NewBase
{
    template<typename Self>
    void Interface(this Self&& self)
    {
        self.Implementation();
    }
};

struct Derived : NewBase
{
    void Implementation() { /* ... */ }
};
```

## if consteval

Distinguish Between Compile-Time And Runtime Execution Contexts.

```cpp
consteval int CompileTimeOnly()
{
    return 42;
}

constexpr int FlexibleFunction(int x)
{
    if consteval
    {
        // This Code Runs Only At Compile Time
        return CompileTimeOnly() + x;
    }
    else
    {
        // This Code Runs Only At Runtime
        std::println("Runtime computation");
        return x * 2;
    }
}

// Compile-Time Call
constexpr int compile_time_result = FlexibleFunction(10);  // Uses Compile-Time Branch

// Runtime Call
int runtime_result = FlexibleFunction(10);  // Uses Runtime Branch
```

### Use Case: Optimized String Handling

```cpp
constexpr std::string_view ProcessString(std::string_view input_string)
{
    if consteval
    {
        // Compile-Time: Can Use Complex Validation
        static_assert(input_string.size() > 0, "String must not be empty");
        return input_string.substr(0, 10);
    }
    else
    {
        // Runtime: Simpler Check
        return input_string.empty() ? "default" : input_string;
    }
}
```

## std::print And std::println

Type-Safe, Formatted Output That's Easier And Safer Than Iostream.

```cpp
#include <print>

std::println("Hello, {}!", "World");
// Output: Hello, World!

int integer_value = 42;
double pi_value = 3.14159;
std::println("x = {}, pi = {:.2f}", integer_value, pi_value);
// Output: x = 42, pi = 3.14

// Print To stderr
std::println(stderr, "Error: {}", error_message);

// No Newline Version
std::print("Processing... ");
std::print("Done!\n");
```

### Format Specifications

```cpp
int numeric_value = 255;

std::println("{:b}", numeric_value);   // Binary: 11111111
std::println("{:o}", numeric_value);   // Octal: 377
std::println("{:x}", numeric_value);   // Hex: ff
std::println("{:X}", numeric_value);   // Hex Uppercase: FF
std::println("{:#x}", numeric_value);  // Hex With Prefix: 0xff

double floating_number = 1234.5678;
std::println("{:10.2f}", floating_number);    // Width 10, 2 Decimals: "   1234.57"
std::println("{:<10.2f}", floating_number);   // Left Align: "1234.57   "
std::println("{:>10.2f}", floating_number);   // Right Align: "   1234.57"
```

## Monadic Operations For std::optional

Chain Operations On Optional Values Without Explicit Checking.

```cpp
#include <optional>

std::optional<int> GetValue();
std::optional<int> ProcessValue(int x);

// Old Way
std::optional<int> ProcessOldWay()
{
    std::optional<int> result;
    if (auto value = GetValue())
    {
        result = ProcessValue(*value);
    }
    return result;
}

// C++23 Way
auto ProcessNewWay()
{
    return GetValue()
        .and_then([](int x) 
            { 
                return ProcessValue(x); 
            });
}
```

### Transform, and_then, or_else

```cpp
std::optional<std::string> GetUsername();

auto CreateGreeting()
{
    return GetUsername()
        .transform([](const std::string& user_name) 
            {
                return "Hello, " + user_name;
            })
        .or_else([]() -> std::optional<std::string> 
            {
                return "Hello, Guest";
            });
}

// More Complex Chain
std::optional<int> initial_value = std::optional{5};
auto processed_result = initial_value
    .and_then([](int x) -> std::optional<int>
        {
            return x > 0 ? std::optional{x * 2} : std::nullopt;
        })
    .transform([](int x) 
        { 
            return x + 10; 
        })
    .or_else([]() -> std::optional<int> 
        { 
            return 0; 
        });
```

## Stack Trace Library

Programmatically Capture And Inspect Stack Traces.

```cpp
#include <stacktrace>
#include <print>

void ProblematicFunction()
{
    auto stack_trace = std::stacktrace::current();
    
    std::println("Stack trace:");
    for (const auto& trace_entry : stack_trace)
    {
        std::println("  {} at {}:{}",
            trace_entry.description(),
            trace_entry.source_file(),
            trace_entry.source_line());
    }
}

// Custom Error Handling
class CustomError
{
    std::stacktrace ErrorTrace;
    std::string ErrorMessage;
    
public:
    CustomError(std::string error_message) 
        : ErrorTrace(std::stacktrace::current())
        , ErrorMessage(std::move(error_message)) 
    {
    }
    
    void Print() const
    {
        std::println("Error: {}", ErrorMessage);
        std::println("{}", std::to_string(ErrorTrace));
    }
};
```

## constexpr Improvements

### constexpr std::unique_ptr And std::shared_ptr

```cpp
constexpr int ComputeValue()
{
    auto unique_pointer = std::make_unique<int>(42);
    *unique_pointer += 8;
    return *unique_pointer;  // Result: 50
}

constexpr int computed_result = ComputeValue();  // Evaluated At Compile Time
```

### constexpr std::vector And std::string

```cpp
constexpr std::vector<int> CreateVector()
{
    std::vector<int> number_vector;
    for (int index = 0; index < 5; ++index)
    {
        number_vector.push_back(index * index);
    }
    return number_vector;
}

constexpr auto square_numbers = CreateVector();
// square_numbers = {0, 1, 4, 9, 16} At Compile Time
```

## std::flat_map And std::flat_set

Associative Containers Backed By Contiguous Memory For Better Cache Performance.

```cpp
#include <flat_map>

std::flat_map<int, std::string> key_value_cache;

key_value_cache[1] = "one";
key_value_cache[2] = "two";
key_value_cache[3] = "three";

// Better Cache Locality Than std::map
// Faster Iteration
// Lower Memory Overhead

if (key_value_cache.contains(2))
{
    std::println("Found: {}", key_value_cache[2]);
}

// Bulk Operations Are Efficient
std::vector<std::pair<int, std::string>> data_items = 
{
    {4, "four"}, {5, "five"}, {6, "six"}
};
key_value_cache.insert(data_items.begin(), data_items.end());
```

### Performance Benefits

```cpp
// std::flat_set Is Faster For:
// - Small To Medium-Sized Collections
// - Iteration (Contiguous Memory)
// - Cache-Friendly Operations

std::flat_set<int> identifier_set = {1, 5, 3, 9, 2};
// Automatically Sorted: {1, 2, 3, 5, 9}

for (int identifier : identifier_set)
{
    // Sequential Memory Access - Cache Friendly!
    ProcessIdentifier(identifier);
}
```

## Additional Quality Of Life Improvements

### std::string::contains()

```cpp
std::string text_content = "Hello, World!";

// C++23
if (text_content.contains("World"))
{
    std::println("Found it!");
}

// Before C++23
if (text_content.find("World") != std::string::npos)
{
    std::println("Found it!");
}
```

### std::string::resize_and_overwrite()

Efficiently Resize Strings When You'll Overwrite The Content.

```cpp
std::string text_buffer;
size_t final_size = text_buffer.resize_and_overwrite(100, [](char* buffer_data, size_t buffer_size)
{
    // Write To buffer_data Buffer
    size_t bytes_written = std::snprintf(buffer_data, buffer_size, "Formatted: %d", 42);
    return bytes_written;  // Actual Size Used
});
```

### Multidimensional Subscript Operator

```cpp
struct Matrix
{
    std::vector<int> MatrixData;
    size_t RowCount;
    size_t ColumnCount;
    
    // C++23: Multi-Dimensional Subscript
    int& operator[](size_t row_index, size_t column_index)
    {
        return MatrixData[row_index * ColumnCount + column_index];
    }
};

void UseMatrix()
{
    Matrix matrix_instance{.MatrixData = std::vector<int>(9), .RowCount = 3, .ColumnCount = 3};
    matrix_instance[1, 2] = 42;  // Natural Syntax!
}
```

### std::unreachable()

Hint To The Compiler That Code Is Unreachable For Optimizations.

```cpp
int GetValue(Color color_value)
{
    switch (color_value)
    {
        case Color::Red:   return 1;
        case Color::Green: return 2;
        case Color::Blue:  return 3;
    }
    std::unreachable();  // Better Than Undefined Behavior
}
```

### Literal Suffixes For size_t

```cpp
std::vector<int> number_vector(10uz);  // uz For size_t

for (size_t index = 0uz; index < number_vector.size(); ++index)
{
    // No Signed/Unsigned Comparison Warnings
}
```

### std::byteswap()

Portable Byte Order Reversal.

```cpp
#include <bit>

uint32_t original_value = 0x12345678;
uint32_t swapped_value = std::byteswap(original_value);
// swapped_value = 0x78563412

// Useful For Endianness Conversion
uint16_t network_order_value = std::byteswap(host_order_value);
```

## Migration Tips

When Migrating To C++23, Consider These Changes:

1. **Replace Manual Contains Checks** With `ranges::contains()` For Cleaner Code
2. **Use std::expected** Instead Of Throwing Exceptions For Expected Error Cases
3. **Adopt std::print** For New Code - It's Safer And More Efficient Than Iostream
4. **Use Deducing This** To Eliminate Const/Ref-Qualifier Overload Duplication
5. **Consider std::flat_map/flat_set** For Performance-Critical Associative Containers
6. **Use std::mdspan** When Working With Multidimensional Data Views
7. **Leverage ranges::to()** To Simplify Range-To-Container Conversions

## Compiler Support

As Of Early 2025, C++23 Features Have Varying Support Across Compilers:

- **GCC 14+**: Most Features Supported
- **Clang 18+**: Good Support For Core Features
- **MSVC 19.38+**: Increasing Support In Visual Studio 2022

Always Check Your Specific Compiler Version For Feature Availability Using Feature Test Macros Like `__cpp_lib_expected`, `__cpp_lib_print`, Etc.

## Conclusion

C++23 Brings Significant Quality-Of-Life Improvements Focusing On:
- Simpler, More Expressive Code (Ranges Enhancements, std::print)
- Better Error Handling (std::expected)
- Performance Optimizations (std::flat_map, std::mdspan)
- Less Boilerplate (Deducing This, if consteval)

These Features Make Modern C++ More Approachable While Maintaining Zero-Overhead Abstractions And Compile-Time Guarantees.
