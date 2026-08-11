# Memory Management Rules

> **Description**: Core rules and best practices for managing memory in C++. Covers smart pointers, RAII, and common pitfalls.
> **Usage**: Refer to these rules when designing resource-owning classes or handling dynamic memory.
> **Prerequisites**: Basic understanding of C++ pointers and object lifecycle.

## 1. Use Smart Pointers Instead of Raw Pointers
- Prefer **`std::unique_ptr`** for exclusive ownership.
- Use **`std::shared_ptr`** for shared ownership.
- Avoid **`std::weak_ptr`** unless necessary to prevent circular references.

## 2. Allocate and Deallocate Memory Properly
- Always match `new` with `delete` and `new[]` with `delete[]`.

```cpp
int* p = new int(10);  // Allocation
delete p;              // Deallocation
```

## 3. Avoid Memory Leaks
- Ensure every `new` has a corresponding `delete`.
- Use **RAII (Resource Acquisition Is Initialization)** to manage resources safely.

```cpp
class Resource {
    int* data;
public:
    Resource() { data = new int[10]; }
    ~Resource() { delete[] data; }  // Prevent leaks
};
```

## 4. Avoid Dangling Pointers
- A pointer that references a deleted memory location is a **dangling pointer**.

```cpp
int* p = new int(5);
delete p;
p = nullptr;  // Set pointer to nullptr after deletion
```

## 5. Be Careful with Pointer Arithmetic
- Avoid out-of-bounds access.

```cpp
int arr[5];
int* ptr = arr;
ptr += 10;  // Undefined behavior (out of bounds)
```

## 6. Use `nullptr` Instead of `NULL`
- `nullptr` is type-safe, while `NULL` is just `0`.

```cpp
int* ptr = nullptr;  // Good practice
```

## 7. Avoid Memory Fragmentation
- Frequent dynamic allocation and deallocation can lead to fragmentation.
- Prefer **stack allocation** where possible.

```cpp
void func() {
    int x = 5;  // Stack allocation (fast and efficient)
}
```

## 8. Prefer `std::vector` Over Raw Arrays
- **`std::vector`** automatically manages memory.

```cpp
std::vector<int> v = {1, 2, 3, 4, 5};
```

## 9. Be Cautious with `reinterpret_cast` and `malloc/free`
- **Avoid `malloc/free` in C++**; use `new/delete` or smart pointers instead.
- **Avoid `reinterpret_cast` unless absolutely necessary**.

## 10. Use Memory Check Tools
- Tools like **Valgrind**, **AddressSanitizer**, and **Clang's LeakSanitizer** help detect memory issues.

By following these rules, you can write efficient, safe, and bug-free C++ programs. 🚀
