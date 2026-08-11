// Check that given is a function or not
#include <iostream>
using namespace std;

void func() {}
double calc() {}

int main()
{
    cout << boolalpha; // Converts the answer in true and false
    cout << "Is func is function? : " << is_function<decltype(func)>::value << endl;
    cout << "Is calc is function? : " << is_function<decltype(calc)>::value << endl;

    int addition;
    cout << "Is addition is function? " << is_function<decltype(addition)>::value << endl;

    return 0;
}
