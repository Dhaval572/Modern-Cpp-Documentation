# C++ `std::span` – Complete Guide with Tips & Tricks (C++20/23)

## 📋 Table Of Contents
- [Overview](#overview)
- [Basic Usage](#basic-usage)
- [Common Patterns](#common-patterns)
- [Performance Tips](#performance-tips)
- [Safety Considerations](#safety-considerations)
- [Advanced Techniques](#advanced-techniques)
- [Common Pitfalls](#common-pitfalls)
- [C++23 Updates](#c++23-updates)
- [Examples](#examples)

## Overview

`std::span` (C++20) is a non‑owning view over a contiguous sequence of objects. It is lightweight (typically two pointers or a pointer and a size) and provides safe, convenient access to arrays, `std::vector`, `std::array`, and C‑style buffers.

### Key Characteristics
- **Non‑owning** – does not manage memory.
- **Lightweight** – copies are cheap (usually two machine words).
- **Bounds‑checking** – optional via `at()` or explicit checks.
- **Interoperability** – works seamlessly with C‑style arrays, `std::vector`, `std::array`, and any contiguous container.

---

## Basic Usage

### Declaration & Initialization
```cpp
#include <span>
#include <vector>
#include <array>

// From a C‑style array
int my_array[] = {1, 2, 3, 4, 5};
std::span<int> span_1{my_array};          // size deduced

// From std::array
std::array<float, 6> my_std_array = {1.0f, 2.0f, 3.0f};
std::span<float> span_2{my_std_array};

// From std::vector
std::vector<double> my_vector = {1.0, 2.0, 3.0};
std::span<double> span_3{my_vector};

// From pointer and size (dynamic extent)
int* data_ptr = new int[10];
std::span<int> span_4{data_ptr, 10};

// Const span (read‑only view)
std::span<const int> const_span = span_1;
```

### Fixed‑size vs Dynamic Extent
```cpp
// Dynamic extent (runtime size, the default)
std::span<int> dynamic_span;

// Fixed extent (compile‑time size)
std::span<int, 5> fixed_span;   // always contains exactly 5 elements

// Deduction guides
auto span_auto_1 = std::span{my_array};          // std::span<int, 5>
auto span_auto_2 = std::span{my_vector};         // std::span<double, std::dynamic_extent>
```

---

## Common Patterns

### Function Parameters (Replacing `(pointer, size)`)
```cpp
// Old way (error‑prone)
void ProcessOld(int* data, size_t size);

// New way (safer and self‑documenting)
void ProcessData(std::span<int> data)
{
    for (auto& element : data)
    {
        // Process element
    }
}

// Const correctness
void ReadData(std::span<const int> data);
void ModifyData(std::span<int> data);
```

### Subviews & Slicing
```cpp
std::span<int> full_span{my_array, 10};

// First N elements
auto first_three = full_span.first(3);

// Last N elements
auto last_two = full_span.last(2);

// Subspan (offset, count) – bounds‑checked
auto middle = full_span.subspan(3, 4);

// All operations are bounds‑checked at runtime (they throw if out of range)
```

### Iteration
```cpp
std::span<int> data = /* ... */;

// Range‑based for loop
for (auto& element : data)
{
    element *= 2;
}

// With index
for (size_t i = 0; i < data.size(); ++i)
{
    data[i] = static_cast<int>(i * 10);
}

// Using STL algorithms
std::sort(data.begin(), data.end());
auto it = std::find(data.begin(), data.end(), 42);
```

---

## Performance Tips

### 1. Pass by Value, Not by Reference
```cpp
// GOOD: span is small (typically 2 pointers)
void ProcessData(std::span<int> data);

// BAD: unnecessary indirection
void ProcessData(const std::span<int>& data);   // extra reference overhead
```

### 2. Use `std::as_bytes()` / `std::as_writable_bytes()`
```cpp
std::span<int> data = /* ... */;

// For binary I/O or low‑level operations
auto read_only_bytes = std::as_bytes(data);           // span<const std::byte>
auto writable_bytes = std::as_writable_bytes(data);   // span<std::byte>
```

### 3. Avoid Unnecessary Bounds Checks
```cpp
std::span<int> data = /* ... */;

// Use unchecked access when bounds are already verified
if (index < data.size())
{
    // safe to use unchecked
    data[index];          // or data.data()[index] for maximum performance
}

// In performance‑critical loops, obtain raw pointer once
for (size_t i = 0; i < data.size(); ++i)
{
    auto* raw_ptr = data.data();   // single acquisition
    raw_ptr[i] = TransformValue(raw_ptr[i]);
}
```

### 4. Fixed Extent Enables Compile‑time Optimisation
```cpp
template<size_t N>
void ProcessFixed(std::span<int, N> data)
{
    // Compiler knows size at compile‑time → better optimisations
}

// Use when size is known at compile‑time
std::array<int, 1024> buffer;
ProcessFixed(std::span{buffer});
```

---

## Safety Considerations

### 1. Lifetime Management – Never Let a Span Outlive Its Data
```cpp
// DANGER: span outlives the data
std::span<int> GetInvalidSpan()
{
    std::vector<int> local_data = {1, 2, 3};
    return {local_data};   // dangling span!
}

// SAFE: return the container itself (or ensure the span is used within scope)
std::vector<int> GetData()
{
    return {1, 2, 3, 4, 5};
}
```

### 2. Bounds Checking – Use `at()` for Safety
```cpp
std::span<int> data = /* ... */;

// Checked access (throws std::out_of_range)
try
{
    int value = data.at(100);   // throws if out of bounds
}
catch (const std::out_of_range& error)
{
    // handle error
}

// Unchecked access (undefined behaviour if out of bounds)
int value = data[100];   // UB if index out of range

// Safe pattern: check first, then access
if (index < data.size())
{
    int value = data[index];   // safe
}
```

### 3. Const Correctness – Use `span<const T>` for Read‑only Access
```cpp
void BadFunction(std::span<int> data)
{
    data[0] = 42;   // modifies original data
}

void GoodFunction(std::span<const int> data)
{
    // can only read – prevents accidental modification
    int value = data[0];
}

// Use const spans for parameters that should not modify the data
```

---

## Advanced Techniques

### 1. Type‑erased Byte Processing
```cpp
void ProcessBytes(std::span<const std::byte> data)
{
    // accepts any data type via as_bytes()
}

template<typename T>
void SerializeObject(const T& object)
{
    auto bytes = std::as_bytes(std::span{&object, 1});
    ProcessBytes(bytes);
}
```

### 2. Multi‑dimensional Views (using spans of spans)
```cpp
using Matrix = std::span<std::span<int>>;

void ProcessMatrix(Matrix matrix)
{
    for (auto row : matrix)
    {
        for (auto& element : row)
        {
            // process element
        }
    }
}

// Alternatively, a flat buffer with row stride
void ProcessFlatMatrix(std::span<int> data, size_t rows, size_t cols)
{
    for (size_t i = 0; i < rows; ++i)
    {
        auto row = data.subspan(i * cols, cols);
        // process row
    }
}
```

### 3. Interoperability with Legacy Code
```cpp
// C interface
void LegacyFunction(int* data, int size);

// Modern wrapper
void ModernWrapper(std::span<int> data)
{
    LegacyFunction(data.data(), static_cast<int>(data.size()));
}

// Reverse: from legacy to span
void LegacyCallback(int* data, int size)
{
    std::span<int> data_span{data, static_cast<size_t>(size)};
    // modern processing
}
```

### 4. Custom Strided View (Proof‑of‑Concept)
```cpp
template<typename T>
class StridedSpan
{
    std::span<T> data_;
    size_t stride_;
    
public:
    class Iterator
    {
        T* current_;
        size_t stride_;
        
    public:
        Iterator(T* ptr, size_t stride) : current_(ptr), stride_(stride)
        {
        }
        
        // iterator operations (++, !=, etc.) omitted for brevity
    };
    
    Iterator begin()
    {
        return Iterator(data_.data(), stride_);
    }
    
    Iterator end()
    {
        return Iterator(data_.data() + data_.size() * stride_, stride_);
    }
    
    T& operator[](size_t index)
    {
        return data_[index * stride_];
    }
};
```

---

## Common Pitfalls

### 1. Lifetime Issues (Again)
```cpp
// WRONG: temporary container
std::span<int> bad_span = std::vector<int>{1, 2, 3};

// WRONG: local variable
std::span<int> CreateDanglingSpan()
{
    int local_array[10];
    return {local_array};   // dangling!
}

// RIGHT: ensure the span does not outlive its data
class DataProcessor
{
    std::vector<int> owned_data_;   // owns the data
    std::span<int> data_view_;      // views owned_data_
    
public:
    DataProcessor() : data_view_{owned_data_}
    {
    }
};
```

### 2. Assuming Null Termination
```cpp
// BAD: span does not guarantee null termination
std::span<char> string_span = /* ... */;
std::cout << string_span.data();   // could overflow!

// GOOD: use string_view for text
std::string_view string_view{string_span.data(), string_span.size()};
```

### 3. Misusing Fixed Extent
```cpp
// Use fixed extent only when size is truly known at compile time
std::vector<int> dynamic_data;
std::span<int> dynamic_view{dynamic_data};      // GOOD

std::array<int, 10> fixed_data;
std::span<int, 10> fixed_view{fixed_data};      // GOOD

// Don't force fixed extent on dynamic data
int* dynamic_ptr = new int[ComputeSize()];
std::span<int> correct_view{dynamic_ptr, ComputeSize()};   // GOOD
// std::span<int, ???> wrong_view{dynamic_ptr, ComputeSize()};   // BAD
```

### 4. Overlooking Alignment
```cpp
struct t_AlignedData
{
    alignas(64) int data[16];
};

std::span<t_AlignedData> aligned_span;
// as_bytes() preserves alignment information
auto bytes = std::as_bytes(aligned_span);
```

---

## C++23 Updates

`std::span` itself received only minor enhancements in C++23, mainly in the `constexpr` domain. Here are the notable changes:

- **More `constexpr` functions** – many `span` operations are now usable in constant expressions, enabling compile‑time processing.
- **Improved interoperability with the ranges library** – while `std::span` already modelled a range in C++20, C++23 refines the concept requirements and adds more range adaptor compatibility.
- **Introduction of `std::mdspan`** – a new type (not a `span` but a complement) for multi‑dimensional contiguous views. It supports strides, layouts, and compile‑time or dynamic extents.

```cpp
// Example: using mdspan (C++23)
#include <mdspan>

std::array<int, 24> buffer;
std::mdspan<int, std::extents<size_t, 3, 4, 2>> matrix_3d{buffer.data()};

// You can extract a slice (which is also an mdspan) and then, if desired,
// obtain a 1D span from it – but mdspan is not a direct replacement for span.
```

- **`std::as_const_span`?** – not introduced; the existing `std::as_bytes` already provides a const‑byte view.

The core usage patterns of `std::span` remain unchanged; the C++23 updates are backward‑compatible and focus on making the type more powerful in generic and compile‑time contexts.

---

## Best Practices Summary

1. **Prefer `std::span` over `(pointer, size)` pairs** – it’s safer and more expressive.
2. **Pass spans by value** – they are cheap to copy.
3. **Use `std::span<const T>` for read‑only parameters** – clarifies intent.
4. **Always consider the lifetime of the referenced data** – ensure the span does not outlive it.
5. **Use `at()` during development** for bounds checking; switch to `operator[]` only when performance is critical and bounds are already verified.
6. **Leverage subviews (`first`, `last`, `subspan`) instead of manual pointer arithmetic**.
7. **Use `std::as_bytes()` / `std::as_writable_bytes()`** for binary operations.
8. **Prefer fixed extent when the size is known at compile‑time** – it enables compiler optimisations.
9. **Do not use `std::span` for null‑terminated strings** – use `std::string_view`.
10. **Document ownership/lifetime expectations** in your APIs.
11. **In C++23, explore `std::mdspan` for multi‑dimensional data** if needed, but treat it as a separate tool.

---
