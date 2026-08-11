// Using namepace is risky
#include <iostream>
using namespace std;

namespace ns1
{
    void print(const string& str)
    {
        cout << str << '\n';
    }
}

namespace ns2
{
    void print(const char* str)
    {
        string temp = str;
        reverse(temp.begin(), temp.end());
        cout << temp << '\n';
    }
}

// using namespace ns2;
// using namespace ns1;

int main()
{   
    // print("Dhaval"); // This will implicitly call the ns2 string.

    // But if you don't use namespace than you can call print which you want. like below:

    ns1::print("Hello");
    ns2::print("Hello");

    // So, don't use namespace in header. use only and only in particualar function which can be safe

    return 0;
}
