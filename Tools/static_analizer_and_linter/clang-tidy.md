# clang-tidy — The Complete Guide

> A practical, no-fluff guide to understanding, installing, and actually using
> `clang-tidy` in a real C++ project — without letting it wreck your codebase.

---

## 1. What is `clang-tidy`?

`clang-tidy` is a **static analysis linter** built on top of the Clang/LLVM
compiler infrastructure. Unlike a regular linter that works on text or regex
patterns, `clang-tidy` works on the **actual parsed representation of your
code** — the same representation the compiler itself uses.

Its official purpose (per the LLVM documentation) is to provide an
extensible framework for diagnosing and fixing typical programming errors —
style violations, API/interface misuse, and bugs that can be found through
static analysis — and it's built to be modular, so new checks can be added
easily.

Think of it as: **"a compiler that also judges your life choices."**

---

## 2. How it works under the hood (explained simply)

### First, what is a "compiler frontend"?

Every compiler has two rough halves:

- **The frontend** — reads your source code, checks it's syntactically and
  semantically valid, and turns it into an internal tree structure called
  an **AST (Abstract Syntax Tree)**. This is the part that *understands*
  your code: it knows that `x` is a variable of type `int`, that this
  function is a member of that class, that this call resolves to that
  overload — real, structural understanding, not text.
- **The backend** — takes that understood structure and turns it into
  actual machine code / assembly for your CPU.

`clang` (the compiler) and `clang-tidy` (the linter) **share the exact same
frontend.** That's the whole trick. `clang-tidy` runs your code through
Clang's real frontend — the same parser, the same type-checker — but
instead of handing the result to the backend to generate machine code, it
hands it to a set of "checks" that inspect the tree and look for problems.

**So when clang-tidy tells you something, it's not guessing from the text
of your code — it's reading the same understanding of your code that the
compiler itself has.**

### What people used before this existed

Before tools like clang-tidy existed, "code quality checking" for C++
mostly meant one of these, each with real limitations:

- **`-Wall -Wextra` compiler warnings** — genuinely useful, but limited to
  what the compiler flags *during a real compile*, and can't be extended
  with your own naming/style rules.
- **Regex/text-based linters (e.g. old-school `cpplint`)** — these scan
  the raw text of your file. They can't actually tell a class member from
  a local variable, or a real function call from a macro that happens to
  look like one — because they never build an AST, they just pattern-match
  text. This causes both false positives and missed bugs.
- **Manual code review** — a human reading the diff and saying "hey, you
  should use `override` here" or "this should be `const`." Effective, but
  slow, inconsistent between reviewers, and doesn't scale.
- **Full static analyzers (like early Coverity/PVS-Studio)** — powerful,
  but historically heavyweight, commercial, and disconnected from your
  actual compiler — meaning their view of your code could drift from what
  your compiler actually did with it.

`clang-tidy` is essentially the answer to: *"what if the linter was
literally part of the compiler, instead of a guess bolted on next to it?"*
Because it reuses Clang's real frontend, it gets **compiler-grade accuracy**
about your code, for free, plus a plugin system for writing arbitrary
rules on top of that understanding.

### The actual mechanics, briefly

1. **It needs to compile your file to understand it.** Since it's a real
   frontend pass, it needs the exact flags your build uses (include paths,
   defines, standard version). This is why it works best with a
   **compile command database** (`compile_commands.json`) — see Section 6.
2. **Checks hook into the AST using "matchers"** — a small query language
   (the same one used by `clang-query`) that lets a check say things like
   "find every place a raw pointer is `new`'d with no matching `delete`."
   A few checks hook earlier, at the preprocessor level (macro/`#include`
   handling), before the AST even exists.
3. **Diagnostics + Fix-its.** When a check matches something bad, it
   reports it the same way a compiler warning is reported, and can attach
   a **fix-it** — a precise, mechanical rewrite instruction. This is what
   `--fix` applies.
4. **It can go even deeper.** The `clang-analyzer-*` checks hand off to the
   full Clang Static Analyzer, which does *path-sensitive* analysis —
   e.g. "on this specific branch, this pointer could be null" — not just
   "this pattern looks risky."

**In one sentence:** clang-tidy is the compiler's own understanding of your
code, repurposed to catch mistakes instead of just generating machine code.

---

## 3. Why bother? (The real benefits)

- **Catches real bugs, not just style nits** — `bugprone-*` checks catch
  things like use-after-move, suspicious string comparisons, and integer
  overflow patterns that compile clean but blow up at runtime.
- **Enforces a consistent codebase** — naming, header includes, brace usage,
  etc. become automatic instead of a matter of code review nagging.
- **Teaches modern C++ as you go** — `modernize-*` checks will nudge you
  toward `auto`, range-based for loops, `override`, smart pointers, etc.
  It's like having a senior engineer whisper "you don't need `new` there"
  in your ear.
- **Machine-applicable fixes** — many diagnostics can be auto-fixed with
  `--fix`, turning a 200-warning cleanup into a five-second command
  (used carefully — see Section 8).
- **Scales with CI** — because it's a CLI tool with parseable output, it
  plugs cleanly into pre-commit hooks and CI pipelines.

---

## 4. Installing clang-tidy

`clang-tidy` ships as part of the `clang-tools-extra` project, usually
packaged as `clang-tools-extra` or `clang-tidy`.

**Fedora:**
```bash
sudo dnf install clang-tools-extra
```

**Debian / Ubuntu:**
```bash
sudo apt install clang-tidy
```

**Arch Linux:**
```bash
sudo pacman -S clang
```
(`clang-tidy` is bundled with the `clang` package on Arch)

**macOS (Homebrew):**
```bash
brew install llvm
```

**Windows:**

If you're developing on Windows, skip everything below and go straight to
**Section 14 — Using clang-tidy in Visual Studio**. Since MSVC is the
official compiler for C++ on Windows, Visual Studio is the recommended way
to use clang-tidy there — it comes bundled, requires no manual install or
PATH setup, and needs no `compile_commands.json` juggling.

(If you specifically want the standalone command-line tool outside Visual
Studio — e.g. for a CI runner — you can still get it via the
[LLVM releases page](https://github.com/llvm/llvm-project/releases)
installer, `winget install LLVM.LLVM`, or `choco install llvm`.)

Verify installation:
```bash
clang-tidy --version
```

If you have multiple LLVM versions installed, you may see binaries like
`clang-tidy-18`. You can point `find_program` (Section 7) or your shell
alias at the specific version if needed.

---

## 5. Exploring what clang-tidy can check: `--dump-config`

Before enabling anything, it's worth *seeing* what clang-tidy currently
thinks it should do. The `--dump-config` flag prints the **fully resolved
configuration** — every check that's enabled/disabled and every check
option — in YAML format, exactly the format used by a `.clang-tidy` file.

```bash
clang-tidy --dump-config
```

This is genuinely one of the most useful things about clang-tidy: you're
never guessing what config is "active." You can always ask the tool
directly.

### Finding a specific setting with `grep`

The config dump can be long, so pipe it through `grep` to find exactly what
you're after. For example, to inspect everything related to naming
conventions:

```bash
clang-tidy --dump-config | grep readability-identifier-naming
```

A few more useful ones:
```bash
# See if a specific check is currently enabled
clang-tidy --dump-config | grep bugprone-use-after-move

# List every check that's currently active
clang-tidy --list-checks

# List active checks matching a pattern (e.g. all modernize checks)
clang-tidy --list-checks | grep modernize

# See every check that exists in your clang-tidy build, enabled or not
clang-tidy --checks='*' --list-checks
```

This `grep` workflow is the fastest way to answer "wait, is this check even
turned on right now?" without reading a giant YAML dump top to bottom.

---

## 6. Running clang-tidy WITHOUT CMake (plain C++)

You don't need a build system to use clang-tidy — it works on a single file
too, as long as you tell it how to compile that file:

```bash
clang-tidy main.cpp -- -std=c++17 -Iinclude -DMY_DEFINE
```

Everything after `--` is passed straight through as compiler flags —
exactly what you'd pass to `g++`/`clang++` (include paths, defines, standard
version).

For a small multi-file project without CMake, you can generate a compile
command database manually, or use a tool like [`bear`](https://github.com/rizsotto/Bear)
to intercept a normal build and generate `compile_commands.json` for you:

```bash
sudo dnf install bear      # Fedora
bear -- g++ -std=c++17 -Wall -Wextra -pthread main.cpp -o app
```

This produces a `compile_commands.json` you can then point clang-tidy at
with `-p` (see Section 9), giving you the same experience as a full CMake
project without actually needing CMake.

---

## 7. Enabling clang-tidy in CMake

CMake has native support for clang-tidy via `CMAKE_CXX_CLANG_TIDY`. When
set, CMake will automatically generate a `compile_commands.json` under the
hood and pass the right flags to clang-tidy for every translation unit.

```cmake
find_program(CLANG_TIDY clang-tidy REQUIRED)
set(CMAKE_CXX_CLANG_TIDY
    ${CLANG_TIDY}
)
```

Also make sure this is set so a compile command database exists, which
clang-tidy (and clangd, and other tooling) relies on:

```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

### ⚠️ Important: this only *runs* checks — it does not "fix" anything

`CMAKE_CXX_CLANG_TIDY` just tells the compiler driver to invoke clang-tidy
alongside every compile step and print its diagnostics. It does **not**
apply `--fix`. That's actually the safe part — the dangerous part is a
different mistake, covered next.

---

## 8. Two things you should NOT do

### ❌ Don't run `--fix` through CMake

It's tempting to think "well if CMake already knows how to invoke
clang-tidy, why not just add `--fix` to `CMAKE_CXX_CLANG_TIDY` and have
everything auto-fix on every build?"

**Don't.** Here's why:

- It runs on **every single compile**, across every translation unit, with
  zero human review in the loop.
- Fix-its from different checks can **interact badly** — one check
  rewrites a line, another check's fix-it now applies to stale source
  positions, and you get subtly broken code or duplicated edits.
- If your `.clang-tidy` config is even slightly too aggressive (e.g. you
  forgot to disable a check you don't actually want), it will now silently
  rewrite huge portions of your codebase on every build, with no diff
  review, no "are you sure," nothing.
- Your compile step becomes a code-mutation step. That's a genuinely
  dangerous property for a build system to have.

**Instead:** always run `--fix` manually, from the command line, on
specific files, after you've reviewed the plain diagnostics first (Section 9).

### ❌ Don't make clang-tidy warnings show up by default on every compile

Wiring `CMAKE_CXX_CLANG_TIDY` in globally means **every developer**, on
**every build**, gets flooded with clang-tidy's opinions mixed in with
actual compiler errors — which makes real compiler errors harder to spot
and makes people start ignoring warnings altogether (the boy-who-cried-wolf
problem).

**Better options:**
- Keep clang-tidy as an **opt-in / separate step** — a Makefile target, a
  CMake custom target, a CI job, or a manual command — not baked into the
  default `cmake --build .` output.
- If you do want it wired into CMake for convenience, gate it behind a
  cache option that's **off by default**:

```cmake
option(ENABLE_CLANG_TIDY "Run clang-tidy during build" OFF)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY clang-tidy REQUIRED)
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY})
endif()
```

Now it only runs when someone explicitly opts in with
`-DENABLE_CLANG_TIDY=ON`.

---

## 9. The recommended workflow: check first, fix second

This is the safe, deliberate loop you should actually use day to day.

**Step 1 — Just look. No fixing yet.**
```bash
clang-tidy -p build main.cpp
```
`-p build` tells clang-tidy where to find `compile_commands.json` (the
"compilation database"), generated by CMake when
`CMAKE_EXPORT_COMPILE_COMMANDS ON` is set. This just prints diagnostics —
your source file is untouched.

**Step 2 — Read the output.** Understand *why* each check is flagging
something. Some warnings are worth ignoring for good reason (see NOLINT
below) — clang-tidy is a tool that informs your judgment, not a replacement
for it.

**Step 3 — Fix, only once you're confident.**
```bash
clang-tidy -p build main.cpp --fix
```
This applies the fix-it hints directly to the file. Do this on a clean git
state, then **actually read the diff** (`git diff`) before committing —
never blind-trust an automated rewrite, even a "safe" one.

**Step 4 (optional) — Scale it up.** Once you trust the workflow on single
files, LLVM ships a helper script, `run-clang-tidy`, that parallelizes
clang-tidy across an entire compile command database:
```bash
run-clang-tidy -p build
```

**Silencing a specific line you've reviewed and deemed correct:**
```cpp
int x = someLegacyApiThatsFine(); // NOLINT(bugprone-something)
```
`NOLINT` suppresses a warning on that exact line; `NOLINTNEXTLINE` applies
to the following line; `NOLINTBEGIN` / `NOLINTEND` wrap a block. All accept
an optional list of specific check names so you don't accidentally silence
everything.

---

## 10. Understanding check categories (with real examples)

Checks are grouped by prefix, and you enable/disable whole groups using
glob patterns in the `Checks:` config (a leading `-` disables). Here's what
each major group actually does, with a real **bad → warning → fixed**
example for each, so it's not just abstract description.

### `bugprone-*` — catches things that are very likely to be actual bugs

```cpp
std::string name = "dhaval";
std::string moved = std::move(name);
std::cout << name;   // <-- using 'name' after it was moved-from
```
```
warning: 'name' used after it was moved [bugprone-use-after-move]
```
This compiles fine and might even *look* fine at runtime, but `name` is in
an unspecified state after the move. This is exactly the kind of bug a
human reviewer can easily miss on a busy diff.

### `performance-*` — catches things the compiler won't warn about, but that waste cycles

```cpp
void PrintAll(std::vector<std::string> items) { // <-- passed by value
    for (const auto &s : items) std::cout << s;
}
```
```
warning: the parameter 'items' is copied for each invocation but only used
as a const reference [performance-unnecessary-value-param]
```
Fix:
```cpp
void PrintAll(const std::vector<std::string> &items) {
    for (const auto &s : items) std::cout << s;
}
```
Every call now avoids copying the whole vector.

### `readability-*` — human-readability and consistency issues

```cpp
bool ok = (is_ready == true);
```
```
warning: implicit conversion 'bool' -> 'bool' ... redundant boolean
comparison [readability-simplify-boolean-expr]
```
Fix:
```cpp
bool ok = is_ready;
```
This category is also where **naming convention enforcement** lives
(`readability-identifier-naming` — see Section 11) — by far the most
practically useful check in this group for keeping a team's style
consistent.

### `modernize-*` — nudges older C++ toward modern, safer idioms

```cpp
int *arr = new int[10];
for (int i = 0; i < 10; i++) {
    arr[i] = i * 2;
}
delete[] arr;
```
```
warning: use range-based for loop instead [modernize-loop-convert]
warning: consider using std::vector<int> instead of a raw new[]-allocated
array [modernize-avoid-c-arrays]
```
Fix:
```cpp
std::vector<int> arr(10);
for (int i = 0; i < static_cast<int>(arr.size()); ++i) {
    arr[i] = i * 2;
}
```
No manual `delete`, no leak risk if an exception is thrown mid-loop.

Another classic one:
```cpp
class Base {
public:
    virtual void Speak();
};
class Derived : public Base {
public:
    void Speak(); // <-- meant to override, but no 'override' keyword
};
```
```
warning: annotate this function with 'override' [modernize-use-override]
```
Fix:
```cpp
void Speak() override;
```
Now if `Base::Speak()`'s signature ever changes, this becomes a **compile
error** instead of a silently-broken override — a real bug caught before
it happens.

### `cppcoreguidelines-*` — enforces rules from the C++ Core Guidelines

```cpp
void Process(int *data, int size) { // <-- owning/non-owning unclear
    ...
}
```
```
warning: consider using std::span<int> instead of a raw pointer + size
[cppcoreguidelines-avoid-c-arrays / bounds-*]
```
This group leans on ownership/lifetime clarity — things like preferring
`std::span`, `std::unique_ptr`, and avoiding raw owning pointers.

### `clang-analyzer-*` — deep, path-sensitive bug finding

```cpp
int *p = nullptr;
if (condition) {
    p = GetPointer();
}
std::cout << *p; // <-- only safe if 'condition' was true
```
```
warning: Dereference of null pointer (loaded from variable 'p')
[clang-analyzer-core.NullDereference]
```
This one is different from the others — it doesn't just pattern-match the
code, it actually **simulates execution paths** through your function to
find the specific branch where this breaks.

### Quick reference table

| Prefix | What it catches |
|---|---|
| `bugprone-*` | Patterns very likely to be real bugs — use-after-move, suspicious `memset`/`sizeof`, dangling references. |
| `performance-*` | Inefficiencies the compiler won't warn about — unnecessary copies, avoidable heap allocations. |
| `readability-*` | Naming, redundant code, overly complex expressions, magic numbers. |
| `modernize-*` | Nudges toward modern idioms — `nullptr`, range-based `for`, `override`, smart pointers. |
| `cppcoreguidelines-*` | C++ Core Guidelines rules — ownership, lifetime, type safety. |
| `cert-*` | CERT C++ Secure Coding Standard — security-focused. |
| `clang-analyzer-*` | Path-sensitive static analysis via the full Clang Static Analyzer. |
| `misc-*` | Checks that don't fit neatly elsewhere. |
| `hicpp-*` | High-Integrity C++ — safety-critical style projects. |
| `google-*`, `llvm-*`, `fuchsia-*`, `abseil-*`, `mpi-*` | Style-guide-specific checks from those codebases. |

A sane starting point for most projects is:
```
bugprone-*, performance-*, readability-*, modernize-*
```
then selectively disable the specific sub-checks that don't fit your
project's style (rather than disabling a whole category).

---

## 11. Enforcing naming conventions automatically

This is one of clang-tidy's most underrated features: `readability-identifier-naming`
doesn't just *flag* naming violations — it can **auto-generate the fix**,
because it knows exactly what identifier it's looking at (a private member?
a struct? a function?) via the AST, not a regex guess.

You configure it per identifier kind, e.g.:

```yaml
CheckOptions:
  readability-identifier-naming.VariableCase: lower_case
  readability-identifier-naming.FunctionCase: CamelCase
  readability-identifier-naming.ClassCase: CamelCase
  readability-identifier-naming.PrivateMemberCase: CamelCase
  readability-identifier-naming.PrivateMemberPrefix: m_
```

Because this hooks into the AST, it correctly distinguishes a private
member variable from a local variable from a function parameter — something
a regex-based linter fundamentally cannot do reliably (it doesn't know
scope or symbol kind). This means you can enforce a *real* team style guide
— not just "please remember to name things correctly," but a tool that
tells you exactly which identifier is wrong and can rewrite it for you with
`--fix`.

---

## 12. Example `.clang-tidy` config (for reference only)

This is provided purely as an illustration of the format — test any config
like this thoroughly against your own codebase before adopting it, since
overly strict settings can generate a huge amount of noise on an existing
project.

```yaml
Checks: >
  bugprone-*,
  performance-*,
  readability-*,
  modernize-*,
  -modernize-use-trailing-return-type,
  -readability-magic-numbers,
  -readability-identifier-length
WarningsAsErrors: ''
HeaderFilterRegex: '.*'
FormatStyle: file
CheckOptions:
  readability-identifier-naming.VariableCase: lower_case
  readability-identifier-naming.FunctionCase: CamelCase
  readability-identifier-naming.ClassCase: CamelCase
  readability-identifier-naming.StructCase: CamelCase
  readability-identifier-naming.StructPrefix: t_
  readability-identifier-naming.PublicMemberCase: lower_case
  readability-identifier-naming.PublicMemberPrefix: ''

  readability-identifier-naming.PrivateMemberCase: CamelCase
  readability-identifier-naming.PrivateMemberPrefix: m_
```

Place this file at your project root as `.clang-tidy` — clang-tidy
automatically discovers it by walking up from the source file's directory,
and `--dump-config` will show you the resolved result once it's picked up.

`WarningsAsErrors: ''` means nothing is escalated to an error by default —
worth leaving empty until your config is stable, then tightening it later
per-check once you trust it.

---

## 13. A few extra tips worth knowing

- **Editor integration is basically free.** `clangd` (the language server
  many editors/IDEs use for C++) reads the same `.clang-tidy` file and
  shows the same diagnostics live, inline, as you type — no separate CLI
  run needed for day-to-day feedback.
- **Baseline an existing project before enabling checks.** On a large
  existing codebase, turning on `cppcoreguidelines-*` or `modernize-*` cold
  can produce thousands of warnings. Start narrow (`bugprone-*` only),
  clean that up, then expand outward.
- **Use it incrementally in CI on diffs, not the whole repo.** LLVM ships
  `clang-tidy-diff.py`, which runs clang-tidy only on the lines changed in
  a patch/PR — much more practical than re-linting an entire legacy
  codebase on every commit.
- **`-checks='*'` shows you everything that exists**, including checks
  that are off by default (some are experimental or opinionated) — good
  for discovery, not for blind enabling.
- **Version drift is real.** Checks get added, renamed, and occasionally
  removed between LLVM releases. Pin your `clang-tidy` version in CI the
  same way you'd pin a compiler version.

---

## 14. Using clang-tidy in Visual Studio (recommended for Windows)

If you're on Windows and using MSVC (the official, native C++ compiler for
Windows), the right move is to **not** touch the command line at all —
Visual Studio has clang-tidy built in, running natively alongside the
MSVC toolset. No PATH setup, no manually installing LLVM, no
`compile_commands.json` wrangling.

### Setup — one time, no manual config

1. Open the **Visual Studio Installer** → **Modify** your VS install.
2. Make sure the **"Desktop development with C++"** workload is checked.
   `clang-tidy` is included automatically with it (VS 2019 16.4+).

That's it. It works with your normal MSVC toolset — you do **not** need
to switch to the `clang-cl` toolset to get clang-tidy.

### Enabling it for a project

**Regular (MSBuild) project:**
1. Right-click the project in **Solution Explorer** → **Properties**.
2. Go to **Configuration Properties → Code Analysis → General**.
3. Under the tool selection, make sure **Clang-Tidy** is checked as one of
   the analyzers to run (you can run it alongside or instead of Microsoft's
   own Code Analysis).

**CMake project opened directly in Visual Studio:**
Add this to your `CMakeSettings.json` or `CMakePresets.json`:
```json
{
  "enableClangTidyCodeAnalysis": true,
  "clangTidyChecks": "bugprone-*,performance-*,readability-*,modernize-*"
}
```
Or — same as the command-line workflow — just drop a `.clang-tidy` file at
your project root and Visual Studio will pick it up automatically,
exactly like the CLI tool does.

### Configuring which checks run

Instead of hand-typing checks in the Property Pages every time, point
Visual Studio at the same `.clang-tidy` file described in Section 12 —
this keeps your Windows setup and any Linux/Mac teammates' CLI setup
perfectly in sync, one config, everywhere. If you do want to override
checks just for the IDE, that field is at
**Configuration Properties → Code Analysis → Clang-Tidy → Clang-Tidy
Checks**.

### Running it

- It runs **automatically in the background** as you type/edit, showing
  live squiggles in the editor — same idea as a compiler error underline.
- To run it on demand for the whole project: **Analyze menu → Run Code
  Analysis → On \<Your Project\>**.
- Clang-Tidy runs *after* a successful compile — if you have build errors,
  fix those first to get Clang-Tidy results.

### Reviewing and applying fixes — no command line needed

- All findings show up in the **Error List** window, with a **Category**
  column so you can sort/group by check (`modernize-use-override`,
  `bugprone-use-after-move`, etc.) instead of scrolling one-by-one.
- Double-click any entry to jump straight to that line in the editor.
- For an individual warning, click the **lightbulb / screwdriver quick-fix
  icon** that appears next to the squiggle in the editor to apply that
  specific fix inline — this is the IDE equivalent of running
  `clang-tidy --fix` on just that one diagnostic, with the same "look
  before you leap" safety: you see and apply each fix individually rather
  than mass-rewriting the file blind.

This gives you the exact same safe workflow recommended in Section 9
(check first, fix deliberately) — just point-and-click instead of typing
commands, which is the natural way to work when MSVC is already your
day-to-day compiler.

---



```bash
# Install (Fedora)
sudo dnf install clang-tools-extra

# See fully resolved config
clang-tidy --dump-config

# Search the config for one setting
clang-tidy --dump-config | grep readability-identifier-naming

# List currently active checks
clang-tidy --list-checks

# Run on a single file without any build system
clang-tidy main.cpp -- -std=c++17 -Iinclude

# Run using a compile command database (from CMake), check only
clang-tidy -p build main.cpp

# Apply fixes after reviewing the above
clang-tidy -p build main.cpp --fix

# Run across the whole project in parallel
run-clang-tidy -p build
```
