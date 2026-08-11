# Modern C++ Replacements for `union`

In C, `union` allows storing different data types in the same memory space, but it comes with **no type safety**.

In modern C++, there are better, safer, and more powerful alternatives. Let's look at them:

---

## 1. `std::variant` — Type-safe Union (Best Replacement)

Introduced in **C++17**, `std::variant` is a **type-safe** replacement for `union`.  
It knows exactly **which type** is currently held and prevents **undefined behavior**.

### Example:

```cpp
#include <variant>
#include <iostream>
#include <string>

int main() {
    std::variant<int, float, std::string> data;

    data = 10;           // store an int
    data = 3.14f;        // now store a float
    data = "Hello";      // now store a string

    // Accessing the value safely
    if (std::holds_alternative<int>(data)) {
        std::cout << "Integer: " << std::get<int>(data) << '\n';
    }
    else if (std::holds_alternative<std::string>(data)) {
        std::cout << "String: " << std::get<std::string>(data) << '\n';
    }
}
