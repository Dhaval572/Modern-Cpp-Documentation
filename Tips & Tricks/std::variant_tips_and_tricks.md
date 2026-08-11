# C++ Variants: Complete Tips and Tricks Guide

`std::variant` is a type-safe union introduced in C++17 that can hold one of several specified types at any given time. It's a powerful alternative to traditional unions and provides compile-time type safety with runtime flexibility.

## Table of Contents

- [Getting Started](#getting-started)
- [Essential Methods and Operations](#essential-methods-and-operations)
- [Advanced Techniques](#advanced-techniques)
- [Performance Optimization](#performance-optimization)
- [Real-World Use Cases](#real-world-use-cases)
- [Common Pitfalls](#common-pitfalls)
- [Best Practices](#best-practices)

## Getting Started

To use `std::variant`, include the `<variant>` header. A variant is declared by specifying all possible types it can hold:

```cpp
#include <variant>
#include <string>
#include <iostream>

// Basic variant declaration
std::variant<int, double, std::string> my_data;

// Variant with more complex types
std::variant<std::vector<int>, std::map<std::string, int>, std::set<double>> container_variant;
```

### Assignment and Type Changing

One of variant's most convenient features is that you can simply assign values of any held type, and it automatically manages the active alternative:

```cpp
#include <variant>
#include <string>
#include <iostream>

int main() {
    std::variant<int, double, std::string> my_data;
    
    my_data = 42;              // holds int
    std::cout << "Holds int, index: " << my_data.index() << '\n';
    
    my_data = 3.14;            // now holds double
    std::cout << "Holds double, index: " << my_data.index() << '\n';
    
    my_data = "hello";         // now holds std::string
    std::cout << "Holds string, index: " << my_data.index() << '\n';
    
    return 0;
}
```

**Output:**
```
Holds int, index: 0
Holds double, index: 1
Holds string, index: 2
```

The variant destructs the old value and constructs the new one automatically.

## Essential Methods and Operations

### 1. std::get - Type-Safe Value Access

`std::get` provides type-safe access to the variant's value. It throws `std::bad_variant_access` if you request the wrong type:

```cpp
#include <variant>
#include <string>
#include <iostream>

void DemonstrateSafeGet() {
    std::variant<int, double, std::string> my_variant = 42;
    
    try {
        // Correct type access
        int int_value = std::get<int>(my_variant);
        std::cout << "Integer value: " << int_value << '\n';
        
        // Wrong type access - throws exception
        double double_value = std::get<double>(my_variant);
    } 
    catch (const std::bad_variant_access& exception) {
        std::cout << "Error: " << exception.what() << '\n';
    }
    
    // Index-based access
    auto value_by_index = std::get<0>(my_variant);  // Gets first alternative type
    std::cout << "Value by index: " << value_by_index << '\n';
}
```

### 2. std::holds_alternative - Type Checking

Check which type is currently stored before accessing:

```cpp
#include <variant>
#include <string>
#include <iostream>

void CheckAndAccess(const std::variant<int, double, std::string>& my_data) {
    if (std::holds_alternative<int>(my_data)) {
        int int_value = std::get<int>(my_data);
        std::cout << "Contains int: " << int_value << '\n';
    }
    else if (std::holds_alternative<double>(my_data)) {
        double double_value = std::get<double>(my_data);
        std::cout << "Contains double: " << double_value << '\n';
    }
    else if (std::holds_alternative<std::string>(my_data)) {
        const std::string& string_value = std::get<std::string>(my_data);
        std::cout << "Contains string: " << string_value << '\n';
    }
}
```

### 3. index() - Get Active Type Index

Returns the zero-based index of the currently active type:

```cpp
#include <variant>
#include <string>
#include <iostream>

void ProcessByIndex(const std::variant<int, double, std::string>& my_variant) {
    switch (my_variant.index()) {
        case 0:
            std::cout << "Processing int: " << std::get<0>(my_variant) << '\n';
            break;
        case 1:
            std::cout << "Processing double: " << std::get<1>(my_variant) << '\n';
            break;
        case 2:
            std::cout << "Processing string: " << std::get<2>(my_variant) << '\n';
            break;
        default:
            std::cout << "Unknown type!\n";
    }
}
```

### 4. std::get_if - Safe Pointer Access

`std::get_if` returns a pointer to the value if the type matches, otherwise `nullptr`. This is excellent for safe access without exceptions:

```cpp
#include <variant>
#include <string>
#include <iostream>

void SafePointerAccess(std::variant<int, double, std::string>& my_data) {
    // Note: std::get_if takes a POINTER to the variant
    if (auto* int_ptr = std::get_if<int>(&my_data)) {
        std::cout << "Integer value: " << *int_ptr << '\n';
        *int_ptr += 10;  // Can modify through pointer
    } 
    else if (auto* double_ptr = std::get_if<double>(&my_data)) {
        std::cout << "Double value: " << *double_ptr << '\n';
    } 
    else if (auto* string_ptr = std::get_if<std::string>(&my_data)) {
        std::cout << "String value: " << *string_ptr << '\n';
        string_ptr->append(" - modified");
    }
}
```

### 5. std::variant_size - Get Number of Alternatives

Compile-time constant indicating how many types the variant can hold:

```cpp
#include <variant>
#include <iostream>

void ShowVariantSize() {
    using MyVariant = std::variant<int, double, std::string>;
    
    constexpr size_t num_types = std::variant_size_v<MyVariant>;
    std::cout << "Variant can hold " << num_types << " types\n";
    
    // Can also use without _v suffix
    std::cout << "Alternative count: " 
              << std::variant_size<MyVariant>::value << '\n';
}
```

### 6. std::variant_alternative - Get Type by Index

Retrieve the type at a specific index at compile-time:

```cpp
#include <variant>
#include <string>
#include <type_traits>
#include <iostream>

void DemonstrateVariantAlternative() {
    using MyVariant = std::variant<int, double, std::string>;
    
    // Get type at index 0
    using FirstType = std::variant_alternative_t<0, MyVariant>;
    static_assert(std::is_same_v<FirstType, int>, "First type is int");
    
    // Get type at index 2
    using ThirdType = std::variant_alternative_t<2, MyVariant>;
    static_assert(std::is_same_v<ThirdType, std::string>, "Third type is string");
    
    std::cout << "Types verified at compile-time!\n";
}
```

### 7. valueless_by_exception() - Exception Safety Check

Check if the variant is in an invalid state due to an exception:

```cpp
#include <variant>
#include <iostream>
#include <stdexcept>

struct ThrowingType {
    ThrowingType() = default;
    ThrowingType(const ThrowingType&) {
        throw std::runtime_error("Copy failed!");
    }
};

void CheckValuelessState() {
    std::variant<int, ThrowingType> my_variant = 42;
    
    try {
        ThrowingType throwing_obj;
        my_variant = throwing_obj;  // May throw during assignment
    } 
    catch (const std::exception& e) {
        if (my_variant.valueless_by_exception()) {
            std::cout << "Variant is in valueless state!\n";
            std::cout << "Index: " << my_variant.index() 
                      << " (variant_npos = " << std::variant_npos << ")\n";
        }
    }
}
```

### 8. emplace - Construct in Place

Efficiently construct a value directly within the variant without temporary objects:

```cpp
#include <variant>
#include <string>
#include <iostream>
#include <vector>

void DemonstrateEmplace() {
    std::variant<int, std::string, std::vector<int>> my_variant;
    
    // Emplace by type
    my_variant.emplace<std::string>("Hello", 5);  // Constructs "Hello" (5 chars)
    std::cout << "String: " << std::get<std::string>(my_variant) << '\n';
    
    // Emplace by index
    my_variant.emplace<2>(std::initializer_list<int>{1, 2, 3, 4, 5});
    std::cout << "Vector size: " << std::get<std::vector<int>>(my_variant).size() << '\n';
    
    // Emplace with multiple arguments
    my_variant.emplace<std::string>(10, 'X');  // Constructs "XXXXXXXXXX"
    std::cout << "String: " << std::get<std::string>(my_variant) << '\n';
}
```

### 9. swap - Efficiently Swap Variants

Swap the contents of two variants:

```cpp
#include <variant>
#include <string>
#include <iostream>

void DemonstrateSwap() {
    std::variant<int, std::string> variant_a = 42;
    std::variant<int, std::string> variant_b = "Hello";
    
    std::cout << "Before swap:\n";
    std::cout << "variant_a index: " << variant_a.index() << '\n';
    std::cout << "variant_b index: " << variant_b.index() << '\n';
    
    variant_a.swap(variant_b);
    
    std::cout << "\nAfter swap:\n";
    std::cout << "variant_a: " << std::get<std::string>(variant_a) << '\n';
    std::cout << "variant_b: " << std::get<int>(variant_b) << '\n';
}
```

## Advanced Techniques

### The Visitor Pattern with std::visit

`std::visit` is the most powerful way to work with variants. It allows you to define operations for each possible type:

#### Basic Generic Visitor

```cpp
#include <variant>
#include <string>
#include <iostream>

void BasicVisitorExample() {
    std::variant<int, double, std::string> my_data = 3.14;
    
    std::visit([](auto&& current_value) {
        using T = std::decay_t<decltype(current_value)>;
        
        if constexpr (std::is_same_v<T, int>) {
            std::cout << "Integer: " << current_value << '\n';
        } 
        else if constexpr (std::is_same_v<T, double>) {
            std::cout << "Double: " << current_value << '\n';
        } 
        else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "String: " << current_value << '\n';
        }
    }, my_data);
}
```

#### Overloaded Lambda Pattern (Most Elegant)

```cpp
#include <variant>
#include <string>
#include <iostream>

// Helper for overloading lambdas
template<class... Ts> 
struct Overloaded : Ts... { 
    using Ts::operator()...; 
};

// Deduction guide (not needed in C++20)
template<class... Ts> 
Overloaded(Ts...) -> Overloaded<Ts...>;

void OverloadedVisitorExample() {
    std::variant<int, double, std::string> my_data = "Hello";
    
    std::visit(Overloaded {
        [](int value) { 
            std::cout << "Integer: " << value << '\n'; 
        },
        [](double value) { 
            std::cout << "Double: " << value << '\n'; 
        },
        [](const std::string& value) { 
            std::cout << "String: " << value << '\n'; 
        }
    }, my_data);
}
```

#### Returning Values from Visitors

```cpp
#include <variant>
#include <string>
#include <iostream>

std::string ConvertToString(const std::variant<int, double, std::string>& my_variant) {
    return std::visit(Overloaded {
        [](int value) -> std::string { 
            return "int: " + std::to_string(value); 
        },
        [](double value) -> std::string { 
            return "double: " + std::to_string(value); 
        },
        [](const std::string& value) -> std::string { 
            return "string: " + value; 
        }
    }, my_variant);
}
```

#### Visiting Multiple Variants Simultaneously

```cpp
#include <variant>
#include <string>
#include <iostream>

void MultiVariantVisit() {
    std::variant<int, double> variant_a = 42;
    std::variant<int, double> variant_b = 3.14;
    
    std::visit([](auto&& value_a, auto&& value_b) {
        std::cout << "Sum: " << (value_a + value_b) << '\n';
        std::cout << "Product: " << (value_a * value_b) << '\n';
    }, variant_a, variant_b);
}
```

### Handling std::monostate (Empty Variants)

If you need a variant that can be empty or uninitialized, use `std::monostate` as the first type:

```cpp
#include <variant>
#include <string>
#include <iostream>

class OptionalDataHandler {
public:
    using DataVariant = std::variant<std::monostate, int, std::string>;
    
    void SetData(int value) {
        data_ = value;
    }
    
    void SetData(const std::string& value) {
        data_ = value;
    }
    
    void Clear() {
        data_ = std::monostate{};
    }
    
    bool IsEmpty() const {
        return std::holds_alternative<std::monostate>(data_);
    }
    
    void Print() const {
        std::visit(Overloaded {
            [](std::monostate) { 
                std::cout << "Empty\n"; 
            },
            [](int value) { 
                std::cout << "Integer: " << value << '\n'; 
            },
            [](const std::string& value) { 
                std::cout << "String: " << value << '\n'; 
            }
        }, data_);
    }
    
private:
    DataVariant data_;  // Default constructs to monostate
};

void MonostateExample() {
    OptionalDataHandler handler;
    
    handler.Print();           // Empty
    handler.SetData(42);
    handler.Print();           // Integer: 42
    handler.SetData("Hello");
    handler.Print();           // String: Hello
    handler.Clear();
    handler.Print();           // Empty
}
```

### Variant with Custom Types

```cpp
#include <variant>
#include <string>
#include <iostream>

struct Point {
    double x, y;
    
    friend std::ostream& operator<<(std::ostream& output_stream, const Point& point) {
        return output_stream << "Point(" << point.x << ", " << point.y << ")";
    }
};

struct Color {
    uint8_t red, green, blue;
    
    friend std::ostream& operator<<(std::ostream& output_stream, const Color& color) {
        return output_stream << "Color(R:" << (int)color.red 
                            << ", G:" << (int)color.green 
                            << ", B:" << (int)color.blue << ")";
    }
};

void CustomTypeVariant() {
    std::variant<Point, Color, std::string> shape_property;
    
    shape_property = Point{3.5, 7.2};
    std::visit([](const auto& value) {
        std::cout << value << '\n';
    }, shape_property);
    
    shape_property = Color{255, 128, 64};
    std::visit([](const auto& value) {
        std::cout << value << '\n';
    }, shape_property);
    
    shape_property = "Circle";
    std::visit([](const auto& value) {
        std::cout << value << '\n';
    }, shape_property);
}
```

## Performance Optimization

### Memory Layout and Size

Variants have minimal overhead. The storage size is the maximum of all held types plus a small discriminator:

```cpp
#include <variant>
#include <string>
#include <iostream>
#include <vector>

void ShowVariantSizes() {
    using SmallVariant = std::variant<char, int, double>;
    using LargeVariant = std::variant<std::string, std::vector<int>>;
    
    std::cout << "sizeof(char): " << sizeof(char) << '\n';
    std::cout << "sizeof(int): " << sizeof(int) << '\n';
    std::cout << "sizeof(double): " << sizeof(double) << '\n';
    std::cout << "sizeof(SmallVariant): " << sizeof(SmallVariant) << '\n';
    std::cout << "Overhead: " << (sizeof(SmallVariant) - sizeof(double)) << " bytes\n\n";
    
    std::cout << "sizeof(std::string): " << sizeof(std::string) << '\n';
    std::cout << "sizeof(std::vector<int>): " << sizeof(std::vector<int>) << '\n';
    std::cout << "sizeof(LargeVariant): " << sizeof(LargeVariant) << '\n';
}
```

### Avoiding Unnecessary Copies

Use `emplace` instead of assignment to avoid temporary object creation:

```cpp
#include <variant>
#include <string>
#include <iostream>
#include <chrono>

void PerformanceComparison() {
    std::variant<int, std::string> my_variant;
    
    // Less efficient - creates temporary
    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000; ++i) {
        my_variant = std::string(100, 'X');
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto assignment_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time
    );
    
    // More efficient - constructs in place
    start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000; ++i) {
        my_variant.emplace<std::string>(100, 'X');
    }
    end_time = std::chrono::high_resolution_clock::now();
    auto emplace_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time
    );
    
    std::cout << "Assignment: " << assignment_duration.count() << " μs\n";
    std::cout << "Emplace: " << emplace_duration.count() << " μs\n";
}
```

### Using Pointers for Large Types

For large objects that change frequently, consider storing pointers:

```cpp
#include <variant>
#include <memory>
#include <vector>
#include <string>

struct LargeData {
    std::vector<double> values;
    std::string metadata;
    // ... many more fields
    
    LargeData() : values(10000) {}
};

// Less efficient for frequent type changes
using DirectVariant = std::variant<int, double, LargeData>;

// More efficient - only pointer is copied
using PointerVariant = std::variant<
    int, 
    double, 
    std::unique_ptr<LargeData>
>;

void LargeTypeOptimization() {
    PointerVariant efficient_variant;
    
    efficient_variant = 42;
    efficient_variant = 3.14;
    efficient_variant = std::make_unique<LargeData>();
    
    // Access through pointer
    if (auto* data_ptr = std::get_if<std::unique_ptr<LargeData>>(&efficient_variant)) {
        std::cout << "Vector size: " << (*data_ptr)->values.size() << '\n';
    }
}
```

### Branch Prediction Optimization

Order variant types by frequency of use for better branch prediction:

```cpp
#include <variant>
#include <string>

// If int is most common, put it first
using OptimizedVariant = std::variant<int, double, std::string>;

// Better than this if int is most frequent
// using UnoptimizedVariant = std::variant<std::string, double, int>;

void ProcessManyVariants(const std::vector<OptimizedVariant>& variants) {
    for (const auto& variant : variants) {
        std::visit([](const auto& value) {
            // Process value
        }, variant);
    }
}
```

### Constexpr and Compile-Time Optimization

Many variant operations can be performed at compile-time:

```cpp
#include <variant>
#include <type_traits>

constexpr int GetIntValue() {
    std::variant<int, double> my_variant = 42;
    return std::get<int>(my_variant);
}

constexpr bool CheckVariantProperties() {
    using MyVariant = std::variant<int, double, float>;
    
    constexpr size_t type_count = std::variant_size_v<MyVariant>;
    static_assert(type_count == 3, "Should have 3 types");
    
    using FirstType = std::variant_alternative_t<0, MyVariant>;
    static_assert(std::is_same_v<FirstType, int>, "First type is int");
    
    return true;
}

void CompileTimeChecks() {
    constexpr int result = GetIntValue();
    constexpr bool checks_passed = CheckVariantProperties();
}
```

## Real-World Use Cases

### 1. Abstract Syntax Tree (AST) for Parsers

```cpp
#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

struct NumberNode {
    double value;
};

struct IdentifierNode {
    std::string name;
};

struct BinaryOpNode;

using AstNode = std::variant<
    NumberNode,
    IdentifierNode,
    std::unique_ptr<BinaryOpNode>
>;

struct BinaryOpNode {
    char op;  // +, -, *, /
    std::unique_ptr<AstNode> left;
    std::unique_ptr<AstNode> right;
};

double EvaluateAst(const AstNode& node) {
    return std::visit(Overloaded {
        [](const NumberNode& num_node) -> double {
            return num_node.value;
        },
        [](const IdentifierNode& id_node) -> double {
            // Look up variable value
            std::cout << "Looking up: " << id_node.name << '\n';
            return 0.0;  // Simplified
        },
        [](const std::unique_ptr<BinaryOpNode>& bin_op) -> double {
            double left_val = EvaluateAst(*bin_op->left);
            double right_val = EvaluateAst(*bin_op->right);
            
            switch (bin_op->op) {
                case '+': return left_val + right_val;
                case '-': return left_val - right_val;
                case '*': return left_val * right_val;
                case '/': return left_val / right_val;
                default: return 0.0;
            }
        }
    }, node);
}

void AstExample() {
    // Represents: 3 + 5
    auto ast = std::make_unique<BinaryOpNode>();
    ast->op = '+';
    ast->left = std::make_unique<AstNode>(NumberNode{3.0});
    ast->right = std::make_unique<AstNode>(NumberNode{5.0});
    
    AstNode root = std::move(ast);
    double result = EvaluateAst(root);
    std::cout << "Result: " << result << '\n';
}
```

### 2. Event System / Message Passing

```cpp
#include <variant>
#include <string>
#include <vector>
#include <iostream>

struct MouseClickEvent {
    int x, y;
    int button;
};

struct KeyPressEvent {
    char key;
    bool shift_pressed;
};

struct WindowResizeEvent {
    int new_width, new_height;
};

struct TextInputEvent {
    std::string text;
};

using Event = std::variant<
    MouseClickEvent,
    KeyPressEvent,
    WindowResizeEvent,
    TextInputEvent
>;

class EventHandler {
public:
    void HandleEvent(const Event& event) {
        std::visit(Overloaded {
            [this](const MouseClickEvent& mouse_event) {
                HandleMouseClick(mouse_event);
            },
            [this](const KeyPressEvent& key_event) {
                HandleKeyPress(key_event);
            },
            [this](const WindowResizeEvent& resize_event) {
                HandleWindowResize(resize_event);
            },
            [this](const TextInputEvent& text_event) {
                HandleTextInput(text_event);
            }
        }, event);
    }
    
private:
    void HandleMouseClick(const MouseClickEvent& event) {
        std::cout << "Mouse clicked at (" << event.x << ", " << event.y 
                  << ") button: " << event.button << '\n';
    }
    
    void HandleKeyPress(const KeyPressEvent& event) {
        std::cout << "Key pressed: " << event.key 
                  << (event.shift_pressed ? " (with Shift)" : "") << '\n';
    }
    
    void HandleWindowResize(const WindowResizeEvent& event) {
        std::cout << "Window resized to " << event.new_width 
                  << "x" << event.new_height << '\n';
    }
    
    void HandleTextInput(const TextInputEvent& event) {
        std::cout << "Text input: " << event.text << '\n';
    }
};

void EventSystemExample() {
    EventHandler handler;
    
    std::vector<Event> event_queue;
    event_queue.push_back(MouseClickEvent{100, 200, 1});
    event_queue.push_back(KeyPressEvent{'A', true});
    event_queue.push_back(WindowResizeEvent{1920, 1080});
    event_queue.push_back(TextInputEvent{"Hello World"});
    
    for (const auto& event : event_queue) {
        handler.HandleEvent(event);
    }
}
```

### 3. Configuration / Settings System

```cpp
#include <variant>
#include <string>
#include <map>
#include <iostream>

class ConfigurationManager {
public:
    using ConfigValue = std::variant<int, double, bool, std::string>;
    
    void SetValue(const std::string& key, ConfigValue value) {
        config_map_[key] = std::move(value);
    }
    
    template<typename T>
    T GetValue(const std::string& key, const T& default_value) const {
        auto iterator = config_map_.find(key);
        if (iterator == config_map_.end()) {
            return default_value;
        }
        
        if (auto* value_ptr = std::get_if<T>(&iterator->second)) {
            return *value_ptr;
        }
        
        return default_value;
    }
    
    void PrintAll() const {
        for (const auto& [key, value] : config_map_) {
            std::cout << key << " = ";
            std::visit([](const auto& val) {
                std::cout << val;
            }, value);
            std::cout << '\n';
        }
    }
    
private:
    std::map<std::string, ConfigValue> config_map_;
};

void ConfigurationExample() {
    ConfigurationManager config;
    
    config.SetValue("window_width", 1920);
    config.SetValue("window_height", 1080);
    config.SetValue("fullscreen", false);
    config.SetValue("gamma", 2.2);
    config.SetValue("player_name", std::string("Player1"));
    
    config.PrintAll();
    
    int width = config.GetValue<int>("window_width", 800);
    std::string name = config.GetValue<std::string>("player_name", "Unknown");
    
    std::cout << "\nWindow width: " << width << '\n';
    std::cout << "Player name: " << name << '\n';
}
```

### 4. State Machine Implementation

```cpp
#include <variant>
#include <string>
#include <iostream>

struct IdleState {
    void Enter() { std::cout << "Entering Idle state\n"; }
    void Exit() { std::cout << "Exiting Idle state\n"; }
};

struct RunningState {
    double speed;
    void Enter() { std::cout << "Entering Running state (speed: " << speed << ")\n"; }
    void Exit() { std::cout << "Exiting Running state\n"; }
};

struct JumpingState {
    double height;
    double velocity;
    void Enter() { std::cout << "Entering Jumping state\n"; }
    void Exit() { std::cout << "Exiting Jumping state\n"; }
};

struct CrouchingState {
    void Enter() { std::cout << "Entering Crouching state\n"; }
    void Exit() { std::cout << "Exiting Crouching state\n"; }
};

class PlayerStateMachine {
public:
    using State = std::variant<IdleState, RunningState, JumpingState, CrouchingState>;
    
    PlayerStateMachine() : current_state_(IdleState{}) {
        std::visit([](auto& state) { state.Enter(); }, current_state_);
    }
    
    void TransitionTo(State new_state) {
        // Exit current state
        std::visit([](auto& state) { state.Exit(); }, current_state_);
        
        // Enter new state
        current_state_ = std::move(new_state);
        std::visit([](auto& state) { state.Enter(); }, current_state_);
    }
    
    void Update(double delta_time) {
        std::visit(Overloaded {
            [delta_time](IdleState& state) {
                // Idle logic
            },
            [delta_time](RunningState& state) {
                std::cout << "Running at speed: " << state.speed << '\n';
            },
            [delta_time](JumpingState& state) {
                state.velocity -= 9.8 * delta_time;  // Gravity
                state.height += state.velocity * delta_time;
                std::cout << "Jumping at height: " << state.height << '\n';
            },
            [delta_time](CrouchingState& state) {
                // Crouching logic
            }
        }, current_state_);
    }
    
    template<typename StateType>
    bool IsInState() const {
        return std::holds_alternative<StateType>(current_state_);
    }
    
private:
    State current_state_;
};

void StateMachineExample() {
    PlayerStateMachine player;
    
    player.TransitionTo(RunningState{5.0});
    player.Update(0.016);
    
    player.TransitionTo(JumpingState{0.0, 10.0});
    player.Update(0.016);
    player.Update(0.016);
    
    player.TransitionTo(IdleState{});
    
    if (player.IsInState<IdleState>()) {
        std::cout << "Player is idle\n";
    }
}
```

### 5. Result Type (Error Handling)

```cpp
#include <variant>
#include <string>
#include <iostream>

enum class ErrorCode {
    FileNotFound,
    PermissionDenied,
    NetworkError,
    InvalidFormat
};

template<typename T>
class Result {
public:
    using ValueType = T;
    using ErrorType = std::pair<ErrorCode, std::string>;
    
    // Success constructor
    static Result Success(T value) {
        Result result;
        result.data_ = std::move(value);
        return result;
    }
    
    // Error constructor
    static Result Error(ErrorCode code, std::string message) {
        Result result;
        result.data_ = ErrorType{code, std::move(message)};
        return result;
    }
    
    bool IsSuccess() const {
        return std::holds_alternative<ValueType>(data_);
    }
    
    bool IsError() const {
        return std::holds_alternative<ErrorType>(data_);
    }
    
    const T& GetValue() const {
        return std::get<ValueType>(data_);
    }
    
    T& GetValue() {
        return std::get<ValueType>(data_);
    }
    
    const ErrorType& GetError() const {
        return std::get<ErrorType>(data_);
    }
    
    // Monadic operations
    template<typename Func>
    auto Map(Func&& function) -> Result<decltype(function(std::declval<T>()))> {
        using NewType = decltype(function(std::declval<T>()));
        
        if (IsSuccess()) {
            return Result<NewType>::Success(function(GetValue()));
        } else {
            auto [code, msg] = GetError();
            return Result<NewType>::Error(code, msg);
        }
    }
    
private:
    std::variant<ValueType, ErrorType> data_;
};

Result<std::string> ReadFile(const std::string& filename) {
    if (filename.empty()) {
        return Result<std::string>::Error(
            ErrorCode::InvalidFormat, 
            "Filename cannot be empty"
        );
    }
    
    // Simulate file reading
    if (filename == "missing.txt") {
        return Result<std::string>::Error(
            ErrorCode::FileNotFound,
            "File not found: " + filename
        );
    }
    
    return Result<std::string>::Success("File contents of " + filename);
}

Result<int> ParseInteger(const std::string& text) {
    try {
        int value = std::stoi(text);
        return Result<int>::Success(value);
    } catch (...) {
        return Result<int>::Error(
            ErrorCode::InvalidFormat,
            "Cannot parse integer from: " + text
        );
    }
}

void ResultTypeExample() {
    auto file_result = ReadFile("data.txt");
    
    if (file_result.IsSuccess()) {
        std::cout << "File contents: " << file_result.GetValue() << '\n';
    } else {
        auto [error_code, error_message] = file_result.GetError();
        std::cout << "Error: " << error_message << '\n';
    }
    
    // Chaining with Map
    auto number_result = ReadFile("number.txt")
        .Map([](const std::string& content) {
            return ParseInteger(content);
        });
}
```

### 6. JSON-like Data Structure

```cpp
#include <variant>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>

class JsonValue;

using JsonNull = std::monostate;
using JsonBool = bool;
using JsonNumber = double;
using JsonString = std::string;
using JsonArray = std::vector<JsonValue>;
using JsonObject = std::map<std::string, JsonValue>;

class JsonValue {
public:
    using VariantType = std::variant<
        JsonNull,
        JsonBool,
        JsonNumber,
        JsonString,
        JsonArray,
        JsonObject
    >;
    
    JsonValue() : data_(JsonNull{}) {}
    
    template<typename T>
    JsonValue(T value) : data_(std::move(value)) {}
    
    bool IsNull() const { return std::holds_alternative<JsonNull>(data_); }
    bool IsBool() const { return std::holds_alternative<JsonBool>(data_); }
    bool IsNumber() const { return std::holds_alternative<JsonNumber>(data_); }
    bool IsString() const { return std::holds_alternative<JsonString>(data_); }
    bool IsArray() const { return std::holds_alternative<JsonArray>(data_); }
    bool IsObject() const { return std::holds_alternative<JsonObject>(data_); }
    
    template<typename T>
    const T& Get() const { return std::get<T>(data_); }
    
    template<typename T>
    T& Get() { return std::get<T>(data_); }
    
    void Print(int indent_level = 0) const {
        std::string indent(indent_level * 2, ' ');
        
        std::visit(Overloaded {
            [&](JsonNull) { std::cout << "null"; },
            [&](JsonBool value) { std::cout << (value ? "true" : "false"); },
            [&](JsonNumber value) { std::cout << value; },
            [&](const JsonString& value) { std::cout << '"' << value << '"'; },
            [&](const JsonArray& array) {
                std::cout << "[\n";
                for (size_t i = 0; i < array.size(); ++i) {
                    std::cout << std::string((indent_level + 1) * 2, ' ');
                    array[i].Print(indent_level + 1);
                    if (i < array.size() - 1) std::cout << ",";
                    std::cout << '\n';
                }
                std::cout << indent << "]";
            },
            [&](const JsonObject& object) {
                std::cout << "{\n";
                size_t count = 0;
                for (const auto& [key, value] : object) {
                    std::cout << std::string((indent_level + 1) * 2, ' ');
                    std::cout << '"' << key << "\": ";
                    value.Print(indent_level + 1);
                    if (++count < object.size()) std::cout << ",";
                    std::cout << '\n';
                }
                std::cout << indent << "}";
            }
        }, data_);
    }
    
private:
    VariantType data_;
};

void JsonExample() {
    JsonObject root;
    root["name"] = JsonString("John Doe");
    root["age"] = JsonNumber(30);
    root["is_student"] = JsonBool(false);
    
    JsonArray hobbies;
    hobbies.push_back(JsonString("Reading"));
    hobbies.push_back(JsonString("Gaming"));
    hobbies.push_back(JsonString("Coding"));
    root["hobbies"] = hobbies;
    
    JsonObject address;
    address["city"] = JsonString("New York");
    address["zip"] = JsonString("10001");
    root["address"] = address;
    
    JsonValue json(root);
    json.Print();
    std::cout << '\n';
}
```

## Common Pitfalls

### 1. Duplicate Types

You cannot have the same type twice in a variant's type list:

```cpp
// ❌ WRONG - Compilation error
// std::variant<int, int> duplicate_variant;

// ✅ CORRECT - Wrap in distinct types
struct FirstInt { int value; };
struct SecondInt { int value; };
std::variant<FirstInt, SecondInt> distinct_variant;

// Or use std::monostate if you need an empty state
std::variant<std::monostate, int> optional_int;
```

### 2. Default Construction Requirements

A variant default-constructs its first alternative. Ensure the first type is default-constructible:

```cpp
struct NonDefaultConstructible {
    NonDefaultConstructible(int value) : data_(value) {}
    int data_;
};

// ❌ WRONG - Compilation error
// std::variant<NonDefaultConstructible, int> bad_variant;

// ✅ CORRECT - Put default-constructible type first
std::variant<int, NonDefaultConstructible> good_variant;

// ✅ CORRECT - Use std::monostate
std::variant<std::monostate, NonDefaultConstructible> monostate_variant;
```

### 3. Exception Safety

If construction throws during assignment, the variant may enter the `valueless_by_exception` state:

```cpp
#include <variant>
#include <iostream>
#include <stdexcept>

struct ThrowingType {
    ThrowingType() = default;
    ThrowingType(const ThrowingType&) {
        throw std::runtime_error("Copy failed!");
    }
    ThrowingType& operator=(const ThrowingType&) = default;
};

void SafeVariantHandling() {
    std::variant<int, ThrowingType> my_variant = 42;
    ThrowingType throwing_instance;
    
    try {
        my_variant = throwing_instance;
    } catch (const std::exception& e) {
        // Check if variant is in invalid state
        if (my_variant.valueless_by_exception()) {
            std::cout << "Variant is valueless!\n";
            // Restore to valid state
            my_variant = 0;
        }
    }
}
```

### 4. Forgetting std::get_if Takes a Pointer

```cpp
std::variant<int, double> my_variant = 42;

// ❌ WRONG
// if (auto value = std::get_if<int>(my_variant)) { }

// ✅ CORRECT - Pass pointer to variant
if (auto* value_ptr = std::get_if<int>(&my_variant)) {
    std::cout << *value_ptr << '\n';
}
```

### 5. Implicit Conversions Can Be Ambiguous

```cpp
std::variant<std::string, bool> ambiguous_variant;

// ❌ WRONG - "hello" could convert to bool!
// ambiguous_variant = "hello";

// ✅ CORRECT - Explicitly construct
ambiguous_variant = std::string("hello");

// Or use emplace
ambiguous_variant.emplace<std::string>("hello");
```

## Best Practices

### 1. Prefer std::visit Over Manual Type Checking

```cpp
// ❌ Less maintainable
if (std::holds_alternative<int>(my_variant)) {
    ProcessInt(std::get<int>(my_variant));
} else if (std::holds_alternative<double>(my_variant)) {
    ProcessDouble(std::get<double>(my_variant));
}

// ✅ Better - compiler ensures all types handled
std::visit(Overloaded {
    [](int value) { ProcessInt(value); },
    [](double value) { ProcessDouble(value); }
}, my_variant);
```

### 2. Use Overloaded Lambda Helper

Create a reusable helper for clean visitor syntax:

```cpp
template<class... Ts> 
struct Overloaded : Ts... { 
    using Ts::operator()...; 
};

template<class... Ts> 
Overloaded(Ts...) -> Overloaded<Ts...>;
```

### 3. Order Types by Frequency

Put the most frequently used type first for better performance:

```cpp
// If int is used 80% of the time
using OptimizedVariant = std::variant<int, double, std::string>;
```

### 4. Use emplace for Complex Types

```cpp
std::variant<int, std::string, std::vector<int>> my_variant;

// ❌ Less efficient
my_variant = std::vector<int>{1, 2, 3, 4, 5};

// ✅ More efficient - constructs in place
my_variant.emplace<std::vector<int>>({1, 2, 3, 4, 5});
```

### 5. Leverage constexpr When Possible

```cpp
constexpr auto GetDefaultValue() {
    std::variant<int, double> value = 42;
    return std::get<int>(value);
}

constexpr int default_value = GetDefaultValue();
```

### 6. Document Variant Purpose

```cpp
// ✅ Clear purpose
using DatabaseValue = std::variant<std::monostate, int, double, std::string>;

// Better than
using MyVariant = std::variant<std::monostate, int, double, std::string>;
```

### 7. Use Type Aliases for Complex Variants

```cpp
// Define reusable variant types
using Numeric = std::variant<int, long, float, double>;
using Container = std::variant<std::vector<int>, std::list<int>>;
using AnyValue = std::variant<Numeric, Container, std::string>;
```

### 8. Avoid Deep Nesting

```cpp
// ❌ Hard to work with
using BadVariant = std::variant<
    std::variant<int, double>,
    std::variant<std::string, bool>
>;

// ✅ Flatten the types
using GoodVariant = std::variant<int, double, std::string, bool>;
```

## Comparison with Alternatives

### vs std::any

- **Variant**: Compile-time type safety, known types, better performance
- **Any**: Runtime type checking, can hold any type, more flexibility

```cpp
#include <variant>
#include <any>

// Variant - types known at compile time
std::variant<int, double, std::string> typed_variant = 42;

// Any - type known only at runtime
std::any runtime_any = 42;
```

### vs std::optional

- **Variant**: Multiple possible types
- **Optional**: Single type or nothing

```cpp
#include <variant>
#include <optional>

// Variant - multiple types
std::variant<std::monostate, int, std::string> multi_type;

// Optional - one type or empty
std::optional<int> maybe_int;
```

### vs Traditional Unions

- **Variant**: Type-safe, automatic lifetime management, works with complex types
- **Union**: Manual lifetime management, no type safety, limited to POD types

```cpp
// ❌ Traditional union - unsafe
union OldStyle {
    int integer_value;
    double double_value;
};

// ✅ Modern variant - safe
std::variant<int, double> new_style;
```

## Summary

`std::variant` is a powerful, type-safe alternative to unions that provides:

- **Type safety** through compile-time checking
- **Automatic lifetime management** for contained objects
- **Excellent performance** with minimal overhead
- **Clean syntax** with visitor pattern support
- **Modern C++ idioms** like constexpr support

Key takeaways:
1. Use `std::visit` with overloaded lambdas for clean, maintainable code
2. Prefer `emplace` over assignment for complex types
3. Order variant types by usage frequency for better performance
4. Always check for `valueless_by_exception` in critical code
5. Use `std::get_if` for safe, exception-free access
6. Leverage compile-time features like `variant_size` and `variant_alternative`

Variants are particularly useful for state machines, AST nodes, event systems, configuration systems, and any scenario requiring type-safe polymorphism without inheritance overhead.
