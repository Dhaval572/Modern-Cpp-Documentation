# C++20/C++23 Ranges: Complete Guide (Beginner to Master)

## Table of Contents
1. [Introduction](#introduction)
2. [Level 1: Understanding the Problem](#level-1-understanding-the-problem)
3. [Level 2: Basic Views](#level-2-basic-views)
4. [Level 3: Composing Views](#level-3-composing-views)
5. [Level 4: Advanced Views](#level-4-advanced-views)
6. [Level 5: Range Generators](#level-5-range-generators)
7. [Level 6: Range Algorithms](#level-6-range-algorithms)
8. [Level 7: C++23 Range Features](#level-7-c23-range-features)
9. [Level 8: Real-World Examples](#level-8-real-world-examples)
10. [Level 9: Performance & Safety](#level-9-performance-and-safety)
11. [Level 10: Common Pitfalls & Best Practices](#level-10-common-pitfalls-and-best-practices)
12. [Level 11: Custom Range Adaptors](#level-11-custom-range-adaptors)
13. [Level 12: Advanced Patterns](#level-12-advanced-patterns)
14. [Key Takeaways](#key-takeaways)

---

## Introduction

C++20 Ranges revolutionize how we work with data in C++. C++23 makes them even better with crucial additions.

**What Ranges Provide:**
- ✅ **Composable operations** - Chain transformations together
- ✅ **Lazy evaluation** - Compute only when needed
- ✅ **Better readability** - Left-to-right pipeline flow
- ✅ **Type safety** - Compile-time checks
- ✅ **Efficiency** - No temporary containers
- ✅ **Safety** - No iterator pair mismatches

**C++23 Game Changers:**
- `std::ranges::to` - Easy materialization
- `zip`, `enumerate` - Multiple range handling
- `slide`, `chunk_by` - Window operations
- `as_const` - Const views
- `as_rvalue` - Move views

---

## Level 1: Understanding the Problem

### The Old Way (Pre-C++20)

```cpp
#include <vector>
#include <algorithm>
#include <iostream>

std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Task: Double each number, then keep only those > 10

// OLD WAY: Verbose, requires temporary containers
std::vector<int> temp1;
std::transform
(
    numbers.begin(),
    numbers.end(),
    std::back_inserter(temp1),
    [](int n)
    {
        return n * 2;
    }
);

std::vector<int> result;
std::copy_if
(
    temp1.begin(),
    temp1.end(),
    std::back_inserter(result),
    [](int n)
    {
        return n > 10;
    }
);

// Output: 12 14 16 18 20
```

**Problems:**
- ❌ Creates temporary container (`temp1`)
- ❌ Two separate steps
- ❌ Hard to read (right-to-left)
- ❌ Not efficient (extra memory allocation)

### The New Way (C++20 Ranges)

```cpp
#include <ranges>
#include <vector>
#include <iostream>

std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// NEW WAY: Clean, composable, lazy
auto result = numbers
    | std::views::transform
      (
          [](int n)
          {
              return n * 2;
          }
      )
    | std::views::filter
      (
          [](int n)
          {
              return n > 10;
          }
      );

for(int n : result)
{
    std::cout << n << " ";  // Output: 12 14 16 18 20
}
```

**Advantages:**
- ✅ No temporary containers
- ✅ Lazy evaluation (computed on-demand)
- ✅ Pipeline style (reads left-to-right)
- ✅ Composable operations

---

## Level 2: Basic Views

### 1. `filter` - Select elements matching condition

```cpp
std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Get only even numbers
auto evens = data | std::views::filter
(
    [](int n)
    {
        return n % 2 == 0;
    }
);

// Output: 2 4 6 8 10
```

### 2. `transform` - Apply function to each element

```cpp
// Square each number
auto squares = data | std::views::transform
(
    [](int n)
    {
        return n * n;
    }
);

// Output: 1 4 9 16 25 36 49 64 81 100
```

### 3. `take` - Take first N elements

```cpp
// Get first 5 elements
auto first_five = data | std::views::take(5);

// Output: 1 2 3 4 5
```

### 4. `drop` - Skip first N elements

```cpp
// Skip first 5 elements
auto skip_five = data | std::views::drop(5);

// Output: 6 7 8 9 10
```

### 5. `reverse` - Reverse order

```cpp
// Reverse the range
auto reversed = data | std::views::reverse;

// Output: 10 9 8 7 6 5 4 3 2 1
```

---

## Level 3: Composing Views

The real power of ranges comes from **composing** multiple views together!

### Example 1: Multiple Filters

```cpp
std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

// Numbers greater than 5 AND even
auto result = numbers
    | std::views::filter
      (
          [](int n)
          {
              return n > 5;
          }
      )
    | std::views::filter
      (
          [](int n)
          {
              return n % 2 == 0;
          }
      );

// Output: 6 8 10 12
```

### Example 2: Transform + Filter + Take

```cpp
// Square numbers, keep those > 50, take first 3
auto result = numbers
    | std::views::transform
      (
          [](int n)
          {
              return n * n;
          }
      )
    | std::views::filter
      (
          [](int n)
          {
              return n > 50;
          }
      )
    | std::views::take(3);

// Output: 64 81 100
```

### Example 3: Complex Pipeline

```cpp
// Drop 2, reverse, double, filter odd
auto result = numbers
    | std::views::drop(2)              // {3, 4, 5, ..., 12}
    | std::views::reverse              // {12, 11, 10, ..., 3}
    | std::views::transform
      (
          [](int n)
          {
              return n * 2;
          }
      )                                // {24, 22, 20, ...}
    | std::views::filter
      (
          [](int n)
          {
              return n % 2 != 0;
          }
      );                               // odd only

// Note: Result depends on the data flow!
```

---

## Level 4: Advanced Views

### 1. `take_while` - Take until condition fails

```cpp
std::vector<int> nums = {1, 2, 5, 3, 8, 4, 9};

// Take elements while < 6
auto taken = nums | std::views::take_while
(
    [](int n)
    {
        return n < 6;
    }
);

// Output: 1 2 5 3
// Stops at 8 because 8 >= 6
```

### 2. `drop_while` - Skip until condition fails

```cpp
// Skip elements while < 6
auto dropped = nums | std::views::drop_while
(
    [](int n)
    {
        return n < 6;
    }
);

// Output: 8 4 9
// Starts from 8 (first element >= 6)
```

### 3. `split` - Split range by delimiter

```cpp
std::string text = "Hello world from ranges";

// Split by spaces
auto words = text | std::views::split(' ');

for(auto word : words)
{
    for(char c : word)
    {
        std::cout << c;
    }
    std::cout << "\n";
}

// Output:
// Hello
// world
// from
// ranges
```

### 4. `join` - Flatten nested ranges

```cpp
std::vector<std::vector<int>> nested = {{1, 2}, {3, 4}, {5, 6}};

// Flatten to single range
auto flattened = nested | std::views::join;

// Output: 1 2 3 4 5 6
```

### 5. `keys` and `values` - For maps

```cpp
#include <map>

std::map<std::string, int> scores =
{
    {"Alice", 95},
    {"Bob", 87},
    {"Carol", 92}
};

// Extract just keys
auto names = scores | std::views::keys;
// Output: Alice Bob Carol

// Extract just values
auto points = scores | std::views::values;
// Output: 95 87 92
```

---

## Level 5: Range Generators

Views can also **generate** data, not just transform existing ranges!

### 1. `iota` - Generate sequence

```cpp
// Generate numbers 1 to 10
auto seq = std::views::iota(1, 11);

// Output: 1 2 3 4 5 6 7 8 9 10
```

### 2. `iota` unbounded - Infinite sequence

```cpp
// Infinite sequence starting from 1, but take only 10
auto infinite = std::views::iota(1) | std::views::take(10);

// Output: 1 2 3 4 5 6 7 8 9 10
```

### 3. `repeat` - Repeat value

```cpp
// Repeat value 42, five times
auto repeated = std::views::repeat(42) | std::views::take(5);

// Output: 42 42 42 42 42
```

### 4. `empty` - Empty range

```cpp
auto empty_range = std::views::empty<int>;

std::cout << "Size: " << std::ranges::distance(empty_range);
// Output: Size: 0
```

### 5. `single` - Single element

```cpp
auto single = std::views::single(100);

// Output: 100
```

---

## Level 6: Range Algorithms

C++20 also provides range-based algorithms that are safer and cleaner!

### Comparison: Old vs New

```cpp
std::vector<int> data = {5, 2, 8, 1, 9, 3, 7, 4, 6};

// OLD WAY
std::sort(data.begin(), data.end());

// NEW WAY - No iterators needed!
std::ranges::sort(data);

// Output: 1 2 3 4 5 6 7 8 9
```

### Common Range Algorithms

#### 1. `find_if`

```cpp
// Find first element > 5
auto it = std::ranges::find_if
(
    data,
    [](int n)
    {
        return n > 5;
    }
);

if(it != data.end())
{
    std::cout << "Found: " << *it;
}
```

#### 2. `count_if`

```cpp
// Count even numbers
auto count = std::ranges::count_if
(
    data,
    [](int n)
    {
        return n % 2 == 0;
    }
);

std::cout << "Even count: " << count;
```

#### 3. `any_of`, `all_of`, `none_of`

```cpp
bool has_large = std::ranges::any_of
(
    data,
    [](int n)
    {
        return n > 5;
    }
);
// true

bool all_positive = std::ranges::all_of
(
    data,
    [](int n)
    {
        return n > 0;
    }
);
// true

bool none_negative = std::ranges::none_of
(
    data,
    [](int n)
    {
        return n < 0;
    }
);
// true
```

#### 4. `copy_if`

```cpp
std::vector<int> odds;

// Copy odd numbers
std::ranges::copy_if
(
    data,
    std::back_inserter(odds),
    [](int n)
    {
        return n % 2 != 0;
    }
);

// odds: {1, 3, 5, 7, 9}
```

---

## Level 7: C++23 Range Features

### 1. `ranges::to` - Materialization Made Easy

**Before C++23:** Manual conversion
```cpp
auto view = data | std::views::filter(is_even);
std::vector<int> result(view.begin(), view.end());
```

**C++23:** Clean one-liner
```cpp
#include <ranges>  // C++23 feature

auto result = data
    | std::views::filter(is_even)
    | std::ranges::to<std::vector>();  // Easy!

// Can also specify allocator
auto vec = data | std::views::filter(is_even)
               | std::ranges::to<std::vector>(allocator);
```

### 2. `zip` - Multiple Ranges Simultaneously

```cpp
#include <ranges>  // C++23

std::vector<int> ids = {1, 2, 3, 4};
std::vector<std::string> names = {"Alice", "Bob", "Charlie", "Diana"};
std::vector<double> scores = {95.5, 87.0, 92.5, 88.5};

// Combine all three ranges - no more manual indexing!
for (auto [id, name, score] : std::views::zip(ids, names, scores))
{
    std::cout << "ID: " << id 
              << ", Name: " << name 
              << ", Score: " << score << "\n";
}
// Output:
// ID: 1, Name: Alice, Score: 95.5
// ID: 2, Name: Bob, Score: 87.0
// ID: 3, Name: Charlie, Score: 92.5
// ID: 4, Name: Diana, Score: 88.5
```

### 3. `enumerate` - Index + Value Pairs

```cpp
#include <ranges>  // C++23

std::vector<std::string> fruits = {"apple", "banana", "cherry", "date"};

// Get index and value together
for (auto [index, fruit] : std::views::enumerate(fruits))
{
    std::cout << index << ": " << fruit << "\n";
}
// Output:
// 0: apple
// 1: banana
// 2: cherry
// 3: date

// Works with any range
for (auto [i, n] : std::views::enumerate(std::views::iota(10, 15)))
{
    std::cout << "Position " << i << " = " << n << "\n";
}
```

### 4. `slide` - Sliding Window

```cpp
#include <ranges>  // C++23

std::vector<int> data = {1, 2, 3, 4, 5};

// Get sliding windows of size 3
for (auto window : data | std::views::slide(3))
{
    for (int n : window)
    {
        std::cout << n << " ";
    }
    std::cout << "\n";
}
// Output:
// 1 2 3
// 2 3 4
// 3 4 5
```

### 5. `chunk` and `chunk_by` - Grouping Elements

```cpp
#include <ranges>  // C++23

std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9};

// Fixed-size chunks
for (auto chunk : numbers | std::views::chunk(3))
{
    std::cout << "Chunk: ";
    for (int n : chunk)
    {
        std::cout << n << " ";
    }
    std::cout << "\n";
}
// Output:
// Chunk: 1 2 3
// Chunk: 4 5 6
// Chunk: 7 8 9

// Group by condition (chunk_by)
std::vector<int> mixed = {1, 1, 2, 3, 3, 3, 4, 4, 5};
for (auto chunk : mixed | std::views::chunk_by(std::equal_to{}))
{
    std::cout << "Group: ";
    for (int n : chunk)
    {
        std::cout << n << " ";
    }
    std::cout << "\n";
}
// Output:
// Group: 1 1
// Group: 2
// Group: 3 3 3
// Group: 4 4
// Group: 5
```

### 6. `as_const` and `as_rvalue` - Const and Move Views

```cpp
#include <ranges>  // C++23

std::vector<std::string> words = {"hello", "world", "c++23"};

// Create const view (can't modify elements)
auto const_view = words | std::views::as_const;
// *const_view.begin() = "test";  // Compile error!

// Create rvalue view (can move from elements)
auto rvalue_view = words | std::views::as_rvalue;
for (auto&& word : rvalue_view)
{
    // Can std::move from word if needed
    std::string moved = std::move(word);
}
```

### 7. `join_with` - Join with Separator

```cpp
#include <ranges>  // C++23

std::vector<std::string> parts = {"hello", "world", "c++23"};

// Join with separator
for (char c : parts | std::views::join_with(' '))
{
    std::cout << c;
}
// Output: hello world c++23

// Join with range separator
std::vector<std::vector<int>> nested = {{1, 2}, {3, 4}, {5, 6}};
auto joined = nested | std::views::join_with(std::array{-1, -1});
// Result: 1 2 -1 -1 3 4 -1 -1 5 6
```

### 8. `stride` - Take Every Nth Element

```cpp
#include <ranges>  // C++23

std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Take every 3rd element
auto strided = data | std::views::stride(3);

for (int n : strided)
{
    std::cout << n << " ";
}
// Output: 1 4 7 10
```

### 9. `cartesian_product` - Cartesian Product

```cpp
#include <ranges>  // C++23

std::array xs = {1, 2, 3};
std::array ys = {'A', 'B', 'C'};

// Generate all pairs (x, y)
for (auto [x, y] : std::views::cartesian_product(xs, ys))
{
    std::cout << "(" << x << ", " << y << ") ";
}
// Output: (1, A) (1, B) (1, C) (2, A) (2, B) (2, C) (3, A) (3, B) (3, C)
```

### 10. `adjacent` and `pairwise` - Adjacent Elements

```cpp
#include <ranges>  // C++23

std::vector<int> numbers = {1, 2, 4, 8, 16};

// Get adjacent elements as tuples
for (auto [a, b] : numbers | std::views::adjacent<2>)
{
    std::cout << a << "->" << b << " ";
}
// Output: 1->2 2->4 4->8 8->16

// pairwise is alias for adjacent<2>
for (auto [a, b] : numbers | std::views::pairwise)
{
    std::cout << "Diff: " << (b - a) << " ";
}
// Output: Diff: 1 Diff: 2 Diff: 4 Diff: 8
```

---

## Level 8: Real-World Examples

### Example 1: Data Processing Pipeline

```cpp
struct Student
{
    std::string name;
    int score;
};

std::vector<Student> students =
{
    {"Alice", 95}, {"Bob", 67}, {"Carol", 88},
    {"David", 72}, {"Eve", 91}, {"Frank", 55}
};

// Get top 3 passing students (score >= 70)
auto top_students = students
    | std::views::filter
      (
          [](const Student& s)
          {
              return s.score >= 70;
          }
      )
    | std::views::transform
      (
          [](const Student& s)
          {
              return s.name + ": " + std::to_string(s.score);
          }
      )
    | std::views::take(3);

// Output:
// Alice: 95
// Carol: 88
// David: 72

// C++23: Materialize easily
auto top_students_list = top_students | std::ranges::to<std::vector>();
```

### Example 2: Text Processing with C++23

```cpp
#include <ranges>  // C++23

std::string sentence = "The quick brown fox jumps over the lazy dog";

// Get words with more than 3 characters, join with commas
auto processed = sentence
    | std::views::split(' ')
    | std::views::filter
      (
          [](auto word)
          {
              return std::ranges::distance(word) > 3;
          }
      )
    | std::views::transform
      (
          [](auto word)
          {
              return std::string(word.begin(), word.end());
          }
      )
    | std::views::join_with(',')  // C++23
    | std::ranges::to<std::string>();  // C++23

// Output: quick,brown,jumps,over,lazy
```

### Example 3: Financial Data Analysis

```cpp
#include <ranges>  // C++23

struct StockData
{
    std::string symbol;
    double price;
    double change;
};

std::vector<StockData> stocks =
{
    {"AAPL", 150.25, 1.5},
    {"GOOGL", 2800.50, -5.2},
    {"MSFT", 305.75, 2.1},
    {"TSLA", 700.30, -12.5},
    {"AMZN", 3300.00, 0.8}
};

// Find stocks with positive change, sort by change descending
auto gainers = stocks
    | std::views::filter
      (
          [](const StockData& s)
          {
              return s.change > 0;
          }
      )
    | std::views::transform
      (
          [](const StockData& s)
          {
              return std::pair{s.symbol, s.change};
          }
      );

// C++23: Easy sorting and materialization
auto sorted_gainers = gainers
    | std::ranges::to<std::vector>()
    | std::ranges::sort
      (
          [](const auto& a, const auto& b)
          {
              return a.second > b.second;  // descending
          }
      );

// Output sorted gainers
for (auto [symbol, change] : sorted_gainers)
{
    std::cout << symbol << ": +" << change << "%\n";
}
```

### Example 4: Database Query Simulation

```cpp
#include <ranges>  // C++23

struct Employee
{
    int id;
    std::string name;
    std::string department;
    double salary;
    int years_of_service;
};

std::vector<Employee> employees =
{
    {101, "Alice", "Engineering", 95000, 5},
    {102, "Bob", "Sales", 75000, 3},
    {103, "Charlie", "Engineering", 110000, 8},
    {104, "Diana", "Marketing", 85000, 4},
    {105, "Eve", "Engineering", 105000, 7},
    {106, "Frank", "Sales", 70000, 2}
};

// Query: Get senior engineers with salary > 100k, sorted by experience
auto senior_engineers = employees
    | std::views::filter
      (
          [](const Employee& e)
          {
              return e.department == "Engineering" && e.salary > 100000;
          }
      )
    | std::views::transform
      (
          [](const Employee& e)
          {
              return std::tuple{e.name, e.salary, e.years_of_service};
          }
      )
    | std::ranges::to<std::vector>()  // C++23
    | std::ranges::sort
      (
          [](const auto& a, const auto& b)
          {
              return std::get<2>(a) > std::get<2>(b);  // sort by experience
          }
      );

// C++23: Use enumerate for ranking
for (auto [rank, data] : std::views::enumerate(senior_engineers))
{
    auto [name, salary, experience] = data;
    std::cout << rank + 1 << ". " << name
              << " ($" << salary
              << ", " << experience << " years)\n";
}
```

---

## Level 9: Performance & Safety

### Advantage 1: Const Correctness

```cpp
const std::vector<int> const_data = {1, 2, 3, 4, 5};

// This works! Views don't modify the source
auto doubled = const_data
    | std::views::transform
      (
          [](int n)
          {
              return n * 2;
          }
      );

// Output: 2 4 6 8 10
```

### Advantage 2: Lazy Evaluation

```cpp
std::vector<int> big_data(1000);
std::iota(big_data.begin(), big_data.end(), 1);

// Only processes until first match is found!
auto result = big_data
    | std::views::filter
      (
          [](int n)
          {
              std::cout << "[Checking " << n << "] ";
              return n > 995;
          }
      )
    | std::views::take(1);

for(int n : result)
{
    std::cout << "\nFound: " << n;
}

// Output shows it only checks: 996 997 998...
// Then stops! Doesn't check all 1000 elements.
```

### Advantage 3: Type Safety

```cpp
// Pipeline with type transformations
auto safe_pipeline = std::views::iota(1, 6)
    | std::views::transform
      (
          [](int n)
          {
              return n * 1.5;  // int -> double
          }
      )
    | std::views::filter
      (
          [](double d)
          {
              return d > 3.0;  // Correct type
          }
      );

// Compiler ensures type safety throughout!
```

### Advantage 4: No Iterator Mismatches

```cpp
// OLD WAY - Easy to make mistakes
std::sort(vec1.begin(), vec2.end());  // WRONG! Mixing vectors
std::copy(vec1.begin(), vec1.end(), vec2.begin() + 100);  // WRONG! Out of bounds

// NEW WAY - Can't make these mistakes
std::ranges::sort(vec1);  // Always correct
std::ranges::copy(vec1, vec2);  // Type-safe
```

---

## Level 10: Common Pitfalls & Best Practices

### ⚠️ Critical Pitfalls to Avoid

#### 1. **Dangling References (Most Common!)**

```cpp
// DANGEROUS: Returns a view to local data
auto get_even_numbers_bad()
{
    std::vector<int> local_data = {1, 2, 3, 4, 5};
    return local_data | std::views::filter
    (
        [](int n)
        {
            return n % 2 == 0;
        }
    );  // local_data is destroyed when function returns!
        // The view now points to destroyed memory!
}

// SAFE: Returns actual data (C++23)
auto get_even_numbers_good()
{
    std::vector<int> local_data = {1, 2, 3, 4, 5};
    return local_data
        | std::views::filter
          (
              [](int n)
              {
                  return n % 2 == 0;
              }
          )
        | std::ranges::to<std::vector>();  // Materialize before returning
}
```

#### 2. **View Invalidation**

```cpp
std::vector<int> numbers = {1, 2, 3, 4, 5};
auto even_view = numbers | std::views::filter(is_even);

numbers.push_back(6);  // May invalidate even_view!
numbers.erase(numbers.begin());  // Definitely invalidates!

// SAFER: Create view when you need it
void process_evens(const std::vector<int>& nums)
{
    for (int n : nums | std::views::filter(is_even))
    {
        // Safe: view created and used immediately
    }
}
```

#### 3. **Capturing by Reference in Lambdas**

```cpp
int threshold = 5;
auto above_threshold = numbers | std::views::filter
(
    [&threshold](int n)  // Capturing reference
    {
        return n > threshold;
    }
);

threshold = 10;  // Changes behavior of the view!

// For safety, capture by value or be aware of this
auto stable_filter = numbers | std::views::filter
(
    [threshold = 5](int n)  // Capture by value
    {
        return n > threshold;
    }
);
```

#### 4. **Not All Ranges Support All Operations**

```cpp
// Input ranges can only be traversed once
std::istringstream stream("1 2 3 4 5");
auto input_range = std::istream_iterator<int>{stream};

// This works once
for (int n : std::ranges::subrange(input_range, {}))
{
    std::cout << n << " ";
}

// This won't work - already consumed
for (int n : std::ranges::subrange(input_range, {}))
{
    std::cout << n << " ";  // No output!
}
```

### ✅ Best Practices

#### 1. **Use Descriptive Lambda Names**

```cpp
auto is_valid_score = [](int score) { return score >= 0 && score <= 100; };
auto calculate_grade = [](int score) { /* ... */ };

auto grades = scores
    | std::views::filter(is_valid_score)
    | std::views::transform(calculate_grade);
```

#### 2. **Keep Lambdas Simple**

```cpp
// BAD: Complex lambda
auto result = data | std::views::filter
(
    [](const auto& item)
    {
        if (item.type == Type::A && item.value > threshold)
        {
            if (item.timestamp > last_week)
            {
                return item.is_valid;
            }
        }
        return false;
    }
);

// GOOD: Extract complex logic
bool is_recent_valid_item(const Item& item, int threshold)
{
    if (item.type == Type::A && item.value > threshold)
    {
        if (item.timestamp > last_week)
        {
            return item.is_valid;
        }
    }
    return false;
}

auto result = data | std::views::filter(is_recent_valid_item);
```

#### 3. **Use `auto` for Range Types**

```cpp
// GOOD
auto filtered = data | std::views::filter(is_even);

// BAD - type is complex!
std::ranges::filter_view<std::ranges::transform_view<...>> filtered = ...;
```

#### 4. **Materialize When Needed (C++23)**

```cpp
// Materialize once, use multiple times
auto processed_data = raw_data
    | std::views::filter(is_valid)
    | std::views::transform(process_item)
    | std::ranges::to<std::vector>();  // C++23

// Now can use multiple times safely
std::ranges::sort(processed_data);
auto count = std::ranges::count_if(processed_data, is_special);
```

#### 5. **Combine with Structured Bindings**

```cpp
// With C++23 zip
for (auto [index, name, value] : std::views::zip(
    std::views::iota(1),
    names,
    values
))
{
    std::cout << index << ". " << name << ": " << value << "\n";
}
```

---

## Level 11: Custom Range Adaptors

### Simple Custom Adaptor Example

```cpp
// Create a reusable "strided" view (C++20 style)
auto stride = [](std::size_t step)
{
    return std::views::transform
    (
        [step, index = 0](auto&& value) mutable -> std::optional<decltype(value)>
        {
            if (index++ % step == 0)
            {
                return std::forward<decltype(value)>(value);
            }
            return std::nullopt;
        }
    )
    | std::views::filter
      (
          [](const auto& opt)
          {
              return opt.has_value();
          }
      )
    | std::views::transform
      (
          [](auto&& opt)
          {
              return *std::forward<decltype(opt)>(opt);
          }
      );
};

// Usage
std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
auto every_third = data | stride(3);
// Output: 1 4 7 10
```

### Custom Transform with State

```cpp
// Running average adaptor
auto running_average = [](std::size_t window_size)
{
    return std::views::transform
    (
        [window_size, history = std::vector<double>{}, sum = 0.0]
        (double value) mutable
        {
            history.push_back(value);
            sum += value;
            
            if (history.size() > window_size)
            {
                sum -= history.front();
                history.erase(history.begin());
            }
            
            return sum / history.size();
        }
    );
};

// Usage
std::vector<double> prices = {100.0, 102.0, 101.0, 103.0, 105.0, 104.0};
auto avg_prices = prices | running_average(3);
// Output: 100, 101, 101, 102, 103, 104 (running averages)
```

---

## Level 12: Advanced Patterns

### Pattern 1: Data Validation Pipeline

```cpp
#include <ranges>  // C++23

struct UserInput
{
    std::string username;
    std::string email;
    int age;
};

std::vector<UserInput> inputs = 
{
    {"alice123", "alice@example.com", 25},
    {"bob", "invalid-email", 17},
    {"charlie_very_long_name", "charlie@test.com", 30},
    {"diana", "diana@domain.com", -5}
};

// Validation pipeline
auto valid_users = inputs
    | std::views::filter
      (
          [](const UserInput& input)
          {
              return input.username.length() >= 3 
                  && input.username.length() <= 20;
          }
      )
    | std::views::filter
      (
          [](const UserInput& input)
          {
              return input.email.find('@') != std::string::npos;
          }
      )
    | std::views::filter
      (
          [](const UserInput& input)
          {
              return input.age >= 0 && input.age <= 150;
          }
      );

// C++23: Process with enumerate
for (auto [index, user] : std::views::enumerate(valid_users))
{
    std::cout << "Valid user #" << index + 1 
              << ": " << user.username << "\n";
}
```

### Pattern 2: Batch Processing

```cpp
#include <ranges>  // C++23

std::vector<int> data(1000);
std::iota(data.begin(), data.end(), 1);

// Process in batches of 100
for (auto batch : data | std::views::chunk(100))
{
    // Process batch
    auto batch_sum = std::ranges::fold_left(batch, 0, std::plus{});
    std::cout << "Batch sum: " << batch_sum << "\n";
    
    // Can also process batch items
    for (auto item : batch | std::views::filter(is_prime))
    {
        process_prime(item);
    }
}
```

### Pattern 3: Matrix Operations

```cpp
#include <ranges>  // C++23

// Represent matrix as vector of rows
std::vector<std::vector<double>> matrix = 
{
    {1.0, 2.0, 3.0},
    {4.0, 5.0, 6.0},
    {7.0, 8.0, 9.0}
};

// Get diagonal elements using zip and iota
auto diagonal = std::views::zip
(
    std::views::iota(0, (int)matrix.size()),
    matrix
)
| std::views::transform
  (
      [](auto&& pair)
      {
          auto [index, row] = pair;
          return row[index];
      }
  )
| std::ranges::to<std::vector>();  // C++23

// Get transpose using cartesian_product
auto transpose = std::views::cartesian_product
(
    std::views::iota(0, (int)matrix[0].size()),
    std::views::iota(0, (int)matrix.size())
)
| std::views::transform
  (
      [&matrix](auto&& pair)
      {
          auto [col, row] = pair;
          return matrix[row][col];
      }
  )
| std::views::chunk(matrix.size())  // Group into columns
| std::ranges::to<std::vector<std::vector<double>>>();  // C++23
```

---

## Key Takeaways

### 🎯 When to Use Ranges

**Use Ranges When:**
- You need to chain multiple transformations
- You want lazy evaluation for efficiency
- You're processing large datasets
- Readability and maintainability matter
- You want to avoid temporary containers

**Use Traditional Loops When:**
- You need complex control flow (multiple breaks/continues)
- You need to modify multiple variables simultaneously
- The logic is inherently imperative
- Performance is critical and you need fine-grained control

### 📊 C++23 vs C++20 Features

| Feature | C++20 | C++23 | Benefit |
|---------|-------|-------|---------|
| Materialization | Manual | `ranges::to` | One-liner conversion |
| Multiple ranges | Manual | `zip`, `cartesian_product` | Clean multi-range handling |
| Index access | Manual | `enumerate` | Automatic indexing |
| Windowing | Manual | `slide`, `adjacent` | Built-in window operations |
| Grouping | Limited | `chunk`, `chunk_by` | Flexible grouping |
| Join | Basic | `join_with` | Join with separators |
| Const safety | Manual | `as_const` | Automatic const views |

### ✨ Most Useful Views (C++23 Enhanced)

**Filtering & Selection:**
- `filter` - Select elements
- `take` / `drop` - First/skip N elements
- `take_while` / `drop_while` - Conditional selection

**Transformation:**
- `transform` - Map function over elements
- `join` / `join_with` - Flatten and join
- `split` - Split by delimiter

**Generation:**
- `iota` - Generate sequences
- `repeat` - Repeat value

**C++23 Specials:**
- `zip` - Combine multiple ranges
- `enumerate` - Add indices
- `slide` / `chunk` - Window operations
- `cartesian_product` - All combinations

**Algorithms:**
- `std::ranges::sort`
- `std::ranges::find_if`
- `std::ranges::count_if`
- `std::ranges::copy_if`

### 🚀 Best Practices Summary

1. **Start with C++20, upgrade to C++23 features**
   - Use `ranges::to` for materialization
   - Use `zip` and `enumerate` for cleaner code

2. **Pipeline Style for Readability**
   ```cpp
   auto result = data
       | view1
       | view2
       | view3;
   ```

3. **Materialize When Needed (C++23)**
   ```cpp
   auto vec = pipeline | std::ranges::to<std::vector>();
   ```

4. **Handle Dangling References**
   - Never return views to local data
   - Materialize before returning

5. **Use Structured Bindings with C++23**
   ```cpp
   for (auto [index, value] : std::views::enumerate(data)) { ... }
   for (auto [a, b, c] : std::views::zip(xs, ys, zs)) { ... }
   ```

6. **Profile Performance**
   - Ranges are usually efficient
   - But profile hot paths
   - Use raw loops only when proven faster

### 🎓 Learning Path

1. **Level 1-3**: Master basic views and composition
2. **Level 4-6**: Learn advanced views and algorithms
3. **Level 7**: Embrace C++23 features
4. **Level 8-9**: Apply to real-world problems
5. **Level 10**: Learn pitfalls and best practices
6. **Level 11-12**: Create custom adaptors and patterns

**Include Files:**
```cpp
#include <ranges>       // Core ranges
#include <algorithm>    // Range algorithms
#include <numeric>      // Range numeric operations
```

**Namespace:**
- `std::ranges` - Algorithms
- `std::views` - View adaptors (C++20 style)
- Many views also in `std::` directly in C++23
---

## Conclusion

C++20 Ranges + C++23 enhancements = **The future of C++ data processing**.

**Evolution:**
- **C++20**: Foundation - Views, algorithms, pipelines
- **C++23**: Polish - Easier usage, more utilities

**Benefits:**
- 🎨 **Elegant syntax** - Reads like natural language
- ⚡ **Performance** - Lazy evaluation, no temporaries
- 🛡️ **Safety** - Type checking, no iterator errors
- 🔧 **Composability** - Build complex transformations
- 📈 **Productivity** - Less code, fewer bugs

**Start Today:**
1. Begin with C++20 ranges in new projects
2. Gradually adopt C++23 features as compilers support them
3. Refactor old loops to ranges for clarity
4. Share knowledge with your team

---

**Happy Coding with Modern C++ Ranges! 🚀**
