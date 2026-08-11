// Memory allocation in c++
#include <iostream>
#include <memory>
using namespace std;

int main()
{
    // 1). Automatic memory management
    int arr[10];  // Stack memory is automatically managed (memory will be freed when the variable goes out of scope)

    // 2). Dynamic memory allocation

    int* ptr = new int; // Allocation of memory for a single variable
    *ptr = 10;          // Initialize the value of ptr

    delete ptr;         // Deallocate memory manually


    // 3). Smart pointer

    unique_ptr<int> smart_ptr = make_unique<int>(10);  // The number 10 is used to initialize the dynamically allocated memory

    return 0;
}

/*
Explanation:

	C++ is often considered one of the most powerful and popular languages in terms of memory management due to its fine-grained control over memory allocation.

	Here are some methods for memory management in C++:

	1). Automatic memory management

	    The memory is automatically freed when the variable goes out of scope.

	    Example: (Line no. 9)

	2). Dynamic memory allocation

	    You can dynamically allocate memory at runtime using `new` and deallocate it using `delete`.

	    In this method, you must deallocate memory manually using the `delete` keyword.

	    Syntax (for primitive data types):

	        data_type *variable_name = new data_type;
	        (Note: This method is used only for variables, not for containers like arrays)

	    Syntax (for container data structures):

	        data_type *container_name = new data_type[size];

	        Example:

	            int *arr = new int[100];

	        This will provide memory for a 100-size array at runtime.

	    	After allocation, you must deallocate memory in this method.

	    	Syntax for deallocation:

	        	delete variable_name;

	        Example for array:

	            delete[] arr;

	    This method of memory allocation is risky because you must manually deallocate memory.

	3). Smart pointer:

	    A smart pointer takes ownership of a dynamically allocated object and ensures that the object is deleted when the pointer goes out of scope.

	    This method is safer than the second method (dynamic memory allocation).

	    To use this method, you must include the `<memory>` library. (e.g., `#include <memory>`)

	    Syntax:

	        unique_ptr<data_type> variable_name = make_unique<data_type>(value_to_initialize);

	    So, in this method, there is less risk of memory leaks because memory is automatically allocated and deallocated.

	So, this was a short explanation of memory management in C++.



	I hope you understand everything perfectly.
*/
