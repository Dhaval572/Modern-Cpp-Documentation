# Modern C++ String Reference Guide

A comprehensive guide to using strings in modern C++ with safety, performance, and style.

---

## Table of Contents

1. [C-Style vs Modern C++ - Quick Reference](#c-style-vs-modern-c---quick-reference)
2. [String Types Overview](#string-types-overview)
3. [String Creation & Initialization](#string-creation--initialization)
4. [String Views (C++17)](#string-views-c17)
5. [String Operations](#string-operations)
6. [String Conversions](#string-conversions)
7. [Performance Tips](#performance-tips)
8. [Common Patterns](#common-patterns)

---

## C-Style vs Modern C++ - Quick Reference

### ❌ Don't Use C-Style Functions → ✅ Use Modern C++ Instead

| ❌ C-Style (Unsafe) | ✅ Modern C++ (Safe) | Why Modern is Better |
|---------------------|----------------------|----------------------|
| `strcpy(dest, src)` | `dest = src` | Automatic bounds checking, no buffer overflow |
| `strncpy(dest, src, n)` | `dest.assign(src, n)` | Safe, manages memory automatically |
| `strcat(dest, src)` | `dest += src` or `dest.append(src)` | No buffer overflow, automatic resizing |
| `strncat(dest, src, n)` | `dest.append(src, n)` | Safe concatenation with bounds |
| `strlen(str)` | `str.size()` or `str.length()` | No null-terminator traversal needed |
| `strcmp(s1, s2)` | `s1 == s2` or `s1.compare(s2)` | Clean syntax, same performance |
| `strncmp(s1, s2, n)` | `s1.compare(0, n, s2, 0, n)` | Type-safe comparison |
| `strchr(str, ch)` | `str.find(ch)` | Returns safe index, not raw pointer |
| `strstr(str, substr)` | `str.find(substr)` | Safe index-based search |
| `sprintf(buf, fmt, ...)` | `std::format(fmt, ...)` (C++20) or `std::to_string()` | Type-safe, no buffer overflow |
| `atoi(str)` | `std::stoi(str)` | Exception-based error handling |
| `atof(str)` | `std::stod(str)` | Proper error handling |
| `char* str = new char[n]` | `std::string str` | Automatic memory management |
| `malloc()` / `free()` for strings | `std::string` | RAII, no memory leaks |

### Complete Comparison Table

```cpp
// ❌ C-STYLE - AVOID THESE!
char dest[100];
char src[] = "Hello";

strcpy(dest, src);                    // Buffer overflow risk!
strncpy(dest, src, 99);               // Doesn't null-terminate if truncated!
strcat(dest, " World");               // Buffer overflow risk!
int len = strlen(dest);               // O(n) traversal every time
if (strcmp(dest, src) == 0) {}        // Verbose comparison
char* found = strchr(dest, 'H');      // Raw pointer, can be NULL
sprintf(dest, "Num: %d", 42);         // Buffer overflow risk!
int num = atoi("123");                // No error handling!

// ✅ MODERN C++ - USE THESE!
std::string dest;
std::string src = "Hello";

dest = src;                           // Safe copy!
dest.assign(src, 0, 99);              // Safe with bounds!
dest += " World";                     // Automatic resizing!
size_t len = dest.size();             // O(1) constant time!
if (dest == src) {}                   // Clean comparison!
size_t pos = dest.find('H');          // Returns index or npos!
dest = std::format("Num: {}", 42);    // Type-safe! (C++20)
int num = std::stoi("123");           // Throws exception on error!
```

---

## String Types Overview

### `std::string`
- **Use for:** Owning string data, modifications
- **Memory:** Dynamic allocation, manages its own memory
- **Safety:** ✅ Automatic memory management

### `std::string_view` (C++17)
- **Use for:** Read-only string access, function parameters
- **Memory:** Non-owning view, no allocation
- **Safety:** ⚠️ Must ensure viewed string outlives the view

### `const char*` (C-style)
- **Use for:** Interfacing with C APIs only
- **Memory:** Manual management required
- **Safety:** ❌ Avoid in modern C++

---

## String Creation & Initialization

### Basic Initialization

```cpp
#include <string>
#include <string_view>

// Empty string - BEST PRACTICES
std::string empty_str{};              // ✅ BEST - Uniform initialization
std::string empty_str2;               // ✅ OK - Default constructor
std::string empty_str3 = "";          // ❌ AVOID - Unnecessary literal

// From literal - BEST PRACTICES
std::string greeting{"Hello, World!"};  // ✅ BEST - Uniform initialization
std::string greeting_alt = "Hello, World!";  // ✅ OK - Copy initialization
std::string greeting_alt2
{
    "Hello, World!"
};                                      // ✅ BEST - Multi-line uniform init

// From another string (BEST PRACTICE: Use assignment)
std::string dest;
std::string src = "Source";
dest = src;  // ✅ Modern way (replaces strcpy)

// Copy constructor
std::string copy_str
{
    greeting
};

// Using raw string literals (C++11)
std::string multiline = R"(
    Line 1
    Line 2
    Line 3
)";

// Repeated characters
std::string stars(10, '*'); // "**********"

// From substring
std::string sub_str
{
    greeting, 0, 5
}; // "Hello"
```

### Modern Initialization (C++14/17)

```cpp
using namespace std::string_literals;

// String literals with 's' suffix - BEST PRACTICES
auto modern_str = "Hello"s;        // ✅ std::string with literal suffix
auto raw_ptr = "Hello";            // const char* (no suffix)
std::string explicit_str{"Hello"}; // ✅ BEST - Explicit type with uniform init

// Unicode strings
auto utf8_str = u8"Hello"s;   // UTF-8
auto utf16_str = u"Hello"s;   // UTF-16
auto utf32_str = U"Hello"s;   // UTF-32
```

### ✅ Initialization Best Practices Summary

```cpp
// ✅ BEST - Uniform initialization (C++11)
std::string best_empty{};           // Empty string
std::string best_init{"Hello"};     // From literal
std::string best_copy{other_str};   // From another string

// ✅ OK - Traditional initialization
std::string ok_empty;               // Empty string (default constructor)
std::string ok_init = "Hello";      // From literal

// ❌ AVOID - Unnecessary empty literal
std::string avoid_this = "";        // Wasteful - use {} instead!

// Why uniform initialization is better:
// 1. Consistent syntax for all types
// 2. Prevents narrowing conversions
// 3. No ambiguity with function declarations
// 4. Modern C++ style (C++11 and later)
```

---

## Best Practices - String Operations

### String Copying

```cpp
std::string dest;
std::string src = "Hello, World!";

// ❌ C-Style - DON'T USE!
char c_dest[100];
char c_src[] = "Hello";
strcpy(c_dest, c_src);           // Dangerous!
strncpy(c_dest, c_src, 99);      // Still problematic!

// ✅ Modern C++ - USE THIS!
dest = src;                       // Simple assignment - safe!
dest.assign(src);                 // Explicit assignment
dest.assign(src, 0, 5);          // Partial copy: "Hello"
```

### String Concatenation

```cpp
std::string result = "Hello";
std::string addition = " World";

// ❌ C-Style - DON'T USE!
char c_result[100] = "Hello";
strcat(c_result, " World");       // Buffer overflow risk!
strncat(c_result, " World", 6);   // Still risky!

// ✅ Modern C++ - USE THIS!
result += " World";               // Operator overload - clean!
result.append(" World");          // Explicit append
result.append(addition, 0, 5);   // Partial append: " Worl"
result = result + addition;       // Creates new string
```

### String Length

```cpp
std::string text = "Hello, World!";

// ❌ C-Style - DON'T USE!
const char* c_text = "Hello";
size_t len = strlen(c_text);      // O(n) - traverses entire string!

// ✅ Modern C++ - USE THIS!
size_t len = text.size();         // O(1) - instant!
size_t len2 = text.length();      // Same as size(), O(1)
bool is_empty = text.empty();     // Better than checking size() == 0
```

### String Comparison

```cpp
std::string str1 = "Apple";
std::string str2 = "Banana";

// ❌ C-Style - DON'T USE!
const char* c_str1 = "Apple";
const char* c_str2 = "Banana";
if (strcmp(c_str1, c_str2) == 0) {}     // Verbose!
if (strncmp(c_str1, c_str2, 3) == 0) {} // Compare first 3 chars

// ✅ Modern C++ - USE THIS!
if (str1 == str2) {}                     // Clean and readable!
if (str1 != str2) {}                     // Not equal
if (str1 < str2) {}                      // Lexicographical comparison
int cmp = str1.compare(str2);            // Returns <0, 0, or >0
int partial_cmp = str1.compare(0, 3, str2, 0, 3);  // Compare substrings
```

### String Searching

```cpp
std::string text = "Hello, World!";

// ❌ C-Style - DON'T USE!
const char* c_text = "Hello, World!";
char* found_char = strchr(c_text, 'W');      // Returns pointer or NULL
char* found_str = strstr(c_text, "World");   // Returns pointer or NULL
if (found_char != nullptr) {}                // Ugly null checks

// ✅ Modern C++ - USE THIS!
size_t pos = text.find('W');                 // Returns index or npos
size_t pos2 = text.find("World");            // Find substring
if (pos != std::string::npos)                // Clean check
{
    std::cout << "Found at: " << pos << '\n';
}

// More search operations
size_t last = text.rfind('o');               // Find last occurrence
size_t first_of = text.find_first_of("aeiou"); // Find any vowel
size_t first_not = text.find_first_not_of(" "); // Find non-space
```

### String Formatting

```cpp
int number = 42;
double pi = 3.14159;
std::string name = "Alice";

// ❌ C-Style - DON'T USE!
char buffer[100];
sprintf(buffer, "Number: %d, Pi: %.2f", number, pi);  // Buffer overflow!
snprintf(buffer, 100, "Name: %s", name.c_str());      // Better but still risky

// ✅ Modern C++ - USE THIS!

// Option 1: std::format (C++20) - BEST!
#include <format>
std::string result = std::format("Number: {}, Pi: {:.2f}", number, pi);
std::string greeting = std::format("Hello, {}!", name);

// Option 2: std::ostringstream (Pre-C++20)
#include <sstream>
std::ostringstream oss;
oss << "Number: " << number << ", Pi: " << std::fixed << std::setprecision(2) << pi;
std::string result2 = oss.str();

// Option 3: String concatenation
std::string result3 = "Number: " + std::to_string(number) + 
                      ", Pi: " + std::to_string(pi);
```

### String to Number Conversion

```cpp
std::string num_str = "12345";
std::string float_str = "3.14";

// ❌ C-Style - DON'T USE!
int num = atoi(num_str.c_str());        // No error handling!
double d = atof(float_str.c_str());     // No error handling!
long l = strtol(num_str.c_str(), nullptr, 10);  // Better but ugly

// ✅ Modern C++ - USE THIS!
try
{
    int num = std::stoi(num_str);            // Throws on error
    long long_num = std::stol(num_str);      // For long
    unsigned long unum = std::stoul(num_str); // For unsigned
    float f = std::stof(float_str);          // For float
    double d = std::stod(float_str);         // For double
    
    // With error position tracking
    size_t pos;
    int value = std::stoi(num_str, &pos);    // pos = characters processed
}
catch (const std::invalid_argument& e)
{
    std::cerr << "Invalid number format!\n";
}
catch (const std::out_of_range& e)
{
    std::cerr << "Number out of range!\n";
}
```

### Memory Management

```cpp
// ❌ C-Style - DON'T USE!
char* buffer = (char*)malloc(100);      // Manual allocation
strcpy(buffer, "Hello");
free(buffer);                           // Manual deallocation - error prone!

char* str = new char[100];
strcpy(str, "World");
delete[] str;                           // Easy to forget!

// ✅ Modern C++ - USE THIS!
std::string str = "Hello";              // Automatic management!
// No need to free - destructor handles it!

// For dynamic needs
{
    std::string temp = "Temporary";
    // Automatically destroyed when scope ends
}
```

---

## String Views (C++17)

String views provide a non-owning view over character sequences - fast and efficient!

### Basic Usage

```cpp
#include <string_view>

void process_string(std::string_view sv)
{
    // No copy, just a view
    std::cout << sv << '\n';
    size_t len = sv.size();  // ✅ Use .size(), not strlen()!
}

int main()
{
    std::string str = "Hello";
    const char* c_str = "World";
    
    process_string(str);         // ✅ Works
    process_string(c_str);       // ✅ Works
    process_string("Literal");   // ✅ Works
    
    return 0;
}
```

### String View Operations

```cpp
std::string text = "Hello, World!";
std::string_view view
{
    text
};

// All operations work like std::string
size_t len = view.size();               // ✅ Not strlen()!
bool empty = view.empty();              // ✅ Not !view or checking size
size_t pos = view.find("World");        // ✅ Not strstr()!
std::string_view sub = view.substr(0, 5); // ✅ Not strncpy()!

// Comparison
std::string_view other = "Hello";
if (view == other) {}                   // ✅ Not strcmp()!
```

### ⚠️ Lifetime Safety

```cpp
std::string_view dangerous()
{
    std::string temp = "Temporary";
    return temp;  // ❌ DANGER! Returning view to destroyed object
}

std::string_view safe(const std::string& str)
{
    return str;  // ✅ Safe - string outlives the view
}
```

---

## String Operations

### Accessing Characters

```cpp
std::string text = "Hello";

// ✅ Modern C++ way
char ch1 = text.at(0);        // 'H' - throws if out of bounds
char ch2 = text[0];           // 'H' - undefined if out of bounds
char first = text.front();    // 'H'
char last = text.back();      // 'o'

// For C API interop only
const char* c_string = text.c_str();  // Null-terminated
const char* data_ptr = text.data();   // May not be null-terminated
```

### Modifying Strings

```cpp
std::string text = "Hello";

// Append (replacement for strcat)
text += " World";                    // ✅ Operator (recommended)
text.append("!");                    // ✅ Method
text.push_back('?');                 // ✅ Single character

// Insert
text.insert(5, " Beautiful");        // Insert at position

// Erase
text.erase(5, 10);                   // Erase 10 chars from pos 5
text.pop_back();                     // Remove last character

// Replace
text.replace(6, 5, "C++");          // Replace 5 chars from pos 6

// Clear
text.clear();                        // Empty the string

// Reserve capacity (optimization)
text.reserve(100);                   // Pre-allocate memory
```

### Substrings

```cpp
std::string text = "Hello, World!";

// ✅ Modern C++ - safe extraction
std::string sub = text.substr(0, 5);     // "Hello"
std::string rest = text.substr(7);       // "World!"

// ✅ Using string_view (C++17) - no copy!
std::string_view view
{
    text
};
std::string_view sub_view = view.substr(0, 5);  // "Hello" - zero copy!

// ❌ Don't use strncpy for this!
// char sub[6];
// strncpy(sub, text.c_str(), 5);  // Dangerous!
```

---

## String Conversions

### Number to String

```cpp
int int_val = 42;
double double_val = 3.14159;

// ❌ C-Style - DON'T USE!
char buffer[50];
sprintf(buffer, "%d", int_val);

// ✅ Modern C++ - USE THIS!

// Option 1: std::to_string (C++11) - Simple!
std::string str1 = std::to_string(int_val);      // "42"
std::string str2 = std::to_string(double_val);   // "3.141590"

// Option 2: std::format (C++20) - Best control!
#include <format>
std::string formatted = std::format("Value: {}", int_val);
std::string precise = std::format("Pi: {:.2f}", double_val); // "Pi: 3.14"
std::string hex = std::format("Hex: {:#x}", int_val);        // "Hex: 0x2a"

// Option 3: std::ostringstream (pre-C++20)
#include <sstream>
std::ostringstream oss;
oss << "Value: " << int_val << ", Pi: " << double_val;
std::string result = oss.str();
```

---

## Performance Tips

### 1. Use `string_view` for Parameters

```cpp
// ❌ Slow - creates copy
void process(std::string str)
{
    std::cout << str << '\n';
}

// ✅ Better - no copy, but can't accept temporaries easily
void process(const std::string& str)
{
    std::cout << str << '\n';
}

// ✅ Best - no copy, accepts any string-like type
void process(std::string_view sv)
{
    std::cout << sv << '\n';
}
```

### 2. Reserve Capacity

```cpp
// ❌ Multiple reallocations
std::string result;
for (int i = 0; i < 1000; ++i)
{
    result += "x";
}

// ✅ Single allocation
std::string result;
result.reserve(1000);
for (int i = 0; i < 1000; ++i)
{
    result += "x";
}
```

### 3. Use Move Semantics

```cpp
std::string create_large_string()
{
    std::string result(1000000, 'x');
    return result;  // ✅ Moved, not copied (C++11)
}

std::string str = create_large_string();  // No copy!
```

### 4. Use .size() Not strlen()

```cpp
std::string text = "Hello, World!";

// ❌ Slow - O(n)
size_t len = strlen(text.c_str());

// ✅ Fast - O(1)
size_t len = text.size();
```

---

## Common Patterns

### String Building

```cpp
#include <sstream>
#include <vector>

// Using ostringstream
std::ostringstream oss;
oss << "Name: " << name << ", Age: " << age;
std::string result = oss.str();

// C++20 format (best)
#include <format>
std::string result = std::format("Name: {}, Age: {}", name, age);

// Joining strings
std::vector<std::string> words = {"Hello", "World", "C++"};
std::string joined;
joined.reserve(50);  // Optimize!

for (size_t i = 0; i < words.size(); ++i)
{
    joined += words[i];
    if (i < words.size() - 1)
    {
        joined += " ";
    }
}
// Result: "Hello World C++"
```

### String Splitting

```cpp
#include <sstream>
#include <vector>

std::vector<std::string> split(std::string_view str, char delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string_view::npos)
    {
        tokens.emplace_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    
    tokens.emplace_back(str.substr(start));
    return tokens;
}

// Usage
auto parts = split("one,two,three", ',');
// Result: {"one", "two", "three"}
```

### Trimming Whitespace

```cpp
#include <algorithm>
#include <cctype>

std::string trim_left(std::string str)
{
    str.erase(
        str.begin(),
        std::find_if(str.begin(), str.end(), [](unsigned char ch)
        {
            return !std::isspace(ch);
        })
    );
    return str;
}

std::string trim_right(std::string str)
{
    str.erase(
        std::find_if(str.rbegin(), str.rend(), [](unsigned char ch)
        {
            return !std::isspace(ch);
        }).base(),
        str.end()
    );
    return str;
}

std::string trim(std::string str)
{
    return trim_left(trim_right(std::move(str)));
}

// Usage
std::string text = "  Hello World  ";
std::string trimmed = trim(text);  // "Hello World"
```

### Case Conversion

```cpp
#include <algorithm>
#include <cctype>

std::string to_upper(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c)
        {
            return std::toupper(c);
        }
    );
    return str;
}

std::string to_lower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c)
        {
            return std::tolower(c);
        }
    );
    return str;
}

// Usage
std::string text = "Hello World";
std::string upper = to_upper(text);  // "HELLO WORLD"
std::string lower = to_lower(text);  // "hello world"
```

---

## Summary - Modern C++ String Best Practices

### ✅ Always Use

| Operation | Modern C++ Way | Why |
|-----------|---------------|-----|
| **Empty string** | `std::string str{}` | Uniform initialization, consistent style |
| **Avoid** | `std::string str = ""` | Wasteful, creates unnecessary literal |
| **Copy** | `dest = src` | Safe, automatic memory management |
| **Length** | `str.size()` or `str.length()` | O(1) constant time |
| **Compare** | `str1 == str2` | Clean, readable syntax |
| **Concatenate** | `str1 += str2` | Automatic resizing |
| **Search** | `str.find()` | Returns safe index |
| **Format** | `std::format()` (C++20) | Type-safe, no buffer overflow |
| **Convert to int** | `std::stoi()` | Exception-based errors |
| **Convert to string** | `std::to_string()` | Simple and safe |
| **Function params** | `std::string_view` | Zero-copy efficiency |
| **Memory** | `std::string` | Automatic RAII management |

### ❌ Never Use

| C Function | Why Avoid | Modern Alternative |
|------------|-----------|-------------------|
| `strcpy()` | Buffer overflow risk | `dest = src` |
| `strcat()` | Buffer overflow risk | `dest += src` |
| `strlen()` | O(n) traversal | `str.size()` |
| `strcmp()` | Verbose syntax | `str1 == str2` |
| `sprintf()` | Buffer overflow risk | `std::format()` |
| `atoi()` | No error handling | `std::stoi()` |
| `malloc()/free()` | Manual management | `std::string` |
| `new[]/delete[]` | Manual management | `std::string` |

### Key Principles

1. **Use `std::string{}`** for empty string initialization - not `= ""`!
2. **Use uniform initialization `{}`** - it's the modern C++ way!
3. **Use `std::string`** for owning string data - always!
4. **Use `std::string_view`** for read-only function parameters
5. **Use `.size()` instead of `strlen()`** - it's O(1)!
6. **Use assignment `=` instead of `strcpy()`** - it's safe!
7. **Use `+=` instead of `strcat()`** - no buffer overflow!
8. **Use `==` instead of `strcmp()`** - it's cleaner!
9. **Use `std::format()` instead of `sprintf()`** - type-safe!
10. **Reserve capacity** when building large strings
11. **Avoid C-style functions** - they're unsafe and obsolete

---

**Remember:** Modern C++ strings are safer, faster, and more expressive than C-style strings. Always prefer C++ standard library functions over C string functions!
