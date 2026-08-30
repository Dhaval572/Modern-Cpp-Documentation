# Advanced C++ Documentation

Welcome to the Advanced C++ Documentation repository. This project serves as a comprehensive guide to modern C++ features (C++11 through C++23), best practices, and advanced programming concepts.

## 📂 Project Structure

The repository is organized into logical directories to help you find information quickly:

- **[`core/`](./core)**: Fundamental C++ concepts, memory management rules, RAII, and pointers.
- **[`features/`](./features)**: Deep dives into features introduced in C++14, C++17, C++20, and C++23.
- **[`examples/`](./examples)**: Ready-to-run C++ code snippets demonstrating specific techniques like multithreading, smart pointers, and design patterns.
- **[`C++ Philosophy/`](./C++%20Philosophy)**: High-level guides, mastering roadmaps, and conceptual explanations.
- **[`tips/`](./Tips%20&%20Tricks)**: Performance optimization tricks, loops, string handling, and memory safety advice.
- **[`Tools/`](./Tools)**: Practical guides for modern C++ development tools, including debuggers, testing frameworks, static analyzers, sanitizers, profilers, documentation generators, build systems, and package managers. This section focuses on modern, industry-standard tools with practical workflows, best practices, and references to their official documentation.

## 🚀 Getting Started

### Prerequisites

- A modern C++ compiler (GCC 10+, Clang 10+, or MSVC 2019+ recommended for full feature support).
- Basic knowledge of C++ syntax and object-oriented programming.

### Running Examples

Most examples are self-contained and can be compiled using a single command:

```bash
# Example: Compiling the multithreading demo
g++ -pthread examples/multithreading.cpp -o multithreading
./multithreading
```

## 🛠 Contribution Guidelines

Contributions are welcome! If you'd like to add new documentation or examples:

1.  **Follow the Style Guide**: Adhere to the established directory structure and naming conventions.
2.  **Document Your Code**: Ensure every file has a descriptive header and all functions are commented.
3.  **Use Modern C++**: Prefer modern alternatives (e.g., `std::unique_ptr` over raw `new`/`delete`, Note: Don't use `shared_ptr` blindly).
4.  **Submit a PR**: Create a new branch and submit a pull request with a clear description of your changes.

## 📚 Key Resources

- [Cppreference.com](https://en.cppreference.com/) - The definitive C++ standard library reference.
- [ISO C++ FAQ](https://isocpp.org/faq) - Frequently asked questions about C++.
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) - Best practices from the creators of C++.

---

*Happy learning! 😄💙*
