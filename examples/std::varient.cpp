// Variant in c++ ( No more union )
#include <iostream>
#include <variant>
using namespace std;

// Variant in c++ is replacement of the union
int main()
{

    variant<int, float, char> data;

    data = 3434;
    data = 23.234f;
    data = 'X';
    
    // Checking the current type
    cout << "\nCurrent type index: " << data.index() << endl;
    if (data.index() == 0)
    {
        cout << "It is int: " << get<int>(data) << endl;
    }
    else if (data.index() == 1)
    {
        cout << "It is float: " << get<float>(data) << endl;
    }
    else if (data.index() == 2)
    {
        cout << "It is char: " << get<char>(data) << endl;
    }

    return 0;
}
/*
    +-----------------------------------+
    | Why use variant instead of union? |
    +-----------------------------------+

    1. Variants automatically track the active type
    2. Variants prevent unsafe access with exceptions
    3. Variants provide multiple ways to check types
    4. Unions require manual type tracking (error-prone)
    5. Unions silently fail with undefined behavior
*/
