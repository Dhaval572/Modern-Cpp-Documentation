// Bindinng the function
#include <iostream>
#include <functional>

void print(int a, int b)
{
    std::cout << a << ", " << b << std::endl;
}

int main()
{
    // Swap the order of arguments
    auto swapped = std::bind(print, std::placeholders::_2, std::placeholders::_1);

    swapped(1, 2); 
    return 0;
}
