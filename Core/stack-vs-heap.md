Difference between static allocation and heap allocation
# Difference between Static Allocation and Heap Allocation

| Feature        | Static Allocation                  | Dynamic Allocation                |
|---------------|----------------------------------|---------------------------------|
| **Allocation Time** | Allocated at compile time        | Allocated at runtime             |
| **Memory Location** | Stored in the stack or data segment | Stored in the heap               |
| **Size**        | Fixed and determined at compile time | Dynamic size (can grow or shrink during runtime) |
| **Access Speed** | Faster (due to stack memory management) | Slower (requires heap memory management and pointer dereference) |
| **Lifetime**   | Exists until the variable goes out of scope or the program ends | Exists until manually freed (using `delete` or `free`) |
| **Memory Management** | Automatically managed by the compiler/OS | Must be manually managed by the programmer (using `new`, `malloc`, `delete`, `free`) |
| **Example**    | Local variables, global variables | Dynamically allocated memory (e.g., `new`, `malloc`) |
| **Fragmentation** | No fragmentation                | Can lead to fragmentation if not managed properly |
| **Thread Safety** | Thread-safe for local variables (stack) | Not thread-safe unless synchronized or manually managed |
| **Allocation Limit** | Limited by stack size or data segment | Limited by total available memory (RAM) |
| **Deallocation** | Automatically deallocated when out of scope | Must be manually deallocated using `delete` or `free` |
| **Example Function** | `int x = 5;`                 | `int* ptr = new int; delete ptr;` |
