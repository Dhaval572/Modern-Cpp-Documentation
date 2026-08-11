# C++ Loops: The Complete Guide
## Beginner to Advanced — Use the Right Tool for the Job

> **Core principle:** No loop type is inherently "bad." Each exists for a reason. The best loop is the one that matches your use case with the least room for error.

---

## Table of Contents

1. [Quick Reference](#quick-reference)
2. [The Raw `for` Loop](#1-the-raw-for-loop)
3. [The Range-Based `for` Loop](#2-the-range-based-for-loop)
4. [While and Do-While](#3-while-and-do-while)
5. [std::for_each](#4-stdfor_each)
6. [std::ranges::for_each (C++20)](#5-stdrangesfor_each-c20)
7. [Performance: The Truth](#performance-the-truth)
8. [Common Pitfalls](#common-pitfalls)
9. [Professional Patterns](#professional-patterns)
10. [Decision Guide](#decision-guide)

---

## Quick Reference

| Loop | Best For | Avoid When |
|---|---|---|
| `for (int i = 0; ...)` | Index arithmetic, raw arrays, SIMD/DSP | Simple container traversal |
| `for (auto& x : c)` | Container traversal, clean code | You need the index (C++17 workaround exists) |
| `while` | Unknown iteration count, event loops | Fixed-count iteration |
| `std::for_each` | Named ops, parallel execution | Simple read-only iteration |
| `std::ranges::for_each` | Pipelines, view composition | Pre-C++20 codebases |

---

## 1. The Raw `for` Loop

```cpp
for (init; condition; increment) { /* body */ }
```

The raw `for` loop is **not unsafe by nature** — it is low-level, which means it gives you full control. With full control comes responsibility.

### When the Raw Loop is the Right Choice

#### Indexed access is the semantic intent
```cpp
// You need both the value and its position
void highlight_errors(const std::vector<std::string>& lines)
{
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (lines[i].empty())
            std::cerr << "Line " << i + 1 << " is empty\n";
    }
}
```

#### Raw arrays and pointer arithmetic
```cpp
// Range-based for works on arrays, but the raw loop is idiomatic
// when stride, offset, or manual bounds matter
void copy_stride(const float* src, float* dst, int count, int stride)
{
    for (int i = 0; i < count; ++i)
        dst[i] = src[i * stride];  // stride access — no range-based equivalent
}
```

#### Performance-critical inner loops (SIMD, DSP, game math)

This is where the raw loop **genuinely wins**. It compiles to direct pointer arithmetic with zero abstraction. Compilers auto-vectorize raw loops more reliably than iterator-based loops in many scenarios.

```cpp
// Hot path: element-wise vector multiply (compiler can auto-vectorize this)
void multiply_arrays(const float* a, const float* b, float* result, int n)
{
    for (int i = 0; i < n; ++i)
        result[i] = a[i] * b[i];
}
```

```cpp
// Two arrays with correlated indices — raw loop is the natural expression
float dot_product(const float* a, const float* b, int n)
{
    float sum = 0.0f;
    for (int i = 0; i < n; ++i)
        sum += a[i] * b[i];
    return sum;
}
```

#### Reverse iteration
```cpp
// Iterate backwards — clean and obvious
for (int i = static_cast<int>(vec.size()) - 1; i >= 0; --i)
    process(vec[i]);
```

#### Step sizes other than 1
```cpp
// Every other element — impossible to express cleanly with range-based for
for (size_t i = 0; i < data.size(); i += 2)
    process_pair(data[i]);
```

---

### Where the Raw Loop Goes Wrong

The raw loop is not broken — but it has common failure modes every C++ developer must know.

#### Off-by-one
```cpp
// Bug: <= should be <
for (int i = 0; i <= vec.size(); ++i)   // ❌ reads one past the end
    std::cout << vec[i];

// Fix
for (int i = 0; i < vec.size(); ++i)    // ✅
    std::cout << vec[i];
```

#### Signed/unsigned mismatch
```cpp
std::vector<int> data(100);

// Bug: int vs size_t — compiler warning, potential infinite loop if size > INT_MAX
for (int i = 0; i < data.size(); ++i)   // ❌ signed/unsigned comparison

// Fix: use size_t
for (size_t i = 0; i < data.size(); ++i)   // ✅

// Or: cast explicitly
for (int i = 0; i < static_cast<int>(data.size()); ++i)   // ✅ (when int is intentional)
```

#### Expensive end-condition recalculation
```cpp
const char* buffer = get_buffer();

// Bug: strlen() is O(n) — this makes the loop O(n²)
for (int i = 0; i < strlen(buffer); ++i)   // ❌

// Fix: cache it
const size_t len = strlen(buffer);
for (size_t i = 0; i < len; ++i)   // ✅
    process(buffer[i]);
```

#### Wrong index type for reverse loops
```cpp
// Bug: size_t is unsigned — wraps to SIZE_MAX when i hits 0
for (size_t i = vec.size() - 1; i >= 0; --i)   // ❌ infinite loop!

// Fix: use signed int, or restructure
for (int i = static_cast<int>(vec.size()) - 1; i >= 0; --i)   // ✅
```

---

### Safe Raw Loop Patterns

```cpp
// Pattern: cache size, use size_t, use prefix increment
const size_t count = items.size();
for (size_t i = 0; i < count; ++i)
{
    items[i].update();
}
```

```cpp
// Pattern: pointer-based loop (idiomatic for C-style arrays)
const int arr[] = {1, 2, 3, 4, 5};
const int* end = arr + std::size(arr);
for (const int* ptr = arr; ptr != end; ++ptr)
    std::cout << *ptr << '\n';
```

```cpp
// Pattern: safe span-based (C++20) — best of both worlds
void process(std::span<const int> data)
{
    for (size_t i = 0; i < data.size(); ++i)
        handle(data[i]);
}
```

---

## 2. The Range-Based `for` Loop

```cpp
for (declaration : range) { /* body */ }
```

Introduced in C++11. The compiler expands this to an iterator-based loop using `begin()` and `end()`. It is the **default choice** when you need every element and don't need the index.

### Forms

```cpp
std::vector<int> numbers = {1, 2, 3, 4, 5};

// Read-only (preferred when not modifying)
for (const auto& n : numbers)
    std::cout << n << '\n';

// Modify in-place
for (auto& n : numbers)
    n *= 2;

// Copy (expensive for non-trivial types — use only when you need a copy)
for (auto n : numbers)
    process_copy(n);

// Explicit type (when auto is ambiguous or misleading)
for (int n : numbers)
    std::cout << n << '\n';
```

### Works on Everything with begin()/end()

```cpp
// Raw arrays
int arr[] = {10, 20, 30};
for (int x : arr) std::cout << x << '\n';

// std::array
std::array<int, 3> std_arr = {1, 2, 3};
for (const auto& x : std_arr) process(x);

// Initializer list
for (int x : {1, 2, 3, 4, 5}) std::cout << x << '\n';

// std::string
for (char c : std::string("hello")) std::cout << c;

// Custom types with begin()/end()
for (const auto& item : my_custom_container) process(item);
```

### Getting the Index (C++17 Workaround)

```cpp
// C++17: init-statement in range-based for
for (size_t i = 0; const auto& item : container)
{
    std::cout << i << ": " << item << '\n';
    ++i;
}
```

### Reverse Iteration (C++20)

```cpp
#include <ranges>

for (const auto& item : std::views::reverse(container))
    process(item);
```

### Limitations

The range-based for always iterates the **entire range** from `begin()` to `end()`. For partial ranges, you need either a raw loop or a view (C++20).

```cpp
// Iterating only a subset — range-based for cannot do this directly
// Option 1: raw loop
for (size_t i = start; i < end; ++i) process(data[i]);

// Option 2: C++20 views
for (const auto& x : data | std::views::drop(start) | std::views::take(end - start))
    process(x);
```

---

## 3. While and Do-While

Use `while` when the iteration count is not known in advance.

```cpp
// while: condition checked before each iteration
while (condition) { /* body */ }

// do-while: body executes at least once
do { /* body */ } while (condition);
```

### When to Use `while`

```cpp
// Reading input until a sentinel value
int value;
while (std::cin >> value && value != -1)
    process(value);

// Iterator advancement with external logic
auto it = container.begin();
while (it != container.end() && !it->is_valid())
    ++it;
```

### When to Use `do-while`

```cpp
// Menu loop — must show menu at least once
int choice;
do
{
    show_menu();
    std::cin >> choice;
} while (choice != 0);

// Retry logic
int attempts = 0;
bool success;
do
{
    success = try_connect();
    ++attempts;
} while (!success && attempts < max_retries);
```

### `while` vs Raw `for`

A raw `for` with a blank increment is just a `while`:

```cpp
// These are equivalent
for (; condition; ) { /* body */ }
while (condition)   { /* body */ }
```

Prefer `while` when there is no natural init/increment. Prefer `for` when all three (init, condition, increment) belong together.

---

## 4. `std::for_each`

```cpp
#include <algorithm>

std::for_each(first, last, function);
```

### When to Use It

```cpp
// Named operation communicates intent
std::for_each(
    records.begin(), records.end(),
    [](auto& r) { r.normalize(); }
);
```

#### Parallel execution — the killer feature (C++17)

```cpp
#include <execution>

// Parallel: order of execution is unspecified, operations must be independent
std::for_each(
    std::execution::par,
    data.begin(), data.end(),
    [](auto& element) { element.heavy_compute(); }
);

// Parallel + vectorized
std::for_each(
    std::execution::par_unseq,
    data.begin(), data.end(),
    [](auto& element) { element.process(); }
);
```

This is the only standard loop with **built-in parallel execution support**. For large datasets, this can be transformative.

#### Partial ranges

```cpp
// Process elements 10 through 50 (exclusive)
std::for_each(
    data.begin() + 10,
    data.begin() + 50,
    [](auto& x) { x.update(); }
);
```

### What `std::for_each` Returns

Since C++11, it returns the function object passed in — useful for stateful functors:

```cpp
struct stats_collector
{
    int count = 0;
    double sum = 0.0;
    void operator()(double x) { ++count; sum += x; }
};

auto stats = std::for_each(
    values.begin(), values.end(),
    stats_collector{}
);
// stats.count, stats.sum are now populated
```

---

## 5. `std::ranges::for_each` (C++20)

```cpp
#include <algorithm>  // or <ranges>

std::ranges::for_each(range, function);
std::ranges::for_each(range, function, projection);
```

The ranges version accepts a **range** directly (no separate begin/end), prevents iterator mismatches, and supports projections.

### Prevents Iterator Mismatch Bugs

```cpp
std::vector<int> vec_a = {1, 2, 3};
std::vector<int> vec_b = {4, 5, 6};

// Classic bug: iterators from different containers — compiles, crashes at runtime
std::for_each(vec_a.begin(), vec_b.end(), fn);   // ❌ undefined behavior

// Ranges: this is impossible — only one range parameter
std::ranges::for_each(vec_a, fn);   // ✅
```

### Projections — Process a Member Without a Wrapper

```cpp
struct employee
{
    std::string name;
    double salary;
};

std::vector<employee> team = get_team();

// Classic: lambda wraps the member access
std::for_each(team.begin(), team.end(),
    [](const auto& e) { print(e.name); });

// Ranges: projection passes the member directly
std::ranges::for_each(team, print, &employee::name);
```

### View Pipelines

```cpp
// Process only senior employees with salary > 100k
std::ranges::for_each(
    team | std::views::filter([](const auto& e) { return e.salary > 100'000.0; }),
    [](const auto& e) { send_bonus(e); }
);
```

### Constrained by Concepts

The ranges algorithms use C++20 concepts to give you **better compile-time error messages** than classic STL algorithms. If your type doesn't satisfy the range concept, the error points to your code — not deep inside `<algorithm>`.

---

## Performance: The Truth

### What Actually Matters

In **optimized release builds** (`-O2`/`-O3`), all loop forms compile to nearly identical machine code. The compiler's optimizer erases the abstraction. The real performance factors are:

1. **Algorithmic complexity** — O(n) vs O(n²) dwarfs any loop-syntax choice
2. **Memory access patterns** — cache-friendly sequential access beats random access regardless of loop type
3. **Branch prediction** — predictable loops run faster than unpredictable ones
4. **Auto-vectorization** — the compiler must recognize the loop pattern

### When the Raw Loop Has a Real Performance Edge

The raw loop can genuinely outperform in specific scenarios:

#### 1. Auto-vectorization (SIMD)

Compilers (GCC, Clang, MSVC) auto-vectorize loops when they can prove no aliasing and a predictable access pattern. Raw loops over raw pointers or `std::span` give the compiler the clearest view.

```cpp
// This structure is highly vectorization-friendly
void add_arrays(const float* a, const float* b, float* c, int n)
{
    for (int i = 0; i < n; ++i)
        c[i] = a[i] + b[i];
    // Compiles to SIMD instructions (SSE/AVX) on x86
}
```

```cpp
// Using restrict hint (GCC/Clang extension) for maximum vectorization
void add_no_alias(const float* __restrict__ a,
                  const float* __restrict__ b,
                  float*       __restrict__ c,
                  int n)
{
    for (int i = 0; i < n; ++i)
        c[i] = a[i] + b[i];
}
```

#### 2. Custom stride and non-unit increments

```cpp
// Impossible to express in range-based for; raw loop is the natural form
for (int i = 0; i < n; i += cache_line_size)
    prefetch(&data[i + prefetch_distance]);
```

#### 3. Loop unrolling hints

```cpp
// Manual unroll — sometimes faster for tight inner loops
for (int i = 0; i + 4 <= n; i += 4)
{
    process(data[i]);
    process(data[i + 1]);
    process(data[i + 2]);
    process(data[i + 3]);
}
// Handle remainder
for (int i = (n / 4) * 4; i < n; ++i)
    process(data[i]);
```

### When Range-Based For is Equally Fast or Faster

For **container types** like `std::vector`, the range-based for loop is **equally fast** in release builds. The compiler generates the same pointer-based inner loop. For iterator types with non-trivial `operator++`, the compiler can sometimes optimize iterator loops better because it has type information through the container's iterator type.

```cpp
// These compile to identical assembly with -O2
for (size_t i = 0; i < vec.size(); ++i) process(vec[i]);
for (const auto& x : vec)              process(x);
```

### Benchmark Mindset

```
Rule: Profile first, optimize second.

If your loop is not in a hot path identified by a profiler,
the choice between loop types has zero impact on user experience.
```

```cpp
// The loop type here does not matter — it runs once
std::vector<std::string> config_lines = load_config();
for (const auto& line : config_lines)
    parse_config_line(line);
```

```cpp
// The loop type here may matter — called 60 times per second on 100k particles
void update_particles(particle* particles, int count, float dt)
{
    for (int i = 0; i < count; ++i)      // Raw: maximum vectorization opportunity
        particles[i].position += particles[i].velocity * dt;
}
```

### Benchmark Summary

| Scenario | Fastest Choice | Notes |
|---|---|---|
| Raw array, float math | Raw `for` | Best auto-vectorization |
| `std::vector` traversal | Tie | Identical with `-O2` |
| `std::map`/`std::list` | Range-based | Iterator is the natural unit |
| Large dataset, parallel | `std::for_each` + `par` | Unique capability |
| String/char processing | Range-based or raw | Depends on whether index needed |
| Custom stride | Raw `for` | Only option |

---

## Common Pitfalls

### Pitfall 1: Modifying a Container While Iterating

```cpp
std::vector<int> data = {1, 2, 3, 4, 5};

// ❌ Dangerous: erase() invalidates iterators and shifts indices
for (size_t i = 0; i < data.size(); ++i)
{
    if (data[i] % 2 == 0)
        data.erase(data.begin() + i);  // i now points to the wrong element
}

// ✅ Erase-remove idiom (C++11)
data.erase(
    std::remove_if(data.begin(), data.end(), [](int n) { return n % 2 == 0; }),
    data.end()
);

// ✅ std::erase_if (C++20)
std::erase_if(data, [](int n) { return n % 2 == 0; });
```

### Pitfall 2: Copying When You Mean to Reference

```cpp
std::vector<std::string> names = {"Alice", "Bob", "Charlie"};

// ❌ Copies each string — expensive
for (auto name : names)
    std::cout << name << '\n';

// ✅ Const reference — no copy
for (const auto& name : names)
    std::cout << name << '\n';
```

### Pitfall 3: Range-Based For Over a Temporary

```cpp
// ❌ Undefined behavior: temporary destroyed before loop runs
for (const auto& x : get_vector())   // get_vector() returns by value
    process(x);                       // x is a dangling reference

// ✅ Bind the temporary to a named variable first
auto vec = get_vector();
for (const auto& x : vec)
    process(x);

// ✅ Exception: range-based for extends lifetime of the range's temporary (C++23 fix pending)
// In C++23, this specific case is made safe. Until then, use the named variable.
```

### Pitfall 4: `i++` vs `++i` for Non-Primitive Types

```cpp
// For primitive types (int, size_t): identical after optimization
// For iterator types: ++it is potentially faster (no copy of the old value)

// Prefer prefix for iterators and custom types
for (auto it = container.begin(); it != container.end(); ++it)  // ✅ prefix
    process(*it);
```

### Pitfall 5: Unsigned Underflow in Reverse Loops

```cpp
std::vector<int> vec = {1, 2, 3};

// ❌ size_t wraps to SIZE_MAX when it hits 0 — infinite loop
for (size_t i = vec.size() - 1; i >= 0; --i)
    process(vec[i]);

// ✅ Option 1: cast to signed
for (int i = static_cast<int>(vec.size()) - 1; i >= 0; --i)
    process(vec[i]);

// ✅ Option 2: C++20 views::reverse
for (const auto& x : vec | std::views::reverse)
    process(x);
```

---

## Professional Patterns

### Pattern 1: Erase Elements Safely

```cpp
// Old (buggy with raw loop)
for (size_t i = 0; i < vec.size(); ++i)
    if (should_remove(vec[i])) vec.erase(vec.begin() + i--);  // fragile

// Modern (correct and expressive)
std::erase_if(vec, should_remove);  // C++20
```

### Pattern 2: Early Exit with Algorithms

```cpp
// Find the first matching element
auto it = std::find_if(
    records.begin(), records.end(),
    [](const auto& r) { return r.is_active(); }
);
if (it != records.end()) handle(*it);
```

### Pattern 3: Multi-Container Iteration

```cpp
std::vector<int>         ids     = {1, 2, 3};
std::vector<std::string> names   = {"A", "B", "C"};

// Safe: check both iterators
auto id_it   = ids.begin();
auto name_it = names.begin();
while (id_it != ids.end() && name_it != names.end())
{
    register_entry(*id_it, *name_it);
    ++id_it;
    ++name_it;
}

// C++23: std::views::zip (cleanest)
// for (const auto& [id, name] : std::views::zip(ids, names))
//     register_entry(id, name);
```

### Pattern 4: Safe Array Processing with `std::span`

```cpp
// Old: unsafe — caller can pass wrong size
void normalize(float* data, int count);

// New: span carries its size — range-based for just works
void normalize(std::span<float> data)
{
    float max_val = *std::max_element(data.begin(), data.end());
    for (auto& x : data)
        x /= max_val;
}
```

### Pattern 5: Structured Binding in Loops (C++17)

```cpp
std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}};

// Without structured binding
for (const auto& pair : scores)
    std::cout << pair.first << ": " << pair.second << '\n';

// With structured binding — much clearer
for (const auto& [name, score] : scores)
    std::cout << name << ": " << score << '\n';
```

### Pattern 6: Parallel Processing with `std::for_each`

```cpp
#include <execution>
#include <algorithm>

std::vector<image> frames = load_frames();

// Processes all frames in parallel across available CPU cores
std::for_each(
    std::execution::par_unseq,
    frames.begin(), frames.end(),
    [](auto& frame) { frame.apply_filter(); }
);
```

---

## Decision Guide

```
Do you need to iterate a collection?
│
├── Do you need the index or a non-unit stride?
│   ├── Yes → Raw for loop
│   └── No → Continue below
│
├── Do you need to iterate the whole container?
│   ├── Yes → Range-based for (default choice)
│   └── No → Raw for loop or std::views (C++20)
│
├── Is this a hot path (profiler confirmed)?
│   ├── Raw array / pointer math → Raw for loop
│   └── Container → Range-based for or raw for (identical performance)
│
├── Do you need parallel execution?
│   └── Yes → std::for_each with std::execution::par
│
├── Are you on C++20?
│   └── Yes → std::ranges::for_each for pipelines and projections
│
└── Unknown count / event-driven?
    └── while or do-while
```

---

## Checklist Before Writing a Loop

**Design**
- [ ] Is there an STL algorithm that already does this? (`std::transform`, `std::accumulate`, `std::count_if`, …)
- [ ] What are the exact boundary conditions?
- [ ] Will the container be modified during iteration?

**Types**
- [ ] Is the index type correct? (`size_t` for container sizes, `int`/`ptrdiff_t` for signed arithmetic)
- [ ] Are references used where copies are unnecessary?

**Safety**
- [ ] Is the end condition computed once, or re-evaluated each iteration?
- [ ] Is the loop tested with an empty container?
- [ ] Is the loop tested with a single element?

---

## Summary

The raw `for` loop is not dangerous — it is **precise**. It is the right tool when you need index arithmetic, raw pointer traversal, non-unit strides, or maximum auto-vectorization opportunity. It is the wrong tool when you simply want to visit every element of a `std::vector`.

The range-based `for` loop is not just a "safer" raw loop — it is a **different tool**. It expresses intent ("do this for each element") without exposing the mechanism. In optimized builds, it is as fast as a raw loop for container types.

```
Choose the loop that best expresses what you are doing.
Then let the compiler decide how to do it efficiently.
```

| Loop | Core Strength |
|---|---|
| Raw `for` | Index control, stride, pointer arithmetic, vectorization-friendly |
| Range-based `for` | Clean element-by-element traversal of any range |
| `while` / `do-while` | Condition-driven, event-driven, unknown count |
| `std::for_each` | Named operations, parallel execution |
| `std::ranges::for_each` | Safe pipelines, projections, type-constrained (C++20+) |

---

*Based on the ISO C++ Standard, cppreference.com, and the C++ Core Guidelines.*  
*Covers C++11 through C++23.*
