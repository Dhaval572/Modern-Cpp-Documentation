# `char` : What It Really Is

> You were told `char` stores a character like `'A'` or `'z'`.  
> That is true — but it is only **half the story**.  
> This guide starts from zero and tells you the whole truth.

---

## Table of Contents

1. [Computers Only Understand Numbers](#1-computers-only-understand-numbers)
2. [The Problem — How Do We Store Letters?](#2-the-problem--how-do-we-store-letters)
3. [The Solution — ASCII](#3-the-solution--ascii)
4. [So What is `char` Really?](#4-so-what-is-char-really)
5. [Why `char` Was Chosen Historically](#5-why-char-was-chosen-historically)
6. [What is `char*` Then?](#6-what-is-char-then)
7. [The Problem With Using `char` for Everything](#7-the-problem-with-using-char-for-everything)
8. [The Modern Replacements](#8-the-modern-replacements)
9. [Summary — The Big Picture](#9-summary--the-big-picture)

---

## 1. Computers Only Understand Numbers

At the deepest level, a computer has no idea what a letter is.  
It only knows **bits** — tiny switches that are either **ON (1)** or **OFF (0)**.

Eight of these bits grouped together form one **byte**.

```
One byte:  1 0 1 0 0 0 0 1
           │ │ │ │ │ │ │ │
           └─┴─┴─┴─┴─┴─┴─┴── 8 switches (bits)
```

One byte can represent **256 different patterns** (from `00000000` to `11111111`), which is the same as the numbers **0 to 255**.

That is all a computer stores in its memory — **numbers**. Nothing else.

---

## 2. The Problem — How Do We Store Letters?

Early computer scientists faced a very human problem:

> Computers can only store numbers.  
> Humans want to read and write text.  
> How do we bridge that gap?

The answer was simple — **make an agreement**.

> *"Let's all agree that the number 65 means the letter A,  
> the number 66 means B, the number 67 means C..."*

This agreement is called an **encoding** — a table that maps numbers to symbols.

---

## 3. The Solution — ASCII

In 1963, the Americans published the **ASCII table** — American Standard Code for Information Interchange.

It was a simple lookup table that every computer and program agreed to use:

```
Number  →  Symbol
──────────────────
65      →  A
66      →  B
67      →  C
...
97      →  a
98      →  b
99      →  c
...
48      →  0  (the digit zero)
49      →  1
...
32      →  ' '  (space)
10      →  newline
```

### 3.1 Why is `A` the number 65 — and not 1?

This is a great question. The short answer: **numbers 0–64 were already taken**.

The ASCII table does not start with letters. It starts with **control characters** — invisible commands that old machines needed to function. For example:

```
Number  →  Meaning
──────────────────────────────────────────
0       →  Null (end of string marker)
7       →  Bell (literally ring a bell on the terminal!)
8       →  Backspace
9       →  Tab
10      →  Line Feed (move down one line)
13      →  Carriage Return (move cursor to start of line)
27      →  Escape
```

These came from the era of **teletype machines** (typewriter-like devices connected over wires). The numbers 0–31 are all such control commands. They have no visible shape — they perform actions.

Then numbers 32–64 were given to **punctuation and digits**:

```
32  →  (space)
33  →  !
48  →  0   ← digits start here
57  →  9   ← digits end here
64  →  @
```

Only after all of that did the alphabet begin:

```
65  →  A   ← uppercase letters start here
90  →  Z   ← uppercase letters end here
97  →  a   ← lowercase letters start here
122 →  z   ← lowercase letters end here
```

### 3.2 There is a Beautiful Pattern Hidden Here

Notice the gap between `A` (65) and `a` (97) is exactly **32**.

```
'A'  =  65  =  01000001  (in binary)
'a'  =  97  =  01100001  (in binary)
                 ↑
            only this one bit is different
```

This was a deliberate design choice. To convert any uppercase letter to lowercase, you just **flip one bit** (or add 32). Early computers could do this extremely fast — no lookup table needed, pure bit manipulation.

```c
char upper = 'A';           // 65
char lower = upper + 32;    // 97 = 'a'
// or with bitwise OR:
char lower2 = upper | 32;   // same result, one CPU instruction
```

This is why 65 was chosen for `A` — not because it is a random number, but because it was the first available slot after reserving space for control characters and symbols, and it created this elegant 32-apart uppercase/lowercase relationship.

So when you write `'A'` in your code, the compiler quietly stores the number **65**.  
When you print it with `%c`, the terminal looks up 65 in the ASCII table and draws the letter **A**.

```c
char letter = 'A';
printf("%d\n", letter);   // prints: 65   (the number)
printf("%c\n", letter);   // prints: A    (the symbol)
```

**These two lines print the exact same byte — just displayed differently.**

---

## 4. So What is `char` Really?

`char` is a **1-byte integer**.

That is the complete truth. Nothing more, nothing less.

```
char = 1 byte = a number between 0 and 255 (or -128 to 127)
```

When you write:

```c
char c = 'A';
```

Your compiler does not store a letter. It stores the **number 65** in one byte of memory.  
The letter `A` only appears when something (a terminal, a screen, a printer) reads that byte and **draws the symbol** that the ASCII table says corresponds to 65.

> The letter lives in the **eye of the beholder** — not in the computer's memory.

---

## 5. Why `char` Was Chosen Historically

### 5.1 The Year Was ~1972

When Dennis Ritchie created the C language, computers were very different:

- Memory was **extremely scarce** — a machine might have only 64 KB total
- There was no Unicode, no emoji, no internet, no non-English text in programs
- ASCII fit perfectly in 7 bits (128 symbols), and one byte was enough for everything
- Text was simple: English letters, digits, punctuation. That was it.

So the designers of C made a practical decision:

> *"The smallest data type will be 1 byte. We'll call it `char` because its most  
> common use is storing ASCII characters. But it's really just a byte."*

### 5.2 `char` Became the Universal Byte

Because `char` was the only 1-byte type in C, it ended up being used for **everything** that needed byte-level access:

- Storing text strings
- Reading binary files
- Network data packets
- Image pixel values
- Memory manipulation

It was never designed for all of this. It just happened because there was no alternative.

```c
// All of these used char*, even though most are NOT text:
char* name       = "Alice";          // text — makes sense
char* file_data  = read_file();      // binary file — does NOT make sense
char* packet     = receive_data();   // network bytes — does NOT make sense
char* pixels     = image.buffer;     // pixel values — does NOT make sense
```

This is the root of decades of confusion.

---

## 6. What is `char*` Then?

A `char*` is a **pointer to a byte** — it holds the memory address of one byte, and you can use it to walk through a sequence of bytes one by one.

```
Memory (each box = 1 byte):

Address:  1000   1001   1002   1003   1004   1005
         ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐
         │ 72 │ │101 │ │108 │ │108 │ │111 │ │  0 │
         └────┘ └────┘ └────┘ └────┘ └────┘ └────┘
           'H'   'e'   'l'   'l'   'o'   '\0'
            ↑
           char* points here
```

When you write `const char* word = "Hello"`, the variable `word` stores the number `1000` (the starting address). It does **not** store the string itself — just where the string begins in memory.

The "string" only exists because of two conventions:
1. The bytes happen to be ASCII values
2. There is a `0` byte at the end to signal where the text stops (the null terminator `'\0'`)

Remove either convention, and `char*` is just a pointer to a sequence of bytes.

---

## 7. The Problem With Using `char` for Everything

Because `char` was used for both text and raw binary data, the code became deeply confusing and bug-prone.

### Problem 1 — What Does This Function Do?

```c
void Process(char* data, int size);
```

Is `data` a string of text? Or raw binary? You cannot tell from the type alone.

### Problem 2 — Signs Are Confusing

`char` can be signed (values -128 to 127) on some platforms and unsigned (0 to 255) on others.

```c
char byte_value = 200;   // stored as -56 on a signed-char platform!

if (byte_value == 200)   // this might NEVER be true
    printf("Found it!\n");
```

### Problem 3 — String Functions Break on Binary Data

```c
char data[] = {72, 101, 0, 108, 111};   // contains a 0 in the middle

int len = strlen(data);   // returns 2 — stops at the 0 byte!
//  correct answer is 5, but strlen thinks the string ended at position 2
```

The function `strlen` was designed for text (where `0` means "end of string"). It has no idea about binary data.

All of these problems exist because `char` was forced to wear too many hats.

---

## 8. The Modern Replacements

Over time, C and C++ introduced better, more honest types.

### 8.1 For Text — Use `std::string` (C++)

```cpp
std::string name = "Alice";   // owns its memory, knows its length, safe
```

No null-terminator juggling. No manual memory management. Just text.

### 8.2 For Raw Bytes — Use `std::byte` (C++17)

C++17 finally gave bytes their own type:

```cpp
#include <cstddef>

std::byte pixel_value{255};   // clearly a byte, not a character
```

`std::byte` looks similar to `unsigned char` but it **refuses to act like a number**:

```cpp
std::byte b{65};

// ❌ These are compile errors — std::byte is not a character or number
char c = b;        // error
int  n = b;        // error
b + 1;             // error

// ✅ It only allows bitwise operations (AND, OR, shift)
// which are the only operations that make sense on raw bits
```

This is a feature. It forces you to be deliberate about what you are doing with bytes.

### 8.3 For UTF-8 Text — Use `char8_t` (C++20)

```cpp
const char8_t* greeting = u8"Hello, 世界!";   // explicitly UTF-8
```

This type says: *"I am a byte that is specifically a UTF-8 code unit, not arbitrary data."*

### 8.4 The Evolution at a Glance

```
1972  →  char*       "We only have one byte type. Use it for everything."
          │
1998  →  std::string  "At least use this for text. Stop juggling char arrays."
          │
2017  →  std::byte    "Raw bytes finally have their own type."
          │
2020  →  char8_t      "UTF-8 text finally has its own type."
          │
2020  →  std::span    "A safe, modern way to pass a sequence of bytes around."
```

---

## 9. Summary — The Big Picture

Here is everything in one place:

| What you were told | The full truth |
|---|---|
| `char` stores a character | `char` stores 1 byte (a number 0–255) |
| `'A'` is a letter | `'A'` is the number 65 stored in 1 byte |
| `char*` is a string | `char*` is a pointer to a sequence of bytes |
| The letter is in memory | The letter is in your terminal's font — memory only has numbers |

### The One Sentence Summary

> A `char` is a **1-byte integer**.  
> It looks like a character only because we agreed on a table (ASCII) that maps numbers to symbols.  
> The computer sees numbers. You see letters. The encoding is the bridge between both worlds.

### When to Use What

```
You want to store...          Use this
─────────────────────────────────────────────────
A text string                 std::string
A single letter/symbol        char
Raw binary data               std::byte (C++17)
A UTF-8 string                std::u8string (C++20)
A small number (0-255)        uint8_t
```

---

*Start here. Once this clicks, everything about pointers, strings, encoding, and binary data falls into place.*
