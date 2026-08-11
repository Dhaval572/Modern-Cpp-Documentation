// Function Pointer
// ( Note: Function pointers is used to call the function dynamically. )
#include <iostream>
using namespace std;

// Function with no arguments
void hello() { cout << "Hello"; }

// Function with argument
int square(int n) { return n * n; }
string greet(string s) { cout << "Hello, " << s; }

int main()
{
    void (*funcWithoutArgs)() = hello;
    funcWithoutArgs();

    int (*funcWithArgs)(int) = square;
    cout << "\nSquare of 4 is: " << funcWithArgs(4) << endl;

    string (*MoreExample)(string) = greet;
    MoreExample("John");

    return 0;
}
