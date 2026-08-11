# C++ Constants & Compile-Time Features: `const`, `constexpr`, `consteval`, `constinit`

This guide explains the **main purpose** and **differences** between the key C++ keywords for constants and compile-time computation. It’s written to be concise, clear, and ready to use as a README.md.

---

## 1️⃣ `const` — Immutable Variable

* **Purpose:** Make a variable **read-only** after initialization.
* **Compile-time?** Not necessarily — may be runtime.
* **Typical use:** Prevent modification of a value.

```cpp
const int x = 42;
x = 10; // ❌ Error: cannot modify a const variable
```

**Key idea:** *“This value cannot be changed after initialization.”*

---

## 2️⃣ `constexpr` — Compile-Time Constant

* **Purpose:** Guarantee a value **can be computed at compile time**.
* **Applies to:** Variables, functions, constructors.
* **Typical use:** Computations that can be evaluated at compile time.

```cpp
constexpr int square(int n) {
    return n * n;
}

constexpr int value = square(5); // ✅ Computed at compile time
```

**Key idea:** *“This value or function is evaluable at compile time if needed.”*

---

## 3️⃣ `consteval` — Immediate Compile-Time Evaluation

* **Purpose:** Force a function to **always execute at compile time**.
* **Applies to:** Functions or constructors only.
* **Typical use:** Generate values that must exist at compile time.

```cpp
consteval int cube(int n) {
    return n * n * n;
}

int a = cube(3);        // ❌ Error: must be compile-time
constexpr int b = cube(3); // ✅ Allowed
```

**Key idea:** *“This function cannot run at runtime — only compile time.”*

---

## 4️⃣ `constinit` — Compile-Time Initialization for Variables

* **Purpose:** Ensure a variable is **initialized at compile time**, but **can be modified later**.
* **Applies to:** Variables only.
* **Typical use:** Avoid static initialization order problems in global/static variables.

```cpp
constinit int counter = 100; // ✅ Initialized at compile-time
counter = 200;               // ✅ Can modify at runtime
```

**Key idea:** *“The initializer must be known at compile time, but the variable is not immutable.”*

---

## 5️⃣ Quick Comparison Table

|     Keyword |         Applies To |              Compile-Time Guarantee | Runtime Modification | Example Purpose                   |
| ----------: | -----------------: | ----------------------------------: | -------------------: | --------------------------------- |
|     `const` |           Variable |                                  No |      ❌ Cannot modify | Immutable variable                |
| `constexpr` | Variable, Function |    Can be evaluated at compile-time |      ❌ Cannot modify | Compile-time computation          |
| `consteval` |           Function |   Must be evaluated at compile-time |                ❌ N/A | Force compile-time function       |
| `constinit` |           Variable | Must be initialized at compile-time |         ✅ Can modify | Safe static/global initialization |

---

### ✅ Summary

* **`const`** = immutable value
* **`constexpr`** = compile-time evaluable value/function
* **`consteval`** = must execute at compile-time
* **`constinit`** = compile-time initialization guaranteed, mutable variable

---

💡 **Tip:** Combine them:

```cpp
consteval int generate() { return 42; }
constinit int globalValue = generate(); // compile-time initialized, mutable
```
