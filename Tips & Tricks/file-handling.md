# Modern C++ File Handling: Complete Guide

A comprehensive guide to file operations in modern C++ (C++11 and beyond), featuring best practices, RAII principles, and modern idioms.

## Table of Contents
1. [Basic File Operations](#basic-file-operations)
2. [Modern C++ Approaches](#modern-cpp-approaches)
3. [Reading Files](#reading-files)
4. [Writing Files](#writing-files)
5. [Binary File Operations](#binary-file-operations)
6. [Error Handling](#error-handling)
7. [Performance Tips](#performance-tips)
8. [C++17 Filesystem Library](#cpp17-filesystem-library)

---

## Basic File Operations

### Opening and Closing Files

In modern C++, you rarely need to explicitly close files thanks to RAII (Resource Acquisition Is Initialization). File streams automatically close when they go out of scope.

```cpp
#include <fstream>
#include <iostream>

void basic_example()
{
    std::ofstream file("output.txt");
    
    if (!file)
    {
        std::cerr << "Failed to open file\n";
        return;
    }
    
    file << "Hello, World!\n";
    // File automatically closes when 'file' goes out of scope
}
```

### File Open Modes

```cpp
#include <fstream>

// Text mode (default)
std::ifstream in("input.txt");                    // Read only
std::ofstream out("output.txt");                  // Write only (truncate)
std::ofstream app("output.txt", std::ios::app);   // Append mode
std::fstream io("data.txt", std::ios::in | std::ios::out);  // Read and write

// Binary mode
std::ifstream bin_in("data.bin", std::ios::binary);
std::ofstream bin_out("data.bin", std::ios::binary);

// Additional flags
std::ofstream out_ate("file.txt", std::ios::ate);  // Start at end
std::ofstream out_trunc("file.txt", std::ios::trunc);  // Truncate if exists
```

---

## Modern C++ Approaches

### 1. Use RAII and Smart Resource Management

```cpp
#include <fstream>
#include <string>
#include <memory>

// Good: Automatic resource management
std::string read_file_modern(const std::string& filename)
{
    std::ifstream file(filename);
    
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    return std::string(std::istreambuf_iterator<char>(file),
                      std::istreambuf_iterator<char>());
    // File automatically closes here
}
```

### 2. Use std::filesystem (C++17)

```cpp
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void modern_file_operations()
{
    fs::path filepath = "data/config.txt";
    
    // Check if file exists
    if (!fs::exists(filepath))
    {
        std::cout << "File does not exist\n";
        return;
    }
    
    // Get file size
    std::uintmax_t size = fs::file_size(filepath);
    std::cout << "File size: " << size << " bytes\n";
    
    // Get last write time
    auto ftime = fs::last_write_time(filepath);
    
    // Create directories if needed
    fs::create_directories(filepath.parent_path());
}
```

### 3. Use Structured Bindings and if-init (C++17)

```cpp
#include <fstream>
#include <optional>
#include <string>

std::optional<std::string> read_first_line(const std::string& filename)
{
    if (std::ifstream file(filename); file)  // C++17 if-init
    {
        std::string line;
        if (std::getline(file, line))
        {
            return line;
        }
    }
    return std::nullopt;
}
```

---

## Reading Files

### Reading Entire File into String

```cpp
#include <fstream>
#include <sstream>
#include <string>

// Method 1: Using iterators (most efficient)
std::string read_file_v1(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(file),
                      std::istreambuf_iterator<char>());
}

// Method 2: Using stringstream
std::string read_file_v2(const std::string& filename)
{
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Method 3: Pre-allocate with file size (fastest for large files)
std::string read_file_v3(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::string content(size, '\0');
    file.read(&content[0], size);
    return content;
}
```

### Reading Line by Line

```cpp
#include <fstream>
#include <string>
#include <vector>

// Modern approach with range-based iteration concept
std::vector<std::string> read_lines(const std::string& filename)
{
    std::ifstream file(filename);
    std::vector<std::string> lines;
    
    for (std::string line; std::getline(file, line); )
    {
        lines.push_back(std::move(line));  // Move instead of copy
    }
    
    return lines;
}

// With callback (functional approach)
template<typename Func>
void process_lines(const std::string& filename, Func callback)
{
    std::ifstream file(filename);
    
    for (std::string line; std::getline(file, line); )
    {
        callback(line);
    }
}

// Usage
process_lines("data.txt", [](const std::string& line)
{
    std::cout << line << '\n';
});
```

### Reading with Custom Delimiter

```cpp
#include <fstream>
#include <string>
#include <vector>

std::vector<std::vector<std::string>> read_csv_line(const std::string& filename)
{
    std::ifstream file(filename);
    std::vector<std::vector<std::string>> data;
    
    for (std::string line; std::getline(file, line); )
    {
        std::vector<std::string> row;
        std::stringstream ss(line);
        
        for (std::string cell; std::getline(ss, cell, ','); )
        {
            row.push_back(std::move(cell));
        }
        
        data.push_back(std::move(row));
    }
    
    return data;
}
```

---

## Writing Files

### Basic Writing Operations

```cpp
#include <fstream>
#include <string>
#include <vector>

// Write string to file
void write_string(const std::string& filename, const std::string& content)
{
    std::ofstream file(filename);
    
    if (!file)
    {
        throw std::runtime_error("Cannot write to file");
    }
    
    file << content;
}

// Write vector of lines
void write_lines(const std::string& filename, const std::vector<std::string>& lines)
{
    std::ofstream file(filename);
    
    for (const auto& line : lines)
    {
        file << line << '\n';
    }
}

// Append to file
void append_to_file(const std::string& filename, const std::string& content)
{
    std::ofstream file(filename, std::ios::app);
    file << content;
}
```

### Formatted Writing

```cpp
#include <fstream>
#include <iomanip>

void write_formatted_data()
{
    std::ofstream file("report.txt");
    
    file << std::fixed << std::setprecision(2);
    file << std::setw(15) << "Product" 
         << std::setw(10) << "Price" 
         << std::setw(10) << "Quantity" << '\n';
    
    file << std::setw(15) << "Apple" 
         << std::setw(10) << 1.99 
         << std::setw(10) << 100 << '\n';
}
```

---

## Binary File Operations

### Writing Binary Data

```cpp
#include <fstream>
#include <vector>
#include <cstdint>

// Write POD structure
struct Data
{
    int32_t id;
    double value;
    char name[32];
};

void write_binary(const std::string& filename, const Data& data)
{
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char*>(&data), sizeof(Data));
}

// Write vector as binary
template<typename T>
void write_vector_binary(const std::string& filename, const std::vector<T>& vec)
{
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    
    std::ofstream file(filename, std::ios::binary);
    
    // Write size first
    size_t size = vec.size();
    file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    
    // Write data
    file.write(reinterpret_cast<const char*>(vec.data()), vec.size() * sizeof(T));
}
```

### Reading Binary Data

```cpp
#include <fstream>
#include <vector>

// Read POD structure
Data read_binary(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    Data data;
    file.read(reinterpret_cast<char*>(&data), sizeof(Data));
    return data;
}

// Read vector from binary
template<typename T>
std::vector<T> read_vector_binary(const std::string& filename)
{
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    
    std::ifstream file(filename, std::ios::binary);
    
    // Read size
    size_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    
    // Read data
    std::vector<T> vec(size);
    file.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
    
    return vec;
}
```

---

## Error Handling

### Modern Exception-Based Approach

```cpp
#include <fstream>
#include <stdexcept>
#include <system_error>

class FileError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

std::string safe_read_file(const std::string& filename)
{
    std::ifstream file(filename);
    
    // Enable exceptions on stream
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try
    {
        return std::string(std::istreambuf_iterator<char>(file),
                          std::istreambuf_iterator<char>());
    }
    catch (const std::ios_base::failure& e)
    {
        throw FileError("Failed to read file: " + filename + 
                       " - " + e.what());
    }
}
```

### std::expected (C++23) or std::optional Approach

```cpp
#include <fstream>
#include <optional>
#include <string>

// Using std::optional for simple error handling
std::optional<std::string> try_read_file(const std::string& filename)
{
    std::ifstream file(filename);
    
    if (!file)
    {
        return std::nullopt;
    }
    
    return std::string(std::istreambuf_iterator<char>(file),
                      std::istreambuf_iterator<char>());
}

// Usage
if (auto content = try_read_file("data.txt"); content)
{
    std::cout << "File content: " << *content << '\n';
}
else
{
    std::cerr << "Failed to read file\n";
}
```

---

## Performance Tips

### 1. Use Binary Mode for Large Files

```cpp
// Faster for large files
std::ifstream file("large.dat", std::ios::binary);
```

### 2. Disable Synchronization with C stdio

```cpp
#include <iostream>

int main()
{
    // Speeds up C++ streams significantly
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    
    // Your file operations...
}
```

### 3. Use Buffering Wisely

```cpp
#include <fstream>

void optimized_write()
{
    std::ofstream file("output.txt");
    
    // Set larger buffer for better performance
    char buffer[65536];
    file.rdbuf()->pubsetbuf(buffer, sizeof(buffer));
    
    // Write operations...
}
```

### 4. Memory-Mapped Files for Very Large Files

```cpp
#include <fstream>
#include <vector>

// For extremely large files, consider memory-mapped I/O
// Using platform-specific APIs (mmap on Unix, MapViewOfFile on Windows)
// Or use libraries like Boost.Interprocess
```

### 5. Batch Writes

```cpp
#include <fstream>
#include <sstream>

void batch_write(const std::string& filename)
{
    std::ostringstream buffer;
    
    // Accumulate all writes in memory
    for (int i = 0; i < 1000; ++i)
    {
        buffer << "Line " << i << '\n';
    }
    
    // Single write operation
    std::ofstream file(filename);
    file << buffer.str();
}
```

---

## C++17 Filesystem Library

### Essential Filesystem Operations

```cpp
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

void filesystem_examples()
{
    // Check file existence
    if (fs::exists("config.txt"))
    {
        std::cout << "File exists\n";
    }
    
    // Get file size
    std::uintmax_t size = fs::file_size("data.bin");
    std::cout << "Size: " << size << " bytes\n";
    
    // Create directories
    fs::create_directories("path/to/directory");
    
    // Copy files
    fs::copy_file("source.txt", "dest.txt", 
                  fs::copy_options::overwrite_existing);
    
    // Remove files
    fs::remove("temp.txt");
    
    // Rename/Move files
    fs::rename("old.txt", "new.txt");
    
    // Iterate directory
    for (const auto& entry : fs::directory_iterator("."))
    {
        if (entry.is_regular_file())
        {
            std::cout << entry.path().filename() << '\n';
        }
    }
    
    // Recursive directory iteration
    for (const auto& entry : fs::recursive_directory_iterator("."))
    {
        std::cout << entry.path() << '\n';
    }
    
    // Get file extension
    fs::path p = "document.txt";
    std::cout << "Extension: " << p.extension() << '\n';
    std::cout << "Stem: " << p.stem() << '\n';
    
    // Get absolute path
    fs::path abs_path = fs::absolute("relative/path.txt");
    
    // Get current path
    fs::path current = fs::current_path();
}
```

### Path Manipulation

```cpp
#include <filesystem>

namespace fs = std::filesystem;

void path_operations()
{
    fs::path p = "/home/user/documents/file.txt";
    
    std::cout << "Root: " << p.root_name() << '\n';
    std::cout << "Parent: " << p.parent_path() << '\n';
    std::cout << "Filename: " << p.filename() << '\n';
    std::cout << "Stem: " << p.stem() << '\n';
    std::cout << "Extension: " << p.extension() << '\n';
    
    // Build paths
    fs::path dir = "documents";
    fs::path file = dir / "data" / "config.json";  // Platform-independent
    
    // Replace extension
    fs::path newpath = p;
    newpath.replace_extension(".md");
}
```

---

## Complete Modern Example

```cpp
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class FileManager
{
public:
    static std::string read_all_text(const fs::path& path)
    {
        if (!fs::exists(path))
        {
            throw std::runtime_error("File does not exist: " + path.string());
        }
        
        std::ifstream file(path, std::ios::binary);
        file.exceptions(std::ifstream::badbit);
        
        return std::string(std::istreambuf_iterator<char>(file),
                          std::istreambuf_iterator<char>());
    }
    
    static void write_all_text(const fs::path& path, const std::string& content)
    {
        // Create parent directories if they don't exist
        if (auto parent = path.parent_path(); !parent.empty())
        {
            fs::create_directories(parent);
        }
        
        std::ofstream file(path);
        file.exceptions(std::ofstream::badbit | std::ofstream::failbit);
        file << content;
    }
    
    static std::vector<std::string> read_lines(const fs::path& path)
    {
        std::ifstream file(path);
        std::vector<std::string> lines;
        
        for (std::string line; std::getline(file, line); )
        {
            lines.push_back(std::move(line));
        }
        
        return lines;
    }
    
    static void write_lines(const fs::path& path, 
                           const std::vector<std::string>& lines)
    {
        std::ofstream file(path);
        
        for (const auto& line : lines)
        {
            file << line << '\n';
        }
    }
    
    static FileInfo get_file_info(const fs::path& path)
    {
        struct FileInfo
        {
            std::uintmax_t size;
            fs::file_time_type last_modified;
            bool is_directory;
            bool is_regular_file;
        };
        
        return FileInfo{
            fs::file_size(path),
            fs::last_write_time(path),
            fs::is_directory(path),
            fs::is_regular_file(path)
        };
    }
};

int main()
{
    try
    {
        // Write to file
        FileManager::write_all_text("output/test.txt", "Hello, Modern C++!");
        
        // Read from file
        std::string content = FileManager::read_all_text("output/test.txt");
        std::cout << "Content: " << content << '\n';
        
        // Write lines
        std::vector<std::string> lines = {"Line 1", "Line 2", "Line 3"};
        FileManager::write_lines("output/lines.txt", lines);
        
        // Read lines
        std::vector<std::string> read_lines = FileManager::read_lines("output/lines.txt");
        for (const auto& line : read_lines)
        {
            std::cout << line << '\n';
        }
        
        // Get file info
        if (fs::exists("output/test.txt"))
        {
            FileInfo info = FileManager::get_file_info("output/test.txt");
            std::cout << "File size: " << info.size << " bytes\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    
    return 0;
}
```

---

## Best Practices Summary

1. **Always use RAII** - Let file streams close automatically
2. **Prefer `std::filesystem`** for path operations (C++17+)
3. **Enable exceptions** for critical file operations
4. **Use binary mode** for non-text files and better performance
5. **Pre-allocate buffers** when file size is known
6. **Move instead of copy** when transferring ownership
7. **Use structured bindings** for cleaner code (C++17+)
8. **Check file existence** before operations with `fs::exists()`
9. **Handle errors gracefully** with exceptions or `std::optional`
10. **Disable stdio sync** for performance-critical applications

This modern approach leverages C++11/14/17/20 features for safer, more efficient, and more maintainable file handling code.
