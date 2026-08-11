// Find total memory which is occupied by program
#include <iostream>
using namespace std;

// Calculates memory of whole program
template <class T, class... Args>
size_t totalMemoryUsage(const T &first, const Args &...rest)
{
    return sizeof(first) + totalMemoryUsage(rest...);
}

// when only one argument is left
template <class T>
size_t totalMemoryUsage(const T &last)
{
    return sizeof(last);
}   

// When 0 arguments 
size_t totalMemoryUsage() { return 0; }

int main()
{
    int a;
    double b;
    char c;
    float d;
    int arr[10];

    struct ExampleStruct
    {
        int x;
        double y;
        char z;
    } myStruct;

    int e;

    size_t totalMemory = totalMemoryUsage(a, b, c, d, e, arr, myStruct);

    cout << "Total memory occupied: " << totalMemory << " bytes" << std::endl;

    return 0;
}
