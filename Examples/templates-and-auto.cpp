// Difference between auto and template
#include <iostream>
using namespace std;

// Template function
template<class T>
T add(T a, T b)
{
    cout << "Template function called: \n";
    return a + b;
}

// function using auto
auto add(auto a, auto b)
{
    cout << "Auto function called: \n";
    return a + b;
}

int main()
{
    cout << add(23.3434,3434.3) << endl;

    cout << add(4545.334,34545); // Explicitly call auto function by using mixed types
    
    return 0;
}
