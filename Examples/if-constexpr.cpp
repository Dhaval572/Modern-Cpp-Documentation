// Compile time if else
#include <iostream>
using namespace std;

int main()
{
    #ifdef _WIN32
        #define PLATFORM "Windows"
    #elif __linux__
        #define PLATFORM "Linux"
    #elif __APPLE__
        #define PLATFORM "MacOS"
    #else
        #define PLATFORM "Unknown OS"
    #endif

    cout << "Running on " << PLATFORM;
    return 0;
}
/*
Explanation of #ifdef, #else, #elif, #endif

#ifdef:

    Checks the conditions like if keyword.

#else:

    Default condition like else keyword.

#elif:

    Checks the next contition like else if.

#endif:

    Used to end the #ifdef.
*/
