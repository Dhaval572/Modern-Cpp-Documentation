// Output the true and false instead of 1 and 0
#include <iostream>
using namespace std;

int main()
{
    bool flag1 = true, flag2 = false;
    cout << "Default: \nflag1 = " << flag1 << " \nflag2 = " << flag2 << endl << endl;

    cout << boolalpha;
    cout << "With boolalpha: \nflag1 = " << flag1 << " \nflag2 = " << flag2;

    return 0;
}
