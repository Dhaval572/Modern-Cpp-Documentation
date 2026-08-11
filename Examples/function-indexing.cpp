// Dynamically call different functions based on an index.
#include <iostream>
using namespace std;

void func1() { cout << "Function 1\n"; }
void func2() { cout << "Function 2\n"; }
void func3() { cout << "Function 3\n"; }
void func4() { cout << "Function 4\n"; }
void func5() { cout << "Function 5\n"; }

int main()
{
    void (*funcTable[])() = {func1, func2, func3, func4, func5}; // Array of function pointers

    int index = 4;

    funcTable[index]();

    index = 2;

    funcTable[index]();
    
    cout << "\nAll functions: \n";
    // Call all function which is in functable 
    for(auto func : funcTable)
        func();

    return 0;
}
