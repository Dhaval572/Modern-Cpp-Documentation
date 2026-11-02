# Move Semantics in C++: The Story of Cpplover, ForeignRef and the Moving Truck

> **Remember this one line for life:**
> **`std::move` is just "moving on". Nothing physically moves. It only changes how you look at an object. The real taking is done by the move constructor.**

This guide is a story. Read it like a story. The full code is at the very end, so you first understand *why*, and only then see *how*.

**Chapters**

1. Meet Cpplover: Lvalue and Rvalue
2. The Notebook Problem: Why Copying Hurts
3. Moving On: What `std::move` Really Does
4. The Moving Truck: Move Constructor and Move Assignment
5. The Two Keys Disaster: Why the Old Object Must Stay Safe
6. The House Seller and the Fruit Seller: Reset Is Not Always Zero
7. Why Plain Numbers Never Move (and Why We Use `std::exchange`)
8. After the Move: Valid but Unspecified vs Undefined Behavior
9. You Are the Shop Owner: Be Responsible
10. Rule of 0, 3 and 5
11. The "Do Not Disturb" Trap: Destructor Without Move
12. Conclusion: Why This Matters in OOP
13. Quick Questions and Answers
14. The Full Code (Everything in One Place)

**Code style in this guide:** variables are `snake_case` (class members start with `m_`), functions and classes are `PascalCase`, and curly brackets always start on a new line.

---

## 1. Meet Cpplover: Lvalue and Rvalue

Cpplover is a college student. He has a crush named **ForeignRef**. 

Let us look at ForeignRef from a C++ point of view:

- She has a **name**. Everyone knows she is ForeignRef.
- She has a **home address**. She is an existing, named object in memory.
- She will be in college **tomorrow**, and next week too. She does not disappear after one sentence. (Cpplover is just holding a *reference* to her).

In C++, anything like ForeignRef is an **lvalue**:

> **Lvalue = has a name, has an address, and stays alive after the current line.**

Now another day. Cpplover is sitting in a bus. A girl sits near him and gives a quick smile. Cpplover's heart does a little dance. Let's call her **TempVal**. But:

- He does not know her **name**.
- He cannot find her **address**. (He tries, but she gets down at the next stop.)
- After the bus stops, she is **gone forever**.

In C++, anything like TempVal is an **rvalue**:

> **Rvalue = a temporary. No name, no address you can use, and it is gone at the end of the line.**

Two simple tests to know which one you have:

| Test | ForeignRef (lvalue) | TempVal (rvalue) |
|---|---|---|
| Does it have a name? | Yes | No |
| Can you ask for its address? (`&`) | Yes | No |
| Will it still exist on the next line? | Yes | No |

A tiny bit of code, just to connect the story with C++:

```cpp
int foreign_ref = 10;        // 'foreign_ref' is an lvalue: it has a name
int* address = &foreign_ref; // allowed: lvalues have an address
// int* oops = &10;          // ERROR: '10' is a TempVal. No address.
```

In C++ we write a reference to an lvalue as `T&`, and a reference to an rvalue as `T&&`. Think of them as **two kinds of ears**:

- `T&` ears listen for: *"Hey, this is someone who is staying (ForeignRef). Be careful."*
- `T&&` ears listen for: *"Hey, this is a passer-by who is leaving anyway (TempVal)."*

And one small trap that confuses everybody:

> **The moment Cpplover learns TempVal's name, she is not a stranger anymore.** She now has a name and an address, so she is an lvalue now.
>
> In C++ it is the same. If a variable has a name, it is an lvalue, **even if its type is `T&&`**.

Keep this trap in your head. We will use it soon.

---

## 2. The Notebook Problem: Why Copying Hurts

Now Cpplover wants the notes of ForeignRef. Her notebook has **1000 pages**. It is the "insides" of ForeignRef in our story. In C++ this is the **heap data** that an object owns.

**Situation A:** ForeignRef is still studying from the notebook. Cpplover cannot take it. She needs it! So he goes to the photocopy shop and copies all 1000 pages. It is slow, it costs money, but it is the only fair way.

> This is a **copy**. We use it for **lvalues**, because they are staying and still need their things.

**Situation B:** TempVal's old notebook is thrown into the dustbin. She does not need it anymore. Would Cpplover still go to the photocopy shop and make 1000 copies? No! He picks up the notebook from the dustbin and walks away. Fast, free, no copy.

> This is a **move**. We use it for **rvalues**, because they are leaving anyway.

Now you understand the whole reason move semantics exists:

> **Copying something that is about to die is like photocopying a book and then burning the original. Silly. Just take the original.**

In C++, a "notebook" can be a big array, a big string, an image with millions of pixels. Copying means making new memory and copying every byte. Moving means copying **one pointer** (8 bytes) and changing the owner.

---

## 3. Moving On: What `std::move` Really Does

One day ForeignRef says to Cpplover, *"Sorry, we are just friends."*

Cpplover thinks for a while and decides: **"Okay. From today, ForeignRef is just a passer-by in my life. I am moving on."**

That is exactly `std::move(foreign_ref)`.

Now think carefully. When Cpplover "moves on", did anything happen to ForeignRef?

- Did ForeignRef move to another city? **No.**
- Did her address change? **No.**
- Did her notebook change? **No.**
- Is she still alive and well? **Yes.**

**Nothing happened to ForeignRef.** Only **Cpplover's way of looking at her** changed. She was a "named, staying person". Now Cpplover treats her as "a passer-by".

This is why:

> **`std::move` does not move anything. It is only a cast. It changes an lvalue into an rvalue. That is all.**

So why is this useful? Remember the two kinds of ears from Chapter 1?

- When Cpplover says "ForeignRef" (an lvalue), the `T&` ears hear it. The code says: *"Be careful, photocopy only."*
- When Cpplover says "ForeignRef, but she is a passer-by now" (`std::move(foreign_ref)`), the `T&&` ears hear it. The code says: *"Passer-by? Then take her notebook from the dustbin."*

And who does the actual carrying of the notebook? Not `std::move`. It is the **move constructor** (or move assignment) that does the work. Think of it like this:

> `std::move` is the **"SOLD" sign** on the house. The **move constructor** is the **moving truck**. The sign cannot carry furniture.

So, what do we really gain? We do **not** gain a "magical move". We gain **no deep copy**. That is the entire game: *avoid the photocopy shop, improve performance.*

---

## 4. The Moving Truck: Move Constructor and Move Assignment

The moving truck comes in two kinds. The difference is easy:

**Move constructor = Cpplover is buying a brand new empty flat.**
There is nothing in it. The truck just brings the furniture in. Simple.

**Move assignment = Cpplover already lives in a flat full of old furniture, and now he is receiving new furniture.**
First he must **clear out his old furniture** (free the old memory). Only then can the truck bring the new things in. If he forgets, the old furniture is lost forever. That is a **memory leak**.

There is one more silly danger in move assignment: **self-assignment**.

> Imagine Cpplover says: "Take the furniture from my own flat and put it in my own flat." If he first throws out his old furniture and *then* tries to take it from himself... there is nothing left! So we check at the start: "Am I the same person?" If yes, do nothing.

In C++ this check is `if (this != &other)`.

| | Move constructor | Move assignment |
|---|---|---|
| Story | Buying a new empty flat | Living in a full flat, getting new furniture |
| Does the target own old data? | No | **Yes, free it first** |
| Self-assignment check needed? | No | **Yes** |

One more thing: the keyword **`noexcept`**.

> The moving company has a rule: *"We will carry your things only if you promise that nothing will break on the way."* If you do not promise (no `noexcept`), then when `std::vector` needs to grow, it says: "I don't trust you. I will go to the photocopy shop instead." So it **copies** everything. Your program becomes slow and **nobody tells you**.

So always write `noexcept` on your move functions.

---

## 5. The Two Keys Disaster: Why the Old Object Must Stay Safe

Cpplover buys a house. The old owner, Mr. Sharma, gives him the key. But Mr. Sharma, being a lazy man, **keeps his own copy of the key too**.

Now two people think the house belongs to them. One day the house is old, and both of them call the demolition company. The company demolishes the house. Then the second order comes: *"Demolish the house."* But the house is already gone! The demolition worker is confused, falls down, and the whole thing becomes a disaster.

This is a **double delete**. In C++, it is undefined behavior, and the program usually crashes.

This is the C++ version of the bug (a very small piece, just to see it):

```cpp
m_data = other.m_data; // I took the key...
// ...but I forgot to take the key from Mr. Sharma!
```

**Why does this happen?** Because the old object (Mr. Sharma) is **still alive**. When the program ends, his destructor **will run** and he will try to demolish the house.

**The fix:** when Mr. Sharma gives the key, he must also say: *"I have no house now."* In C++ this means setting the pointer to `nullptr`:

```cpp
m_data = other.m_data;
other.m_data = nullptr; // YOU MUST DO THIS
```

Now when Mr. Sharma's destructor runs, it calls `delete` on `nullptr`. And C++ guarantees that deleting `nullptr` **does nothing**. No harm.

There is a nice bonus:

> A `nullptr` is not only safe. It is also **easy to check**. Anyone can write `if (pointer != nullptr)` and know: *"Aha, Mr. Sharma has no house now."*

So the first big lesson of move semantics is:

> **After the move, the old object is still alive. Leave it in a safe state.**

---

## 6. The House Seller and the Fruit Seller: Reset Is Not Always Zero

Beginners learn a rule: *"After a move, set everything to 0."*

This is **not always true**. Whether `0` is good depends on **what the value means**. Two stories will make this clear forever.

### The House Seller (unique things)

Mr. Sharma sold his house. Now he has two choices:

1. Stay homeless for the rest of his life (an invalid id like `0`), or
2. **Buy a new house** (a new unique id).

What he can **never** do is keep living in the old house. The buyer owns it now. **He must forget the old one.**

Now think about a C++ class `Player` that has a unique id. Every player must have a different id. Look what happens if we reset the moved-from player to `0`:

- Maybe some real player already has id `0`. Now two players share one id.
- Even if `0` is "reserved for empty", a hundred moved-from players would **all have id 0**. Now your maps, sets and databases are confused.

So the moved-from player must get a **brand new unique id**. Not `0`. And not the old id.

### The Fruit Seller (normal things)

A fruit seller sells all his apples. Now he has `0` apples. Is that a problem? No!

Apples are **not unique**. He is still a fruit seller. He still lives, still works, and he can still eat other fruits. Zero apples is a **normal, valid** state.

So for a simple counter, `0` is perfectly fine.

### What to remember

| Thing | What to leave behind |
|---|---|
| Owning pointer | `nullptr` |
| Counter or size | `0` |
| Container | empty |
| **Unique id** | **a new unique id (never the old one)** |
| **File descriptor** | **`-1` (because `0` is `stdin`, the keyboard!)** |

Before you choose the reset value, ask yourself one question:

> **"Is this value a normal, harmless state for my type?"**

If yes, use it. If not, choose something else.

---

## 7. Why Plain Numbers Never Move (and Why We Use `std::exchange`)

This part confuses a lot of people, so let us make it simple.

Imagine Cpplover writes the number **42** on a sticky note. His friend asks for it. Cpplover writes **42** on another sticky note and gives it. Does Cpplover's own note become blank? **No.** He still has 42.

A plain number (`int`, `double`, `char`) and a raw pointer are like this sticky note. The number *is* the whole thing. There is **nothing inside it to take away**. So "moving" an `int` is exactly the same as **copying** an `int`.

> **Primitive types never move.** `std::move(some_int)` gives you a copy, and the original still has its value.

Now see the danger: a raw pointer is also like a sticky note, and the note holds an **address**. If you "move" a raw pointer, you only copy the note. **Both notes now hold the same address.** This is the Two Keys disaster again!

Even when you write `= default` for the move constructor, a raw pointer member is **only copied**. The compiler will not blank the old note for you.

So **we must blank the old note ourselves**. And C++ gives us a handy tool for this: `std::exchange`.

> `std::exchange(old_note, blank)` means: **"Give me what is written on the old note, and write this blank value there instead."**

```cpp
m_data = std::exchange(other.m_data, nullptr);
// m_data gets the old address, and other.m_data becomes nullptr. One step.
```

It is the short way to write three lines:

```cpp
int* old_value = other.m_data;
other.m_data = nullptr;
m_data = old_value;
```

So the rule is:

> **For raw pointers, handles, counters and flags: copy the value, then reset the source.** (Types like `std::string`, `std::vector` and `std::unique_ptr` already do this inside, so you don't need to.)

---

## 8. After the Move: Valid but Unspecified vs Undefined Behavior

Remember ForeignRef? After Cpplover "moved on", she was still alive and well. She could still study, make new friends, and even get a new notebook. But Cpplover no longer knows what is in her bag today. He should not guess.

That is exactly what a moved-from object is in C++: **valid but unspecified**.

Many people confuse this with **undefined behavior (UB)**. They are very different:

| | **Undefined Behavior (UB)** | **Valid but Unspecified** |
|---|---|---|
| Story | The building collapsed | The hotel room was checked out |
| Is the program correct? | **No, it is broken** | Yes |
| Can I destroy it? | It might crash | **Always safe** |
| Can I give it a new value? | It might crash | **Always safe** |
| Can I read its value? | Garbage or a crash | Safe, but **do not guess the value** |
| Example | Using a deleted pointer, double delete | A `std::string` after a move |

Think of a hotel room after the guest has left:

- **Valid but unspecified:** the room is fine. Maybe it is clean, maybe a sock is left on the bed. You do not know. But a new guest can still move in, and the door still opens.
- **UB:** the hotel building fell down. Nothing is safe now.

So after a move, you may:

- destroy the object,
- give it a new value,
- call functions that have no conditions, like `empty()` or `clear()`.

But you should **not** assume what value it holds.

---

## 9. You Are the Shop Owner: Be Responsible

Think of a shop owner. When he closes the shop, he does not just run away. He turns off the lights, locks the door, and leaves the shop in a clean, safe state.

When *you* write a move constructor, *you* are the shop owner. The compiler will **not** clean up for you. You promise three things:

1. The moved-from object can be **destroyed** safely.
2. The moved-from object can be **given a new value** safely.
3. The moved-from object still **follows the rules of the class** (it is not half-broken).

The people who wrote the C++ standard library (the STL authors) did this very carefully for every type:

- A moved-from `std::vector` is **empty**.
- A moved-from `std::unique_ptr` is **`nullptr`**.
- A moved-from `std::string` is a **valid string** (we just don't know what is in it).

We must give the same quality in **our own classes and our own APIs**. And we should write it in a comment, so the next person knows:

```cpp
// After a move, a RawBuffer is empty (Size() == 0).
// It is safe to destroy it or give it a new value.
```

A move that leaves a half-broken object is a bug that waits quietly for 3 AM in production.

---

## 10. Rule of 0, 3 and 5

These rules answer one question: **"Which special functions should I write in my class?"**

The special functions are: destructor, copy constructor, copy assignment, move constructor and move assignment.

### Rule of 3 (the old rule, before C++11)

> If you write **one** of these, you most likely need **all three**: destructor, copy constructor, copy assignment.

Story: you built a custom lock for your house (destructor). Now when somebody makes a copy of your key, the compiler's automatic copy just makes a **second key for the same lock**. Two people, one house, the Two Keys disaster again. So you must also write how to copy properly.

### Rule of 5 (the modern rule)

> Rule of 3, **plus** move constructor and move assignment.

Story: now you also teach your class how to use the moving truck, so it does not go to the photocopy shop for no reason.

### Rule of 0 (the best rule)

> **Write none of them.**

Story: you hire a **professional** (`std::string`, `std::vector`, `std::unique_ptr`) to look after every resource. The professional already knows copy, move and cleanup. You just use them as members, and the compiler-made functions are correct.

| Rule | Story | Work for you | Safety |
|---|---|---|---|
| **0** | Hire a professional | None | Best |
| **3** | Old method, only photocopy | Medium | No move |
| **5** | Do it yourself, with a truck | Most | Copy and fast move |

> **Choose Rule 0 first. Use Rule 5 only when you must hold a raw resource yourself. Rule 3 is old history.**

---

## 11. The "Do Not Disturb" Trap: Destructor Without Move

This is the sneaky one. It makes programs slow for years and nobody notices.

You have a class `Frame` that holds a big image. You write **only a destructor**, maybe just to print a log message. It looks innocent.

Here is what happens behind the scenes. The C++ compiler sees your destructor and thinks:

> *"This programmer is writing special cleanup code. He is being careful. Maybe the automatic move is not safe for him. Let me not make it."*

So the compiler **quietly stops creating** the move constructor and move assignment. It is like hanging a **"Do Not Disturb"** sign on the door, and the moving truck cannot come in.

Now when you write `std::move(first_frame)`, the code looks for a move constructor and does not find one. So what does it use? The **copy constructor**! (A temporary `Frame&&` can also be accepted by `const Frame&`.)

So **the whole image is deep-copied**. No error. No warning. Just slow.

It gets worse. When a `std::vector<Frame>` grows and needs more space, it should *move* each frame to the new place. But with no move constructor, it **photocopies every single frame**. A program that should take 1 second now takes 10, and nobody knows why.

**The fix is simple:** if you write a destructor (or a copy constructor or copy assignment), write all five functions, even if you just write `= default`. Or follow Rule 0 and write none.

> **Remember:** write a destructor alone, and the moving truck is cancelled.

---

## 12. Conclusion: Why This Matters in OOP

Object-oriented programming is full of **objects that own things**: memory, files, sockets, locks, GPU buffers, unique ids. Before move semantics, C++ gave us two bad choices to pass these objects around:

- **Copy:** safe but slow, and sometimes meaningless. (What does it mean to "copy" a network socket?)
- **Raw pointers:** fast, but nobody clearly owns the data. So we get leaks and double deletes.

Move semantics gives us the third and correct choice: **transfer the ownership.**

What it gives to OOP:

1. **Speed:** a big object changes owner in O(1) time, not O(n).
2. **Honest design:** unique things (file handles, threads, locks, `unique_ptr`) can be **move-only**. They can change owner but can never be copied.
3. **Clear ownership:** at any time, exactly one object owns a resource. This is the heart of RAII.
4. **Clean code:** you can return big objects from functions by value without worry.
5. **Responsibility:** the class author promises a safe and valid moved-from state, so the rules of the object stay true.

The key facts to keep forever:

- `std::move` **moves nothing**. It is "moving on": a cast from lvalue to rvalue.
- A "move" is really **avoiding the deep copy**: copy the pointer, reset the source. That is where the speed comes from.
- **Primitive types never move.** Their "move" is a plain copy, so we reset them by hand with `std::exchange`.
- The moved-from object must be **valid, destroyable and assignable**. Its value is *unspecified*, but its behavior is never *undefined*.
- Reset to **what makes sense**: `nullptr`, `0`, empty, `-1`, or a *new unique id*.
- Prefer **Rule of 0**. Use **Rule of 5** for raw resources. A lonely destructor **silently turns off move**.

---

## 13. Quick Questions and Answers

| Question | Answer |
|---|---|
| Does `std::move` move data? | No. It is "moving on": only a cast to rvalue. |
| Who does the real work? | The move constructor or move assignment. |
| What do we gain? | We avoid a deep copy, so we get better performance. |
| Does `int` have a move? | No. Its "move" is a copy. |
| Why use `std::exchange`? | Take the old value and leave a safe one, in one step. |
| Is a moved-from object UB? | No. It is valid but unspecified. |
| Always reset to 0? | No. Fruit seller: yes. House seller: no. |
| Forgot `noexcept`? | `std::vector` will copy instead of move. |
| Wrote only a destructor? | Move is off. Copies happen quietly. |
| Which rule should I follow? | Rule of 0 first. Rule of 5 when you hold a raw resource. |

> **Last memory hook:** *Cpplover "moves on" from ForeignRef, but ForeignRef is still alive. Sell the house, don't keep the keys. Sell the apples, you are still a fruit seller. And `std::move` is only the "SOLD" sign, not the truck.*

---

## 14. The Full Code (Everything in One Place)

Now that you know the story, here is the code. Each part matches a chapter above.

### Code 1: Lvalue and rvalue, and the two kinds of ears (Chapters 1 and 3)

```cpp
#include <iostream>
#include <string>
#include <utility>

void Describe(const std::string& value)
{
    std::cout << "lvalue ears: this one is staying, so I COPY\n";
}

void Describe(std::string&& value)
{
    std::cout << "rvalue ears: this one is leaving, so I TAKE\n";
}

std::string MakeName()
{
    return "TempVal"; // a temporary: no name, gone after the line
}

int main()
{
    std::string foreign_ref = "ForeignRef";

    Describe(foreign_ref);              // lvalue ears -> copy
    Describe(MakeName());               // rvalue ears -> take
    Describe(std::move(foreign_ref));   // "moving on": now rvalue ears -> take

    // The trap: a named parameter is an lvalue again
    // void Wrapper(std::string&& value) { Describe(value); }             // lvalue ears!
    // void Wrapper(std::string&& value) { Describe(std::move(value)); }  // rvalue ears
}
```

### Code 2: The full class with Rule of 5 (Chapters 2, 4, 5, 7, 9, 10)

```cpp
#include <algorithm>
#include <cstddef>
#include <utility>

// After a move, a RawBuffer is empty (Size() == 0).
// It is safe to destroy it or give it a new value.
class RawBuffer
{
public:
    explicit RawBuffer(std::size_t size)
        : m_data(new int[size]{})
        , m_size(size)
    {
    }

    ~RawBuffer()
    {
        delete[] m_data;
    }

    // Copy constructor: the photocopy shop
    RawBuffer(const RawBuffer& other)
        : m_data(new int[other.m_size])
        , m_size(other.m_size)
    {
        std::copy(other.m_data, other.m_data + other.m_size, m_data);
    }

    // Copy assignment: the photocopy shop
    RawBuffer& operator=(const RawBuffer& other)
    {
        if (this != &other)
        {
            int* new_data = new int[other.m_size];
            std::copy(other.m_data, other.m_data + other.m_size, new_data);

            delete[] m_data;
            m_data = new_data;
            m_size = other.m_size;
        }
        return *this;
    }

    // Move constructor: the moving truck, new empty flat
    RawBuffer(RawBuffer&& other) noexcept
        : m_data(std::exchange(other.m_data, nullptr))
        , m_size(std::exchange(other.m_size, 0))
    {
    }

    // Move assignment: the moving truck, but first clear the old furniture
    RawBuffer& operator=(RawBuffer&& other) noexcept
    {
        if (this != &other)
        {
            delete[] m_data;                              // clear old furniture
            m_data = std::exchange(other.m_data, nullptr);
            m_size = std::exchange(other.m_size, 0);
        }
        return *this;
    }

    std::size_t Size() const
    {
        return m_size;
    }

private:
    int* m_data;
    std::size_t m_size;
};
```

How to use it:

```cpp
RawBuffer first_buffer(1000);
RawBuffer second_buffer = std::move(first_buffer); // move constructor
// first_buffer.Size() is now 0: safe and easy to check

RawBuffer third_buffer(5);
third_buffer = std::move(second_buffer);           // move assignment
```

### Code 3: House seller and fruit seller (Chapter 6)

```cpp
#include <utility>

// House seller: the id is UNIQUE, so the old object gets a NEW id
class Player
{
public:
    Player()
        : m_id(GenerateNextId())
    {
    }

    Player(Player&& other) noexcept
        : m_id(other.m_id)                  // the new owner takes the id
    {
        other.m_id = GenerateNextId();      // the old owner buys a NEW house
    }

    int Id() const
    {
        return m_id;
    }

private:
    static int GenerateNextId()
    {
        static int next_id = 1;             // we never give out 0
        return next_id++;
    }

    int m_id;
};

// Fruit seller: apples are NOT unique, so 0 is perfectly fine
class Stall
{
public:
    explicit Stall(int apple_count)
        : m_apple_count(apple_count)
    {
    }

    Stall(Stall&& other) noexcept
        : m_apple_count(std::exchange(other.m_apple_count, 0))
    {
    }

private:
    int m_apple_count;
};
```

### Code 4: The "Do Not Disturb" trap and its fix (Chapter 11)

```cpp
#include <vector>

// BAD: only a destructor. The moving truck is cancelled. Every "move" is a copy.
class SlowFrame
{
public:
    ~SlowFrame()
    {
        // maybe just a log message
    }

private:
    std::vector<unsigned char> m_pixels; // big data
};

// GOOD: write all five, even if they are only "= default"
class FastFrame
{
public:
    FastFrame() = default;

    ~FastFrame()
    {
        // custom cleanup
    }

    FastFrame(const FastFrame&) = default;
    FastFrame& operator=(const FastFrame&) = default;

    FastFrame(FastFrame&&) noexcept = default;            // the truck is back
    FastFrame& operator=(FastFrame&&) noexcept = default;

private:
    std::vector<unsigned char> m_pixels;
};
```

### Code 5: Rule of 0, the best rule (Chapter 10)

```cpp
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Document
{
public:
    explicit Document(std::string title)
        : m_title(std::move(title))
    {
    }

private:
    std::string m_title;                // the professional looks after it
    std::vector<int> m_page_numbers;    // the professional looks after it
    std::unique_ptr<int[]> m_cache;     // the professional looks after it
};
// No destructor, no copy, no move written. The compiler-made ones are correct.
```

### Code 6: Real-world example: a file handle that can only move (modern C++, Linux)

This shows why "reset to zero" is dangerous. File descriptor `0` is **standard input**. If a moved-from object reset to `0` and then closed it, your program would close the keyboard!

```cpp
#include <unistd.h>
#include <utility>

class FileHandle
{
public:
    FileHandle() = default;

    explicit FileHandle(int file_descriptor)
        : m_file_descriptor(file_descriptor)
    {
    }

    ~FileHandle()
    {
        Reset();
    }

    // A unique OS handle cannot be copied, so copy is deleted
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    FileHandle(FileHandle&& other) noexcept
        : m_file_descriptor(std::exchange(other.m_file_descriptor, -1)) // NOT 0!
    {
    }

    FileHandle& operator=(FileHandle&& other) noexcept
    {
        if (this != &other)
        {
            Reset();
            m_file_descriptor = std::exchange(other.m_file_descriptor, -1);
        }
        return *this;
    }

    bool IsValid() const
    {
        return m_file_descriptor >= 0;
    }

private:
    void Reset()
    {
        if (m_file_descriptor >= 0)
        {
            ::close(m_file_descriptor);
            m_file_descriptor = -1;
        }
    }

    int m_file_descriptor = -1;
};
```

How to use it:

```cpp
FileHandle OpenLog(const char* path); // returns by value: moved (or skipped), never copied

FileHandle log_file = OpenLog("/tmp/app.log");
FileHandle other_owner = std::move(log_file); // ownership changes hands
// log_file.IsValid() is false -> easy to check, and the destructor does nothing
```

### Code 7: Real-time example: audio pipeline without copying (modern C++)

In a real-time system, copying or allocating big buffers in the audio thread causes **glitches (sound dropouts)**. So we pass the **ownership** instead of the data.

```cpp
#include <cstddef>
#include <memory>
#include <queue>
#include <utility>

class AudioBlock
{
public:
    explicit AudioBlock(std::size_t sample_count)
        : m_samples(std::make_unique<float[]>(sample_count))
        , m_sample_count(sample_count)
    {
    }

    // unique_ptr already makes this class move-only.
    // We write the move functions so the moved-from count is a clear 0.
    AudioBlock(AudioBlock&& other) noexcept
        : m_samples(std::move(other.m_samples))
        , m_sample_count(std::exchange(other.m_sample_count, 0))
    {
    }

    AudioBlock& operator=(AudioBlock&& other) noexcept
    {
        if (this != &other)
        {
            m_samples = std::move(other.m_samples);
            m_sample_count = std::exchange(other.m_sample_count, 0);
        }
        return *this;
    }

    std::size_t SampleCount() const
    {
        return m_sample_count;
    }

private:
    std::unique_ptr<float[]> m_samples;
    std::size_t m_sample_count;
};

class AudioQueue
{
public:
    void Push(AudioBlock block)            // take by value, then move into the queue
    {
        m_blocks.push(std::move(block));   // no sample is copied
    }

    AudioBlock Pop()
    {
        AudioBlock front_block = std::move(m_blocks.front());
        m_blocks.pop();
        return front_block;                // moved (or skipped) out
    }

private:
    std::queue<AudioBlock> m_blocks;
};
```

How to use it:

```cpp
AudioQueue queue;
queue.Push(AudioBlock(4096));      // temporary -> moved in: only 8 bytes change hands, 16 KB is not copied
AudioBlock next_block = queue.Pop();

// Even better: build the object right inside the container
std::vector<AudioBlock> blocks;
blocks.reserve(8);
blocks.emplace_back(4096);         // no temporary, no move
```

Best to worst: **build in place > move > copy.**

### Code 8: Copy this template for your own classes

```cpp
// Move constructor template
T(T&& other) noexcept
    : m_pointer(std::exchange(other.m_pointer, nullptr))   // owning pointer -> nullptr
    , m_size(std::exchange(other.m_size, 0))               // counter -> 0
    , m_handle(std::exchange(other.m_handle, -1))          // OS handle -> -1
{
}

// Move assignment template
T& operator=(T&& other) noexcept
{
    if (this != &other)
    {
        ReleaseMyResources();                              // clear the old furniture first
        m_pointer = std::exchange(other.m_pointer, nullptr);
        m_size = std::exchange(other.m_size, 0);
    }
    return *this;
}
```

**The end.** Next time somebody asks you what `std::move` does, smile and say: *"It moves on. Nothing else."*
