# `std::span` — What It Is And Why It Exists

## 1. The Problem Before `std::span`

Before C++20, if you wanted a function to work on "some contiguous block of data" — regardless of whether that data came from an array, a `vector`, or raw memory — you had two bad options:

**Option A: Raw pointer + size**
```cpp
void ProcessData(int* data, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        data[i] *= 2;
    }
}
```
Problems with this:
- Nothing stops you from passing a mismatched `size` (buffer overrun, silent bug)
- `data` could be `nullptr` and the function has no way to know
- No `.size()`, no iterators, no range-based `for` support
- You must repeat this pattern everywhere data is passed around

**Option B: Force everything into `std::vector`**
```cpp
void ProcessData(std::vector<int>& data);
```
Problems with this:
- Forces ownership/copy semantics onto something that should just be a *view*
- Doesn't work with `std::array`, C-style arrays, or raw buffers without copying
- A function that only needs to *read* data still ends up coupled to `std::vector`'s type

`std::span` was introduced to solve exactly this: **a way to describe "a view over contiguous elements" without caring where that memory actually lives.**

---

## 2. What `std::span` Actually Is (Under The Hood)

`std::span<T>` is **not a container**. It does not own, allocate, or free any memory.

Internally, it is nothing more than:
```cpp
template<typename T>
class Span
{
    T* pointer_;
    size_t size_;
};
```
That's it. Two machine words. No heap allocation, no reference counting, no hidden logic.

Because of this, a `std::span`:
- Can be constructed from a C array, `std::array`, `std::vector`, or a raw pointer + count
- Is cheap to copy (copying it copies two values, same cost as copying a pointer)
- Has zero relationship to the lifetime of the data it points to

This last point matters a lot — see the danger section below.

---

## 3. What Problem It Actually Solves

`std::span` gives you **one single parameter type** that can accept data from *any* contiguous source:

```cpp
void ProcessData(std::span<int> data)
{
    for (auto& element : data)
    {
        element *= 2;
    }
}

int c_array[5];
std::array<int, 5> std_array;
std::vector<int> vec(5);

ProcessData(c_array);   // works
ProcessData(std_array); // works
ProcessData(vec);       // works
```

You get, for free:
- `.size()`
- `.begin()` / `.end()` (so range-based `for` and STL algorithms work)
- `.first(n)`, `.last(n)`, `.subspan(offset, count)` — safe slicing without manual pointer math
- A single function signature instead of overloads for array / vector / pointer versions

In short: **it replaces the error-prone `(pointer, size)` pair with a type-safe, self-describing view**, without forcing an allocation or ownership model onto the caller.

---

## 4. Why It Exists (The Design Motivation)

The C++ standards committee added `std::span` because the language had a long-standing gap:

- `std::string_view` already solved this problem for character data (a non-owning view over a string).
- There was no equivalent for arbitrary element types (`int`, `float`, custom structs, etc.).

`std::span` is essentially **`string_view`'s generalization to any contiguous sequence.**

---

## 5. When To Prefer It

Use `std::span` when a function:
- Needs to **read or modify** a contiguous block of elements
- Doesn't need to **own** the data
- Should work regardless of whether the caller has an array, a vector, or a raw buffer

Prefer it over:
- `(pointer, size)` pairs — always, it is strictly safer and just as fast
- `std::vector&` parameters — when the function does not need vector-specific behavior (like `push_back`)

Do **not** use it for:
- Null-terminated strings — use `std::string_view` instead
- Anywhere ownership of the data itself needs to be transferred or managed

---

## 6. Cost — Is There Any?

**At runtime: no cost, when used correctly.**

- Constructing a `span` is just storing a pointer and a size — no allocation, no copy of the underlying elements
- Passing it by value costs exactly as much as passing two `size_t`-sized values
- Iteration and `operator[]` compile down to the same code as manual pointer indexing — the compiler optimizes the abstraction away entirely

**Where cost *can* appear:**
- Calling `.at(index)` instead of `operator[]` adds a runtime bounds check (a comparison + possible exception). This is a deliberate safety/performance trade-off you opt into, not something `span` forces on you.
- Fixed-extent spans (`std::span<int, N>`) carry zero extra runtime cost over dynamic-extent spans — the size is known at compile time and stored nowhere at runtime.

**The one real risk (not a performance cost, a correctness risk):**
Since a `span` does not own its data, it can easily become a **dangling view** if the underlying container is destroyed or resized while the span still exists:

```cpp
std::span<int> GetDanglingSpan()
{
    std::vector<int> local_data{1, 2, 3};
    return local_data;   // local_data is destroyed when the function returns
}                        // the returned span now points to freed memory
```

This is the trade-off for the abstraction being zero-cost: the compiler gives you no lifetime protection, so it's the programmer's responsibility to guarantee the underlying data outlives the span.

---

## Summary

| Aspect | `std::span` |
|---|---|
| Owns memory? | No |
| Underlying representation | Pointer + size (2 machine words) |
| Runtime cost | None (compiles down like raw pointer access) |
| Problem it solves | Unsafe `(pointer, size)` pairs, forced coupling to `std::vector` |
| Main risk | Dangling view if source data is destroyed early |
| Don't use for | Null-terminated strings (`string_view` is correct there) |
