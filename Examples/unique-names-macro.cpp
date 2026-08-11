// Generate Unique Variable Names at Compile-Time
#include <iostream>
using namespace std;

// Macros
#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
#define UNIQUE_ID(name) CONCATENATE(name, __COUNTER__)

int UNIQUE_ID(var) = 10; // var0 = 10;
int UNIQUE_ID(var) = 20; // var1 = 20;
int UNIQUE_ID(var) = 30; // var2 = 30;
int UNIQUE_ID(var) = 40; // var3 = 40;

int main()
{
    cout << "var0: " << var0 << endl;
    cout << "var1: " << var1 << endl;
    cout << "var2: " << var2 << endl;
    cout << "var3: " << var3 << endl;
    
    return 0;
}
