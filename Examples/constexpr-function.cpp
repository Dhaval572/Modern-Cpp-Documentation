// Compile-Time Function Optimization Using Macros in C++ 
#include <iostream>
#define MAX(a, b) ((a) > (b) ? (a):(b))
#define ADD(a, b) ((a) + (b))
#define SQUARE(x) ((x) * (x))
using namespace std;

int main()
{
    cout << "Max from 35 and 67 is: " << MAX(35,67) << endl;
    cout << "Square of 5 is: " << SQUARE(5) << endl;
    cout << "Addition of 3 and 5 is: " << ADD(3, 5);
    
    return 0;
}
