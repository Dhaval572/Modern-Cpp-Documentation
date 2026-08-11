# Zero-Overhead RAII With Custom Deleters: A Complete Guide

## What Problem Are We Solving?

When Working With C Libraries In C++, You Often Get Raw Pointers That Need Manual Cleanup:

```cpp
// C-style resource management (error-prone)
SDL_Window* window = SDL_CreateWindow(...);
// ... use window ...
SDL_DestroyWindow(window);  // Easy to forget!
```

**Problems:**
- ❌ Forgetting to call the destroy function = memory leak
- ❌ Early returns skip cleanup
- ❌ Exceptions skip cleanup
- ❌ Double-destroy if you're not careful

**Solution:** Use C++'s RAII (Resource Acquisition Is Initialization) with smart pointers!

---

## The Core Concept: Custom Deleters

`std::unique_ptr` can take a **custom deleter** — a function that gets called automatically when the pointer goes out of scope.

### Basic Syntax

```cpp
std::unique_ptr<Type, DeleterType> ptr;
```

Where:
- `Type` = what you're managing (e.g., `SDL_Window`)
- `DeleterType` = how to clean it up (e.g., call `SDL_DestroyWindow`)

---

## Method 1: Function Pointer Deleter (Simple But Overhead)

### Example: Managing a C FILE*

```cpp
#include <memory>
#include <cstdio>

// The deleter function
void FileDeleter(FILE* file_pointer)
{
    if (file_pointer)
    {
        fclose(file_pointer);
    }
}

int main()
{
    // Create unique_ptr with custom deleter
    std::unique_ptr<FILE, decltype(&FileDeleter)> file_handle(
        fopen("data.txt", "r"),
        FileDeleter
    );
    
    if (!file_handle)
    {
        return 1;  // No leak even on early return!
    }
    
    // Use the file
    char buffer[100];
    fgets(buffer, 100, file_handle.get());
    
    // fclose() called automatically when 'file_handle' goes out of scope
    return 0;
}
```

**Problem:** This stores a function pointer → adds 8 bytes of overhead per object.

---

## Method 2: Zero-Overhead Generic Deleter (The Smart Way)

### The Magic Template

```cpp
template <auto DestroyFn>
struct ResourceDeleter
{
    template <typename T>
    void operator()(T* pointer) const noexcept
    {
        if (pointer)
        {
            DestroyFn(pointer);
        }
    }
};
```

**What's Happening Here?**

1. `template <auto DestroyFn>` — Takes the destroy function as a **compile-time template parameter**
2. `operator()` — Makes this a callable object (functor)
3. `DestroyFn(pointer)` — Calls the function directly at compile time
4. **Zero runtime overhead** — The function is inlined, no function pointer stored!

---

## How To Use It: Step-By-Step

### Step 1: Define The Generic Deleter (Once)

```cpp
template <auto DestroyFn>
struct ResourceDeleter
{
    template <typename T>
    void operator()(T* pointer) const noexcept
    {
        if (pointer)
        {
            DestroyFn(pointer);
        }
    }
};
```

### Step 2: Create Type Aliases For Each Resource

```cpp
// SDL types
using WindowPtr   = std::unique_ptr<SDL_Window,   ResourceDeleter<SDL_DestroyWindow>>;
using RendererPtr = std::unique_ptr<SDL_Renderer, ResourceDeleter<SDL_DestroyRenderer>>;
using TexturePtr  = std::unique_ptr<SDL_Texture,  ResourceDeleter<SDL_DestroyTexture>>;

// Or for FILE*
using FilePtr = std::unique_ptr<FILE, ResourceDeleter<fclose>>;
```

### Step 3: Use Like Regular `unique_ptr`

```cpp
WindowPtr window_handle{SDL_CreateWindow(...)};
// Automatically calls SDL_DestroyWindow when out of scope!
```

---

## Simple Examples To Build Intuition

### Example 1: Managing a C FILE*

```cpp
#include <memory>
#include <cstdio>
#include <print>

template <auto DestroyFn>
struct ResourceDeleter
{
    template <typename T>
    void operator()(T* pointer) const noexcept
    {
        if (pointer)
        {
            DestroyFn(pointer);
        }
    }
};

using FilePtr = std::unique_ptr<FILE, ResourceDeleter<fclose>>;

int main()
{
    FilePtr file_handle{fopen("test.txt", "w")};
    
    if (!file_handle)
    {
        std::print("Failed to open file\n");
        return 1;
    }
    
    fprintf(file_handle.get(), "Hello, RAII!\n");
    
    // fclose() called automatically — no manual cleanup needed!
    return 0;
}
```

---

### Example 2: Managing malloc/free

```cpp
#include <memory>
#include <cstdlib>

template <auto DestroyFn>
struct ResourceDeleter
{
    template <typename T>
    void operator()(T* pointer) const noexcept
    {
        if (pointer)
        {
            DestroyFn(pointer);
        }
    }
};

using MallocPtr = std::unique_ptr<void, ResourceDeleter<free>>;

int main()
{
    MallocPtr buffer_handle{malloc(1024)};
    
    if (!buffer_handle)
    {
        return 1;
    }
    
    // Use buffer...
    int* data = static_cast<int*>(buffer_handle.get());
    data[0] = 42;
    
    // free() called automatically!
    return 0;
}
```

---

### Example 3: Custom C Library (OpenSSL-style)

```cpp
#include <memory>

// Imagine a C library with:
struct Connection;
Connection* CreateConnection(const char* host);
void DestroyConnection(Connection* connection);

// Our RAII wrapper:
template <auto DestroyFn>
struct ResourceDeleter
{
    template <typename T>
    void operator()(T* pointer) const noexcept
    {
        if (pointer)
        {
            DestroyFn(pointer);
        }
    }
};

using ConnectionPtr = std::unique_ptr<Connection, ResourceDeleter<DestroyConnection>>;

void ExampleFunction()
{
    ConnectionPtr connection_handle{CreateConnection("example.com")};
    
    if (!connection_handle)
    {
        return;  // Early return — no leak!
    }
    
    // Use connection...
    
    // DestroyConnection() called automatically
}
```

---

## Why This Is Zero-Overhead

### Assembly Comparison

**Manual Cleanup:**
```asm
call    SDL_CreateWindow
test    rax, rax
je      .error
mov     rdi, rax
call    SDL_DestroyWindow  ; Direct call
```

**RAII With Custom Deleter:**
```asm
call    SDL_CreateWindow
test    rax, rax
je      .error
mov     rdi, rax
call    SDL_DestroyWindow  ; Identical — inlined!
```

**The Compiler Optimizes Away All The Abstraction!**

---

## Complete Syntax Reference

### Generic Deleter Template

```cpp
template <auto DestroyFn>
struct ResourceDeleter
{
    template <typename T>
    void operator()(T* pointer) const noexcept
    {
        if (pointer)
        {
            DestroyFn(pointer);
        }
    }
};
```

### Creating Type Aliases

```cpp
using TypePtr = std::unique_ptr
<
    ActualType,                    // The C type (e.g., SDL_Window)
    ResourceDeleter<DestroyFunc>   // The destroy function (e.g., SDL_DestroyWindow)
>;
```

### Using The Smart Pointer

```cpp
// Create
TypePtr object_handle{CreateFunction(...)};

// Check validity
if (!object_handle)
{
    // handle error
}

// Access raw pointer
object_handle.get()

// Release ownership (returns raw pointer, won't auto-delete)
ActualType* raw_pointer = object_handle.release();

// Reset to new value
object_handle.reset(new_pointer);

// Automatic cleanup when out of scope — do nothing!
```

---

## Real-World SDL Example

Now Let's See This In A **Complete, Practical SDL Application**:

### 1️⃣ Generic SDL Deleter (Define Once)

```cpp
template <auto DestroyFn>
struct SdlDeleter
{
    template <typename T>
    void operator()(T* pointer) const noexcept
    {
        if (pointer)
        {
            DestroyFn(pointer);
        }
    }
};
```

---

### 2️⃣ Define SDL Smart Handle Types

```cpp
using WindowPtr   = std::unique_ptr<SDL_Window,   SdlDeleter<SDL_DestroyWindow>>;
using RendererPtr = std::unique_ptr<SDL_Renderer, SdlDeleter<SDL_DestroyRenderer>>;
using TexturePtr  = std::unique_ptr<SDL_Texture,  SdlDeleter<SDL_DestroyTexture>>;
```

---

### 3️⃣ Real SDL Usage (No Manual Destroy, No Leaks)

```cpp
#include <SDL.h>
#include <memory>
#include <print>

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::print("SDL_Init error: {}\n", SDL_GetError());
        return 1;
    }

    WindowPtr window_handle
    {
        SDL_CreateWindow
        (
            "RAII SDL",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800, 600,
            SDL_WINDOW_SHOWN
        )
    };

    if (!window_handle)
    {
        std::print("Window error: {}\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    RendererPtr renderer_handle
    {
        SDL_CreateRenderer(window_handle.get(), -1, SDL_RENDERER_ACCELERATED)
    };

    if (!renderer_handle)
    {
        std::print("Renderer error: {}\n", SDL_GetError());
        return 1; // window + SDL auto-cleanup happens
    }

    // Load texture
    SDL_Surface* raw_surface = SDL_LoadBMP("sprite.bmp");
    if (!raw_surface)
    {
        std::print("Surface error: {}\n", SDL_GetError());
        return 1;
    }

    TexturePtr texture_handle
    {
        SDL_CreateTextureFromSurface(renderer_handle.get(), raw_surface)
    };
    SDL_FreeSurface(raw_surface);

    if (!texture_handle)
    {
        std::print("Texture error: {}\n", SDL_GetError());
        return 1;
    }

    // Render loop (simplified)
    SDL_RenderClear(renderer_handle.get());
    SDL_RenderCopy(renderer_handle.get(), texture_handle.get(), nullptr, nullptr);
    SDL_RenderPresent(renderer_handle.get());

    SDL_Delay(2000);

    // NO SDL_DestroyTexture
    // NO SDL_DestroyRenderer
    // NO SDL_DestroyWindow

    SDL_Quit();
    return 0;
}
```

---

### What This Example Proves

#### ✅ Automatic Cleanup (Even On Early Return)

If **any error happens**, all previously created SDL objects are destroyed **correctly and in order**.

- No leaks
- No double-destroy
- No forgotten cleanup

---

#### ✅ Zero Abstraction Overhead

- No virtual functions
- No heap allocation
- No reference counting
- Inlined `SDL_Destroy*` calls
- Same assembly as manual destruction

---

#### ✅ Correct Ownership Semantics

| SDL Object | Owner         |
| ---------- | ------------- |
| Window     | `window_handle`   |
| Renderer   | `renderer_handle` |
| Texture    | `texture_handle`  |

This matches SDL's lifecycle perfectly.

---

### Real-World Scenario Where This *Matters*

#### Without RAII (Bug-Prone)

```cpp
SDL_Window* window = SDL_CreateWindow(...);
SDL_Renderer* renderer = SDL_CreateRenderer(window, ...);

if (!renderer) return; // ❌ window leaked
```

---

#### With RAII (Correct By Default)

```cpp
WindowPtr window_handle{SDL_CreateWindow(...)};
RendererPtr renderer_handle{SDL_CreateRenderer(window_handle.get(), ...)};

if (!renderer_handle) return; // ✅ window destroyed automatically
```

---

## This Technique Is Used In:

- Game engines
- Editors and tools
- Embedded SDL applications
- Professional C++ codebases
- Any C++ project wrapping C libraries (OpenGL, Vulkan, OpenSSL, etc.)

It's the **standard C++ way** to consume C libraries safely.

---

## Key Takeaways

1. **Template parameter `auto`** lets you pass functions as compile-time values
2. **Custom deleters** let `unique_ptr` call any cleanup function
3. **Zero overhead** — the compiler inlines everything
4. **Type aliases** make usage clean and simple
5. **RAII guarantees** resources are cleaned up, even with exceptions or early returns

---

## Quick Reference Card

```cpp
// 1. Define the deleter once
template <auto DestroyFn>
struct ResourceDeleter
{
    template <typename T>
    void operator()(T* pointer) const noexcept
    {
        if (pointer)
        {
            DestroyFn(pointer);
        }
    }
};

// 2. Create type aliases
using TypePtr = std::unique_ptr<Type, ResourceDeleter<destroy_func>>;

// 3. Use it
TypePtr object_handle{create_func(...)};
// Automatic cleanup!
```

---

**You Now Have A Modern, Safe, Zero-Overhead Way To Manage C Resources In C++!** 🎯🚀
