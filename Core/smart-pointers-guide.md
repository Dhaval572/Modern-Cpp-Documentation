# Smart Pointers in C++

## Introduction
Smart pointers in C++ are advanced pointer constructs that manage memory automatically. They help in resource management by ensuring that dynamically allocated memory is properly released when no longer needed, thus preventing memory leaks and dangling pointers.

C++ provides smart pointers in the `<memory>` header.

## Types of Smart Pointers

### 1. `std::unique_ptr`
A `unique_ptr` is a smart pointer that owns and manages an object exclusively. When the `unique_ptr` goes out of scope, the object is automatically deleted.

#### Example:
```cpp
#include <iostream>
#include <memory>

class Test {
public:
    Test() { std::cout << "Test Created" << std::endl; }
    ~Test() { std::cout << "Test Destroyed" << std::endl; }
    void show() { std::cout << "Test Function" << std::endl; }
};

int main() {
    std::unique_ptr<Test> ptr1 = std::make_unique<Test>();
    ptr1->show();
    return 0;
}
```

#### Key Points:
- A `unique_ptr` cannot be copied, only moved.
- Use `std::move()` to transfer ownership.
- Best used when a single owner exists for a resource.

### 2. `std::shared_ptr`
A `shared_ptr` is a smart pointer that allows multiple `shared_ptr` instances to share ownership of the same object. The object is destroyed only when the last `shared_ptr` goes out of scope.

#### Example:
```cpp
#include <iostream>
#include <memory>

class Test {
public:
    Test() { std::cout << "Test Created" << std::endl; }
    ~Test() { std::cout << "Test Destroyed" << std::endl; }
    void show() { std::cout << "Test Function" << std::endl; }
};

int main() {
    std::shared_ptr<Test> ptr1 = std::make_shared<Test>();
    std::shared_ptr<Test> ptr2 = ptr1; // Shared ownership

    ptr1->show();
    return 0;
}
```

#### Key Points:
- Keeps a reference count of owners.
- Object is destroyed when count reaches zero.
- Suitable for shared ownership scenarios.

### 3. `std::weak_ptr`
A `weak_ptr` is a smart pointer that holds a non-owning reference to an object managed by `shared_ptr`. It is used to break circular dependencies.

#### Example:
```cpp
#include <iostream>
#include <memory>

class Test {
public:
    Test() { std::cout << "Test Created" << std::endl; }
    ~Test() { std::cout << "Test Destroyed" << std::endl; }
};

int main() {
    std::shared_ptr<Test> shared = std::make_shared<Test>();
    std::weak_ptr<Test> weak = shared; // Non-owning reference
    return 0;
}
```

#### Key Points:
- Does not contribute to reference count.
- Prevents cyclic references in `shared_ptr` chains.
- Must be converted to `shared_ptr` before use.

## When to Use Which Smart Pointer?
- Use `unique_ptr` when **sole ownership** is required.
- Use `shared_ptr` when **shared ownership** is needed.
- Use `weak_ptr` to **break cyclic dependencies** with `shared_ptr`.

## Conclusion
Smart pointers provide a powerful and safer alternative to raw pointers in C++. By using `unique_ptr`, `shared_ptr`, and `weak_ptr` appropriately, we can prevent memory leaks and dangling pointer issues while writing efficient and robust C++ programs.
