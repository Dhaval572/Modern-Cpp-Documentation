// Find the version of c++ language
#include <iostream>
using namespace std;

int main()
{
    constexpr long cpp_ver = __cplusplus;

    if (cpp_ver == 199711L)
        cout << "C++98/03\n";
    else if (cpp_ver == 201103L)
        cout << "C++11\n";
    else if (cpp_ver == 201402L)
        cout << "C++14\n";
    else if (cpp_ver == 201703L)
        cout << "C++17\n";
    else if (cpp_ver == 202002L)
        cout << "C++20\n";
    else if (cpp_ver == 202302L)
        cout << "C++23\n";
    else if (cpp_ver >= 202002L)
        cout << "Post-C++23 (Future Standard)\n";
    else
        cout << "Pre-standard/Unknown\n";

    return 0;
}
