// Variadic in modern c++ ( type safe )
#include <iostream>
using namespace std;

// ( Note: you can also pass the object as argument in this )
template <typename... Args>
void print(Args... args)
{
    ((cout << args << " "), ...);
    cout << '\n';
}

int main()
{
    print(10, 20, 30, 'e', "str", 3434.553f); 
    return 0;
}
