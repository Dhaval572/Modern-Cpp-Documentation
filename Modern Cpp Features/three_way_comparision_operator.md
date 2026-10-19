# C++20 Three-Way Comparison Operator (`<=>`)

The three-way comparison operator, nicknamed the **spaceship operator**, was added in C++20. One expression tells you whether `a` is less than, equal to, or greater than `b`.

---

## 1. The Problem It Solves

### 1.1 Six operators for one idea

Before C++20, making a type fully comparable meant writing **six** operators: `==`, `!=`, `<`, `<=`, `>`, `>=`.

```cpp
class Point
{
public:
    int x;
    int y;

    bool operator==(const Point& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator<(const Point& other) const
    {
        if (x != other.x)
        {
            return x < other.x;
        }
        return y < other.y;
    }

    // ...and !=, <=, >, >= still to write
};
```

### 1.2 Root causes of the pain

| Problem | Why it happens |
|---|---|
| **Boilerplate** | Each operator is a separate function, even though all six derive from one ordering. |
| **Bugs** | Hand-written chains like `x != other.x ... y < other.y` are easy to get wrong, and forgetting one member silently breaks ordering. |
| **Inconsistency** | Nothing forces `<` and `==` to agree, so `a < b`, `a == b` and `a > b` can all be false. |
| **Maintenance** | Adding a member means updating up to six functions. |
| **Double work** | Using `<` then `==` (or `<` twice) to decide three outcomes walks the data twice. |
| **No standard three-way** | `strcmp`, `std::string::compare` and `memcmp` return negative/zero/positive ints, which is an ad-hoc convention with no type safety and no way to express "unordered" (NaN). |

### 1.3 What C++20 gives you

```cpp
class Point
{
public:
    int x;
    int y;

    auto operator<=>(const Point& other) const = default;
};
```

This one line generates **all six** operators, comparing members in declaration order (lexicographically).

---

## 2. What `a <=> b` Actually Returns

It does not return `bool` or `int`. It returns a **comparison category object**, which you compare against the literal `0`.

```cpp
#include <compare>
#include <iostream>

int main()
{
    int first_value = 5;
    int second_value = 9;

    auto result = first_value <=> second_value;

    if (result < 0)
    {
        std::cout << "first is less\n";
    }
    else if (result > 0)
    {
        std::cout << "first is greater\n";
    }
    else
    {
        std::cout << "equal\n";
    }
}
```

Here `result` is a `std::strong_ordering`. You can only compare it with the literal `0`, never with another int variable. This blocks the old `strcmp(...) == 1` bug, where the result was assumed to be exactly 1.

---

## 3. The Three Comparison Categories

| Category | Possible values | Meaning | Typical types |
|---|---|---|---|
| `std::strong_ordering` | `less`, `equal`, `greater` | Equal values are **fully substitutable** | `int`, `char`, pointers, `std::string` |
| `std::weak_ordering` | `less`, `equivalent`, `greater` | Equivalent values may still **look different** | Case-insensitive strings |
| `std::partial_ordering` | `less`, `equivalent`, `greater`, `unordered` | Some pairs **cannot be compared** | `float`, `double` (NaN) |

Strength order: **strong → weak → partial**. A stronger category converts implicitly to a weaker one, but never the other way round.

### 3.1 Why `partial_ordering` exists

`NaN` is not less than, equal to, or greater than anything. Forcing a two-way answer would lie, so the type system gives you a fourth outcome:

```cpp
#include <compare>
#include <limits>

int main()
{
    double nan_value = std::numeric_limits<double>::quiet_NaN();
    auto result = 1.0 <=> nan_value;

    if (result == std::partial_ordering::unordered)
    {
        // comparison is meaningless, handle it explicitly
    }
}
```

### 3.2 Helper functions

Instead of `result < 0`, you can write named checks that read better:

```cpp
std::is_lt(result);   // less
std::is_lteq(result); // less or equal
std::is_eq(result);   // equal
std::is_neq(result);  // not equal
std::is_gt(result);   // greater
std::is_gteq(result); // greater or equal
```

---

## 4. Defaulted `operator<=>` (The Common Case)

```cpp
#include <compare>
#include <string>

class Version
{
public:
    int major;
    int minor;
    int patch;

    auto operator<=>(const Version& other) const = default;
};

int main()
{
    Version older{1, 4, 2};
    Version newer{1, 5, 0};

    bool is_older = older < newer;     // true
    bool is_same = older == newer;     // false
    bool is_newer_or_same = newer >= older; // true
}
```

### How it works

- Members are compared **in declaration order**, stopping at the first difference.
- Order matters: put `major` before `minor` before `patch`.
- A defaulted `<=>` also **implicitly declares a defaulted `==`**, so `==` and `!=` work too.
- If any member lacks `<=>` or `==`, the defaulted operator is defined as **deleted** (compile error on use).

### The return type is deduced

With `auto`, the category is the **weakest** one among all members:

```cpp
class Measurement
{
public:
    int id;          // strong_ordering
    double value;    // partial_ordering

    auto operator<=>(const Measurement& other) const = default;
    // deduced: std::partial_ordering
};
```

Declare the type explicitly (`std::strong_ordering operator<=>(...) const = default;`) when you want the compiler to **reject** members that would weaken your guarantee.

---

## 5. Custom `operator<=>`

Write your own when the order is not plain member order, for example sorting salary **descending**, then name ascending.

```cpp
#include <compare>
#include <string>

class Employee
{
public:
    Employee(std::string employee_name, int employee_salary)
        : name(std::move(employee_name)), salary(employee_salary)
    {
    }

    std::strong_ordering operator<=>(const Employee& other) const
    {
        if (auto result = other.salary <=> salary; result != 0)
        {
            return result; // reversed operands -> descending salary
        }
        return name <=> other.name;
    }

    bool operator==(const Employee& other) const = default;

private:
    std::string name;
    int salary;
};
```

### Critical rule

> If you **write** `operator<=>` yourself, the compiler does **not** generate `==`. You must declare it, usually `= default`.

Root cause: equality can often be answered faster than ordering. For example, strings of different sizes are unequal without comparing any characters. So the language keeps `==` separate from `<=>` instead of deriving it from the ordering.

---

## 6. How the Compiler Uses It (Rewrite Rules)

For `a < b`, the compiler considers a rewritten form:

```
a < b   →   (a <=> b) < 0
a > b   →   (a <=> b) > 0
a <= b  →   (a <=> b) <= 0
a >= b  →   (a <=> b) >= 0
```

It also tries **reversed** forms such as `0 < (b <=> a)`. That is why one operator handles mixed-type comparisons in **both directions**:

```cpp
class Money
{
public:
    int cents;

    std::strong_ordering operator<=>(int other_cents) const
    {
        return cents <=> other_cents;
    }

    bool operator==(int other_cents) const
    {
        return cents == other_cents;
    }
};

int main()
{
    Money price{500};

    bool a = price < 700;  // direct
    bool b = 700 > price;  // works through reversed rewrite
}
```

Pre-C++20 you would have needed separate overloads for each operand order.

`!=` is rewritten from `==` (as `!(a == b)`), not from `<=>`.

---

## 7. Library Support

| Tool | Purpose |
|---|---|
| `std::compare_three_way` | Function object, the `<=>` equivalent of `std::less` |
| `std::lexicographical_compare_three_way` | Three-way compare two ranges |
| `std::string`, `std::vector`, `std::pair`, `std::tuple`, `std::array`, `std::optional` | Provide `<=>` out of the box |
| `std::strong_order`, `std::weak_order`, `std::partial_order` | Customization-point functions; `std::strong_order` gives a **total** order even for floats |
| `std::common_comparison_category_t<...>` | Computes the weakest category among types |

```cpp
#include <algorithm>
#include <compare>
#include <string>
#include <vector>

int main()
{
    std::vector<std::string> names{"mia", "arun", "zoe"};
    std::ranges::sort(names, std::compare_three_way{}); // sorted via <=>
}
```

---

## 8. Real-World Use Cases

### 8.1 Value types with many fields

Dates, versions, coordinates and IDs get all six operators from one defaulted line (section 4).

### 8.2 Keys for ordered containers

`std::set`, `std::map` and `std::priority_queue` need ordering.

```cpp
#include <map>
#include <string>

class DateKey
{
public:
    int year;
    int month;
    int day;

    auto operator<=>(const DateKey& other) const = default;
};

int main()
{
    std::map<DateKey, std::string> events;
    events[{2026, 10, 19}] = "release day";
}
```

### 8.3 Sorting

```cpp
#include <algorithm>
#include <vector>

int main()
{
    std::vector<Version> releases{{1, 5, 0}, {1, 4, 2}, {2, 0, 0}};
    std::ranges::sort(releases); // uses <=> automatically
}
```

### 8.4 One-pass three-way decisions (search trees, binary search)

When the code must branch three ways, `<=>` computes the comparison **once** instead of twice.

```cpp
struct Node
{
    int key;
    Node* left;
    Node* right;
};

Node* Find(Node* current_node, int target_key)
{
    while (current_node != nullptr)
    {
        auto order = target_key <=> current_node->key;

        if (order < 0)
        {
            current_node = current_node->left;
        }
        else if (order > 0)
        {
            current_node = current_node->right;
        }
        else
        {
            return current_node;
        }
    }
    return nullptr;
}
```

This matters most for expensive comparisons such as long strings or big composite keys.

### 8.5 Multi-field custom ranking

Leaderboards, task schedulers (priority, then deadline), search-result ranking. See section 5 for the pattern: chain `<=>` calls and return early at the first non-equal result.

### 8.6 Generic code

Templates can constrain on the concept `std::three_way_comparable<T>` to require that a type supports `<=>`.

```cpp
#include <compare>
#include <concepts>

template <std::three_way_comparable T>
const T& Largest(const T& first_item, const T& second_item)
{
    return (first_item < second_item) ? second_item : first_item;
}
```

### 8.7 Strongly typed wrappers

Wrap raw ints into `UserId`, `OrderId` and similar types so they cannot be mixed up, with comparisons costing one defaulted line.

---

## 9. Common Pitfalls

| Pitfall | Cause and fix |
|---|---|
| `==` missing after a custom `<=>` | `==` is only auto-generated for a **defaulted** `<=>`. Add `bool operator==(const T&) const = default;` |
| Defaulted `<=>` won't compile | A member has no `<=>` / `==` (e.g. a raw struct or a `std::function`). Add the operator to that member's type. |
| Unexpected `partial_ordering` | A `float`/`double` member weakens the deduced category. Declare the return type explicitly if you need stronger. |
| Wrong order of results | Defaulted comparison follows **declaration order**. Reorder members, or write a custom operator. |
| `result == 1` does not compile | By design. Compare against literal `0` only. |
| Hash containers fail | `std::unordered_map` needs `==` and a hash, not `<=>`. |
| Pointer members compare addresses | Defaulted `<=>` on a pointer compares the pointer value, not the pointee. Write a custom one if you need deep comparison. |
| Old compiler errors | Needs C++20 and `<compare>`. Check `-std=c++20`. |

---

## 10. Under the Hood (How It Works Internally)

`<=>` looks like magic, but it is built from ordinary C++: small classes, operator overloads, and a few compiler rewrite rules. The code below is a **simplified model** of what `<compare>` and the compiler do. Real standard libraries (libstdc++, libc++, MSVC STL) follow the same idea with more `constexpr` and `noexcept` plumbing.

### 10.1 Comparison categories are tiny classes

`std::strong_ordering` is a class that stores a single small integer (`-1`, `0`, `1`) and exposes the named constants `less`, `equal` and `greater`.

```cpp
class ZeroLiteral
{
public:
    // Only a literal 0 (a null pointer constant) converts to ZeroLiteral*
    constexpr ZeroLiteral(ZeroLiteral*) noexcept
    {
    }
};

class StrongOrdering
{
public:
    static const StrongOrdering less;
    static const StrongOrdering equal;
    static const StrongOrdering greater;

    friend constexpr bool operator==(StrongOrdering left, StrongOrdering right) = default;

    friend constexpr bool operator==(StrongOrdering value, ZeroLiteral)
    {
        return value.raw_value == 0;
    }

    friend constexpr bool operator<(StrongOrdering value, ZeroLiteral)
    {
        return value.raw_value < 0;
    }

    friend constexpr bool operator<(ZeroLiteral, StrongOrdering value)
    {
        return 0 < value.raw_value;
    }

    // operator>, operator<=, operator>= follow the same pattern

private:
    constexpr explicit StrongOrdering(signed char raw_value_in)
        : raw_value(raw_value_in)
    {
    }

    signed char raw_value;
};

inline constexpr StrongOrdering StrongOrdering::less(-1);
inline constexpr StrongOrdering StrongOrdering::equal(0);
inline constexpr StrongOrdering StrongOrdering::greater(1);
```

Why it is designed this way:

| Design choice | Root cause / benefit |
|---|---|
| Comparison operators take a `ZeroLiteral` parameter | Only the literal `0` converts to it, so `result < 0` compiles but `result < some_int` and `result == 1` do not. This removes the old "strcmp returns exactly 1" bug. |
| Private constructor | Nobody can fabricate an ordering from a random int. The only valid values are the three named constants. |
| One byte of storage | Cheap to return in a register and cheap to test. |
| `WeakOrdering` / `PartialOrdering` have the same shape | `PartialOrdering` adds a fourth, implementation-defined value for `unordered`, and comparisons against `0` return `false` for it. |

**Conversions** are what make "strong → weak → partial" work. `StrongOrdering` defines `operator WeakOrdering()` and `operator PartialOrdering()`, which map `less/equal/greater` to the matching values. `WeakOrdering` can convert to `PartialOrdering`. No conversion goes the other way.

### 10.2 What a defaulted `<=>` expands to

`= default` makes the compiler generate a memberwise, short-circuiting comparison. For this class:

```cpp
class Point
{
public:
    int x;
    int y;

    auto operator<=>(const Point& other) const = default;
};
```

the generated code behaves like:

```cpp
class Point
{
public:
    int x;
    int y;

    std::strong_ordering operator<=>(const Point& other) const
    {
        if (auto result = x <=> other.x; result != 0)
        {
            return result;
        }
        return y <=> other.y; // last member: return its result directly
    }

    bool operator==(const Point& other) const
    {
        return x == other.x && y == other.y;
    }
};
```

- Base classes are compared first (in declaration order), then members.
- It stops at the first non-equal member, so later members are never touched.
- The implicit `==` is generated separately and uses `==` on each member, not `<=>`. That lets it exit early when sizes differ (for example `std::vector`, `std::string`).

### 10.3 How the compiler picks the operator (rewrite rules)

For an expression such as `a < b`, overload resolution collects **three kinds** of candidates:

```
a < b
 ├─ operator<(A, B)                      normal candidate (preferred)
 ├─ operator<=>(A, B)   →  (a <=> b) < 0         rewritten
 └─ operator<=>(B, A)   →  0 < (b <=> a)         rewritten + reversed
```

Tie-breaking rules, in order: a normal candidate beats a rewritten one, and a non-reversed candidate beats a reversed one.

Equivalent expansion by hand:

```cpp
Point first_point{1, 2};
Point second_point{1, 5};

bool is_less = first_point < second_point;
// compiler rewrites to:
bool is_less_expanded = (first_point <=> second_point) < 0;

bool is_not_equal = first_point != second_point;
// compiler rewrites to:
bool is_not_equal_expanded = !(first_point == second_point);
```

Note that `!=` comes from `==`, never from `<=>`. For mixed types, `a == b` can also be tried as the reversed `b == a`.

### 10.4 How the return type is deduced

When you write `auto operator<=>(...) const = default;`, the compiler applies `std::common_comparison_category` to the result type of every member's `<=>`:

| Types present among members | Deduced category |
|---|---|
| all strong | `strong_ordering` |
| strong + weak | `weak_ordering` |
| any partial | `partial_ordering` |
| a member with no `<=>` | none, so the defaulted operator is **deleted** |

```cpp
#include <compare>
#include <type_traits>

static_assert(std::is_same_v<
    std::common_comparison_category_t<std::strong_ordering, std::partial_ordering>,
    std::partial_ordering>);
```

### 10.5 The fallback used by the library (`synth-three-way`)

Standard types like `std::pair`, `std::tuple` and `std::optional` must also work with element types that only have `<` (old code). The standard describes a helper, often called `synth-three-way`, that uses `<=>` when available and falls back to `<` otherwise. A simplified version:

```cpp
#include <compare>
#include <concepts>

template <typename ValueType>
constexpr auto SynthThreeWay(const ValueType& left, const ValueType& right)
{
    if constexpr (std::three_way_comparable<ValueType>)
    {
        return left <=> right;
    }
    else
    {
        if (left < right)
        {
            return std::weak_ordering::less;
        }
        if (right < left)
        {
            return std::weak_ordering::greater;
        }
        return std::weak_ordering::equivalent;
    }
}
```

The fallback can only prove `less`/`greater`/"neither", so it returns `weak_ordering`. That is the root reason a pair of old-style types gets a weaker category.

### 10.6 How containers compare (lexicographic three-way)

`std::vector<T>::operator<=>` is essentially a three-way lexicographic loop:

```cpp
#include <compare>

template <typename Iterator, typename CompareFunction>
constexpr auto LexicographicalThreeWay(
    Iterator first_begin, Iterator first_end,
    Iterator second_begin, Iterator second_end,
    CompareFunction compare_function)
    -> decltype(compare_function(*first_begin, *second_begin))
{
    for (; first_begin != first_end && second_begin != second_end;
         ++first_begin, ++second_begin)
    {
        if (auto result = compare_function(*first_begin, *second_begin); result != 0)
        {
            return result;
        }
    }

    // All shared elements equal: the shorter sequence is "less"
    bool first_has_more = first_begin != first_end;
    bool second_has_more = second_begin != second_end;
    return first_has_more <=> second_has_more;
}
```

Usage:

```cpp
std::vector<int> first_list{1, 2, 3};
std::vector<int> second_list{1, 2, 4};

auto result = LexicographicalThreeWay(
    first_list.begin(), first_list.end(),
    second_list.begin(), second_list.end(),
    std::compare_three_way{}); // result == std::strong_ordering::less
```

### 10.7 Cost at runtime

- For built-in types, `a <=> b` is not a function call. It is a plain compare instruction plus a little logic to produce `-1/0/1`.
- When you only test the result against `0` (as every rewritten `<`, `>`, `<=`, `>=` does), the optimizer typically reduces `(a <=> b) < 0` to the same machine code as `a < b`. You do not pay for the extra abstraction.
- The real saving shows when a comparison is expensive and you need three outcomes: `<=>` computes it once, where `<` followed by `>` would run it twice.

---

## 11. Quick Cheat Sheet

```cpp
// 1. Simple value type: everything for free
auto operator<=>(const T&) const = default;

// 2. Custom ordering: also declare ==
std::strong_ordering operator<=>(const T& other) const;
bool operator==(const T& other) const = default;

// 3. Using the result
auto result = a <=> b;
result < 0;            // a < b
std::is_eq(result);    // a == b
result == std::partial_ordering::unordered; // NaN case
```

| You want | Use |
|---|---|
| All six operators, member-wise | `= default` |
| Custom order (descending, case-insensitive) | Hand-written `<=>` + defaulted `==` |
| Floats with NaN handling | `std::partial_ordering` |
| Total order on floats | `std::strong_order(a, b)` |
| One comparison, three branches | `auto order = a <=> b;` |

---

## Summary

- **Problem:** six hand-written operators, inconsistent and bug-prone.
- **Solution:** one `operator<=>` returning a typed comparison category; the compiler derives the rest.
- **Rule to remember:** default `<=>` gives you `==`; custom `<=>` does not.
- **Under the hood:** comparison categories are small classes that only compare against literal `0`; the compiler rewrites `a < b` into `(a <=> b) < 0` (or the reversed form), and a defaulted `<=>` expands to a memberwise, short-circuiting comparison.
- **Best uses:** value types, container keys, sorting, one-pass tree or binary searches, and generic code.
