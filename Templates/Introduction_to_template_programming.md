# C++ Templates: A Gentle Introduction

> Templates look scary from the outside. They are not.
> At the core, a template is just **a recipe the compiler uses to write code for you**.

---

## Table of Contents

1. [What is a template?](#1-what-is-a-template)
2. [Why do we need templates?](#2-why-do-we-need-templates)
3. [Templates are compile-time programming](#3-templates-are-compile-time-programming)
4. [Templates vs generics in other languages](#4-templates-vs-generics-in-other-languages)
5. [More power: what else can templates do?](#5-more-power-what-else-can-templates-do)
6. [The catch: templates accept everything](#6-the-catch-templates-accept-everything)
7. [Constraints and concepts: taking control back](#7-constraints-and-concepts-taking-control-back)
8. [Another example: constraining behavior, not just numbers](#8-another-example-constraining-behavior-not-just-numbers)
9. [Key takeaways](#9-key-takeaways)

---

## 1. What is a template?

A template lets you write a function or a class **once**, without deciding the type yet.
The type becomes a placeholder, and the compiler fills it in when you use it.

```cpp
template <typename T>
T Add(T a, T b)
{
    return a + b;
}
```

Read it like plain English:

> "Here is a function called `Add`. It takes two values of some type `T`, and returns a `T`.
> I will tell you what `T` is later."

Using it:

```cpp
int sum_int = Add(2, 3);           // T becomes int
double sum_double = Add(1.5, 2.5); // T becomes double
```

That's it. Same code, different types. No magic.

---

## 2. Why do we need templates?

### Problem 1: Repeating yourself for every type

Without templates, you would write the same logic again and again:

```cpp
int Add(int a, int b)
{
    return a + b;
}

double Add(double a, double b)
{
    return a + b;
}

float Add(float a, float b)
{
    return a + b;
}
// ...and so on for every type
```

The logic is identical. Only the type changes. Templates remove this duplication.

### Problem 2: Custom types you have never seen

This is the **real** reason templates must exist.

Imagine you write a library today. Next year, someone else creates their own type:

```cpp
struct t_Vector2
{
    float x;
    float y;
};
```

You, the library author, **cannot know** this type exists. It is impossible to write
`Add(t_Vector2, t_Vector2)` ahead of time for every type that will ever be created.

With a template, you don't have to. The user's type plugs into your code automatically
(as long as it supports the operations you use, like `+`).

> Templates let you write code for types that **don't exist yet**.

---

## 3. Templates are compile-time programming

Templates are not something that runs when your program runs. They work **while compiling**.

When you write `Add(2, 3)`, the compiler:

1. Sees that `T` is `int`.
2. Generates a real, normal `int` version of `Add`.
3. Compiles that version like any other function.

This is called **instantiation**. The result is exactly the code you would have written by hand,
so there is **no runtime cost** for using templates.

```
Your template  ──►  Compiler  ──►  Add<int>, Add<double>, ...  ──►  Fast machine code
```

Because the compiler does the work, it can also do *computation* at compile time (more on that below).
For now, just remember: **templates = the compiler writing code for you**.

---

## 4. Templates vs generics in other languages

Yes, templates are C++'s version of "generics". But they are **more powerful** than generics
in most other languages.

| | C++ templates | Java generics | C# generics |
|---|---|---|---|
| Resolved when? | Compile time | Compile time, then types are erased | Runtime (by the VM) |
| Separate code per type? | Yes, fully specialized | No, one shared version | Partly |
| Works with primitives (`int`, `float`)? | Yes, directly | No, needs boxing (`Integer`) | Yes |
| Runtime overhead | None | Boxing and casts | Small |
| Can take values, not only types? | Yes (`std::array<int, 5>`) | No | No |
| Can behave differently per type (specialization)? | Yes | No | Limited |
| Can compute things at compile time? | Yes | No | No |

### What makes C++ templates special?

- **Zero-cost.** The compiler generates code tuned for each exact type.
- **Duck typing at compile time.** If a type supports the operations you use, it works. No base class or interface needed.
- **Values as parameters.** You can pass numbers, not just types: `std::array<int, 5>` has its size baked into the type.
- **Specialization.** You can say "for this one special type, do it differently."
- **Compile-time computation.** The compiler can calculate things before your program even starts.

Other languages give you "write once, use with many types".
C++ gives you that **plus** speed, flexibility, and control.

---

## 5. More power: what else can templates do?

Functions are only the beginning. Here is a quick tour, each with a tiny example.

### 5.1 Class templates: reusable data structures

A whole struct or class can be a template. This is how `std::vector`, `std::map`,
and `std::unique_ptr` are built. You have been using templates since your first `std::vector<int>`.

```cpp
template <typename T>
struct t_Box
{
    T value;
};

t_Box<int> number_box{42};
t_Box<std::string> text_box{"hello"};
```

One definition, any content.

### 5.2 Values as parameters, not only types

A template parameter can be a **number**. The compiler bakes it into the type.

```cpp
template <typename T, int size>
struct t_FixedArray
{
    T data[size];
};

t_FixedArray<float, 8> samples; // exactly 8 floats, size known at compile time
```

`t_FixedArray<float, 8>` and `t_FixedArray<float, 16>` are two different types.
Mistakes like mixing sizes are caught by the compiler, not at runtime.

### 5.3 Specialization: a special case for one type

Write the general version once, then override it for the types that need different behavior.

```cpp
template <typename T>
const char* TypeName()
{
    return "unknown";
}

template <>
const char* TypeName<int>()
{
    return "int";
}

TypeName<double>(); // "unknown"
TypeName<int>();    // "int"
```

### 5.4 Compile-time computation

With `constexpr`, the compiler can run your code **before the program starts**.
The result is baked into the binary, so it costs nothing at runtime.

```cpp
template <typename T>
constexpr T Square(T value)
{
    return value * value;
}

static_assert(Square(4) == 16); // checked by the compiler, no program run needed
```

If the check is false, your code simply **won't compile**. Bugs caught before they exist.

### 5.5 Variadic templates: any number of arguments

A template can accept a whole *pack* of arguments, not just two.

```cpp
template <typename... Args>
auto Sum(Args... args)
{
    return (args + ...);
}

Sum(1, 2, 3);          // 6
Sum(1.5, 2.5, 3.0, 4); // 11
```

This is how `std::make_unique`, `std::tuple`, and `printf`-style type-safe functions are possible.

### 5.6 Compile-time polymorphism: no virtual calls needed

Normally, "work with many types" means inheritance and `virtual` functions. The decision
is made **at runtime** through a lookup table, and the compiler can't see through it.

With templates, the decision is made **at compile time**. The compiler knows the exact type,
so it can inline and optimize the call away completely.

| | Virtual functions | Templates |
|---|---|---|
| Decided | At runtime | At compile time |
| Needs a base class? | Yes | No |
| Call overhead | Indirect call | Can be fully inlined |
| Works with `int`, `float`, etc.? | No | Yes |

> Templates give you **abstraction without paying for it**.

---

## 6. The catch: templates accept everything

With great power comes a small problem. Look at `Add` again:

```cpp
template <typename T>
T Add(T a, T b)
{
    return a + b;
}
```

`T` can be **anything**. The template doesn't ask questions.

### Surprise 1: It does something you didn't intend

```cpp
std::string first_name = "Hello, ";
std::string second_name = "World";

auto result = Add(first_name, second_name); // compiles fine!
```

`Add` was meant for numbers, but strings also support `+`, so it quietly **concatenates** them.
No error, no warning. The code is "correct" for the compiler, but wrong for your intention.

### Surprise 2: Errors that nobody can read

```cpp
struct t_Empty
{
};

t_Empty first_value;
t_Empty second_value;

Add(first_value, second_value);
```

Because `t_Empty` has no `+`, the compiler fails. But the error message points deep inside
the template, mentions internal details, and can span many lines.
It tells you **how** it failed, not **what you did wrong**.

> A template is honest but not helpful. It only complains *after* it has tried and failed.

**As the programmer, restricting what a template accepts is your responsibility.**

---

## 7. Constraints and concepts: taking control back

Since C++20, we can state our rules directly in the code using **concepts**.
A concept is just a named condition on a type.

```cpp
#include <concepts>

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;
```

Read it as: *"A type is `Numeric` if it is an integer type or a floating-point type."*

Now attach it to `Add`:

```cpp
template <Numeric T>
T Add(T a, T b)
{
    return a + b;
}
```

Same function, but now `T` **must** be `Numeric`.

```cpp
Add(2, 3);                    // OK
Add(1.5, 2.5);                // OK
Add(first_name, second_name); // Error: std::string is not Numeric
```

### What do we gain?

- **No accidental misuse.** Strings can no longer sneak into `Add` and concatenate silently.
- **Clear errors.** Instead of a wall of text, the compiler says: *"constraints not satisfied: `std::string` is not `Numeric`."*
- **Self-documenting code.** Anyone reading `template <Numeric T>` instantly knows what is allowed.

### Other ways to write the same thing

```cpp
// Style 1: requires clause
template <typename T>
    requires Numeric<T>
T Add(T a, T b)
{
    return a + b;
}

// Style 2: abbreviated (auto with a concept)
Numeric auto Add(Numeric auto a, Numeric auto b)
{
    return a + b;
}
```

All of them mean the same thing. Pick the one you find most readable.

Constraints also work with the variadic template from earlier:

```cpp
template <Numeric... Args>
auto Sum(Args... args)
{
    return (args + ...);
}
```

Now `Sum` accepts any number of arguments, but **every one** of them must be `Numeric`.

---

## 8. Another example: constraining behavior, not just numbers

Concepts are not only about "is it a number?". They can say **"does this type know how to do X?"**

Imagine a function that prints the area of any shape. It doesn't care what the shape is,
only that the shape has an `Area()` function.

```cpp
#include <concepts>
#include <iostream>

// A type is a Shape if it has an Area() that gives back something convertible to double
template <typename T>
concept Shape = requires(T shape)
{
    { shape.Area() } -> std::convertible_to<double>;
};

template <Shape T>
void PrintArea(const T& shape)
{
    std::cout << "Area: " << shape.Area() << '\n';
}
```

Now two shapes, written by different people, with **no shared base class**:

```cpp
struct t_Circle
{
    double radius;

    double Area() const
    {
        return 3.14159 * radius * radius;
    }
};

struct t_Square
{
    double side;

    double Area() const
    {
        return side * side;
    }
};
```

And a type that is **not** a shape:

```cpp
struct t_Rock
{
    double weight;
};
```

Using them:

```cpp
t_Circle circle{2.0};
t_Square square{3.0};
t_Rock rock{10.0};

PrintArea(circle); // OK: Area: 12.5664
PrintArea(square); // OK: Area: 9
PrintArea(rock);   // Error: t_Rock does not satisfy Shape (no Area())
```

Without the concept, `PrintArea(rock)` would still fail, but with a long error from deep
inside the function body. With the concept, the error is short and points straight at the problem:
**"`t_Rock` is not a `Shape`."**

### Why this is powerful

- No inheritance, no `virtual`, no interface class. A type just has to **behave** like a shape.
- Anyone can add a new shape later (`t_Triangle`) and `PrintArea` works without any change.
- The rules of your function are written in its signature, so the code explains itself.

This is the real idea behind constraints: **you describe what you need, and the compiler checks it for you.**

---

## 9. Conclusion: the one idea to remember

If you forget everything else, remember this sentence:

> **The key to writing good template code is to make it work for all types, but only in a valid way.**

That sentence has two halves, and generic programming needs **both**:

| Half | Tool | What it gives you |
|---|---|---|
| **Work for all types** | Templates | Freedom: write once, reuse for any type, even ones that don't exist yet |
| **Only in a valid way** | Constraints and concepts | Validation: reject wrong types early, with clear errors |

Templates alone are freedom without rules: `Add` happily glues two strings together.
Constraints alone are rules without freedom. Together they give you code that is
**reusable, safe, and readable**.

A simple habit to build: whenever you write `template <typename T>`, ask yourself
*"What must be true about `T` for this code to make sense?"* Then write that answer as a concept.

### When is this most useful?

Templates shine brightest in **library and API development**: code written by one person
and used by many others.

- As a library author, you **cannot know** which types your users will bring. Templates let you support all of them.
- Your users call your code but rarely read its insides. Constraints are the **contract** that tells them what is allowed, and the compiler enforces it for them.
- A shared API lives for years and meets many different projects. A clear error like *"`t_Rock` is not a `Shape`"* saves every one of those users from a confusing wall of text.
- This is exactly how the C++ standard library (`std::vector`, `std::sort`, `std::ranges`) and most popular C++ libraries are designed.

For a small program that only ever uses one or two known types, a plain function is often enough.
Reach for templates when the code is **reused**, **shared**, or must work with **types you don't control**.

### Key takeaways

- A template is a **recipe**; the compiler writes the real code for each type you use.
- Templates remove duplication and make it possible to support **types that don't exist yet**.
- They run at **compile time**, so they cost nothing at runtime.
- They go far beyond functions: class templates, values as parameters, specialization, compile-time computation, and variadic arguments.
- They are more powerful than generics in most languages: faster, more flexible, and able to compute at compile time.
- By default templates accept **any** type, so correctness is **your** job.
- **Concepts** let you restrict types by *type category* (`Numeric`) or by *behavior* (`Shape`), avoid silent bugs, and get readable errors.
- They matter most in **libraries and shared APIs**, where the author can't know the user's types.
- None of this is hard. Start with `template <typename T>`, add a concept, and build from there.

---
