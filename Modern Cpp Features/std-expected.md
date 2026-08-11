# Beyond Exceptions: A Complete Guide to `std::expected<T, E>` in C++23

## 1. The Problem Before `std::expected`

For decades, C++ had exactly two mainstream ways to report "this function might fail":

### Option A — Error codes / return flags
```cpp
int Divide(int a, int b, bool* out_success)
{
    if (b == 0)
    {
        *out_success = false;
        return 0;
    }
    *out_success = true;
    return a / b;
}
```
Problems with this:
- The **real return value** and the **error signal** are two separate, disconnected things. Nothing stops you from ignoring `out_success` and using a garbage `int`.
- No compiler warning if you forget to check the flag.
- Doesn't compose. Chaining five functions that each need an out-parameter is painful and ugly.
- The error itself is often just `bool` or an `int` errno — no rich type, no message, no context.

### Option B — Exceptions
```cpp
int Divide(int a, int b)
{
    if (b == 0)
    {
        throw std::runtime_error("divide by zero");
    }
    return a / b;
}
```
Problems with this:
- **Hidden control flow.** Looking at a function signature tells you nothing about whether it can fail — you have to read the implementation (or docs) to know it throws.
- **Stack unwinding cost.** When a `throw` actually happens, the runtime has to walk back up the call stack, running destructors and consulting unwind tables, frame by frame. This is *slow* — often 100x–1000x slower than a normal return.
- **Binary size / cold-path cost.** Even if an exception is never thrown, the compiler still has to emit unwind tables (`.eh_frame` on Linux) and personality routines for every function in the call chain, bloating the binary.
- **Not usable everywhere.** Kernel code, embedded/real-time systems, some game engines, and codebases compiled with `-fno-exceptions` simply cannot use exceptions.
- **try/catch blocks are easy to misuse** — catching too broadly, swallowing errors silently, or catching by value and slicing.

So the real problem: **neither approach forces you, at compile time, to acknowledge that a function can fail.**

---

## 2. What `std::expected<T, E>` Actually Is

`std::expected<T, E>` (added in **C++23**, header `<expected>`) is a **tagged union** — it holds *either*:
- a value of type `T` (success), **or**
- a value of type `E` (the error), **but never both.**

It is essentially `std::variant<T, E>` with an API purpose-built for error handling — closer in spirit to Rust's `Result<T, E>`.

```cpp
#include <expected>

enum class e_ErrorCode
{
    None,
    DivideByZero,
    Overflow
};

std::expected<int, e_ErrorCode> Divide(int a, int b)
{
    if (b == 0)
    {
        return std::unexpected(e_ErrorCode::DivideByZero);
    }
    return a / b;
}
```

Reading the signature alone, you now **know** three things instantly:
1. `Divide` returns an `int` on success.
2. `Divide` can fail, and the failure is described by `e_ErrorCode`.
3. There is no hidden `throw` — everything is visible in the type.

### Naming breakdown (matching your conventions)
| Piece | Meaning | Your style |
|---|---|---|
| `T` | The "happy path" return type | e.g. `int`, `t_ParsedFile` |
| `E` | The error/"flag" type | e.g. `e_ErrorCode` |
| `std::expected<T, E>` | Return type of the function | one type, not a value + an out-param |
| `std::unexpected(err)` | Wraps an `E` to construct the failure case | used only on the error path |

---

## 3. Using the Result

```cpp
t_Result CallSite()
{
    std::expected<int, e_ErrorCode> result = Divide(10, 0);

    if (!result.has_value())
    {
        e_ErrorCode err = result.error();
        // handle err — no unwinding, no throw, just a normal branch
        return t_Result{ .success = false };
    }

    int value = result.value(); // or result.value() / *result
    return t_Result{ .success = true, .data = value };
}
```

Key API pieces:
- `result.has_value()` / `static_cast<bool>(result)` — check success.
- `result.value()` — get `T` (throws `std::bad_expected_access` only if you call it on an error — you opted into that risk explicitly).
- `*result` / `result->member` — same as `.value()` but no bounds-checking overhead, like `optional`.
- `result.error()` — get `E`.
- `result.value_or(fallback)` — get `T` or a default if it failed.
- `[[nodiscard]]` is baked in — **the compiler warns you if you call a function returning `std::expected` and throw away the result without checking it.** This is the single biggest practical win over error codes.

### Composability — the part error codes can't do
```cpp
std::expected<int, e_ErrorCode> ParseAndDivide(std::string_view text, int divisor)
{
    return ParseNumber(text)              // std::expected<int, e_ErrorCode>
        .and_then([&](int n)
        {
            return Divide(n, divisor);    // chains only if ParseNumber succeeded
        });
}
```
`.and_then()`, `.or_else()`, `.transform()`, `.transform_error()` let you chain fallible operations without a single `if` statement or a single `throw`. If any step fails, the error just flows through — like a pipeline.

---

## 4. Why It's Better Than Exceptions (and Better Than Raw Error Codes)

| Aspect | Error codes / out-params | Exceptions | `std::expected<T, E>` |
|---|---|---|---|
| Failure visible in signature? | Sometimes (if documented) | **No — hidden** | **Yes — part of the type** |
| Compiler forces you to check? | No | No | **Yes (`[[nodiscard]]`)** |
| Cost when nothing fails | Free | Free (in theory) but bloats binary with unwind tables | **Free — just a tagged struct return** |
| Cost when something fails | Free | **Expensive — stack unwinding** | **Free — normal return, no unwinding** |
| Works with `-fno-exceptions` / kernel / embedded? | Yes | No | **Yes** |
| Rich error type? | Usually just an `int`/`bool` | Yes (exception object) | **Yes — any type `E`** |
| Composable / chainable? | No | Sort of (try/catch nesting) | **Yes (`and_then`, `or_else`, ...)** |
| Control flow | Explicit, but ad hoc | Implicit, non-local jumps | **Explicit, local, ordinary branches** |

### The core reason it "reduces overhead"
Exceptions are often called "zero-cost" — and on the **success path**, that's roughly true; there's no per-call check. But that framing hides two real costs:
1. **Binary/metadata cost** — unwind tables and landing pads exist for *every function in the throw path*, whether or not a throw ever happens. This is pure overhead sitting in your binary and instruction cache.
2. **Failure-path cost** — when a `throw` *does* fire, unwinding the stack (running destructors frame-by-frame, consulting `.eh_frame`, invoking personality routines) is dramatically more expensive than a function returning a struct. In hot error paths (e.g., parsing untrusted input, validating many small requests), this can matter a lot.

`std::expected` sidesteps both: there are no unwind tables to generate for this path, and a "failure" is exactly as cheap as a normal return — because mechanically, it **is** a normal return.

---

## 5. Memory Overhead of `std::expected<T, E>`

This is the tradeoff you're paying for all of the above. `std::expected<T, E>` is implemented (conceptually) like:

```cpp
template <typename T, typename E>
class t_Expected
{
    bool has_value;   // the discriminant flag
    union
    {
        T value;
        E error;
    };
};
```

So the size is roughly:

```
sizeof(std::expected<T, E>) ≈ max(sizeof(T), sizeof(E)) + alignment padding for the discriminant
```

Concretely:
- `std::expected<int, e_ErrorCode>` where both are 4 bytes → typically **8 bytes** (4 bytes for the union + up to 4 bytes padding for the bool tag, due to alignment rules), vs. a bare `int` at 4 bytes. That's **+100% for this specific tiny case** — but 8 bytes is still nothing.
- For larger `T` (say a 64-byte struct) with a small `E` (an enum), the overhead is usually just **1 byte + padding to the natural alignment of `T`** — often rounds up to 8 bytes on a 64-bit system. So overhead shrinks toward **negligible** as `T` gets bigger.
- Compare directly to **exceptions**, which add **zero size to the return value itself**, but instead pay with:
  - Extra `.eh_frame` / unwind-table bytes in the binary (a fixed, per-function cost, invisible in `sizeof()` but real in binary size and I-cache pressure).
  - A much larger, allocation-based cost *at throw time* (the exception object itself is usually heap-allocated by the runtime).

So the honest comparison:
- **`std::expected`**: small, fixed, predictable extra bytes on the stack per call, paid every single call (success or failure) — but no runtime allocation, no unwinding, ever.
- **Exceptions**: ~0 extra stack bytes per call, but binary-wide static overhead plus a large, unpredictable cost concentrated entirely on the failure path.

If failures are rare and cheap-to-ignore, exceptions can look attractive. If failures are common, need to be composed, or need to run in an exception-free environment, `std::expected` wins on both correctness (compiler-enforced checking) and predictable performance.

---

## 6. Quick Mental Model

```
Expected<T, E> FuncName()
                │
                ├── T  → the value you get on success (the "happy path" return type)
                └── E  → the value you get on failure (the "error flag" / error type)
```

Think of it as: **"a return type that's honest about the fact it might not have a value."**

---

Happy coding! 🚀
