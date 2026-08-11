/**
 * @file unique-pointer.cpp
 * @brief Demonstrates the usage of std::unique_ptr for exclusive ownership.
 * 
 * This example shows how to create a unique_ptr using std::make_unique,
 * how to access the underlying object, and how to transfer ownership using std::move.
 */

#include <iostream>
#include <memory>

/**
 * @class MyClass
 * @brief A simple class to demonstrate object lifecycle.
 */
class MyClass
{
public:
    /**
     * @brief Constructor that prints a message when called.
     * @param x Initial value.
     */
    MyClass(int x) : val(x) 
    { 
        std::cout << "Constructed with value " << val << "\n"; 
    }

    /**
     * @brief Destructor that prints a message to verify memory cleanup.
     */
    ~MyClass() 
    { 
        std::cout << "Destructed\n"; 
    }

    /**
     * @brief Displays the current value.
     */
    void show() 
    { 
        std::cout << "Value: " << val << "\n"; 
    }

private:
    int val;
};

int main()
{
    // Creating a unique_ptr with std::make_unique (preferred)
    std::unique_ptr<MyClass> ptr = std::make_unique<MyClass>(42);
    ptr->show();

    // Transferring ownership from ptr to ptr2 using std::move
    std::unique_ptr<MyClass> ptr2 = std::move(ptr);

    if (!ptr)
    {
        std::cout << "ptr is now empty.\n";
    }

    ptr2->show();

    // Memory is automatically freed when ptr2 goes out of scope
    return 0;
}
