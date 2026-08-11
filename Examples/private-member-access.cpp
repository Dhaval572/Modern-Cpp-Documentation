// Access private member directly
#include <iostream>
using namespace std;

class Secret
{
private:

    int privateData = 42;
};

int main()
{
    Secret obj;

    int *ptr = (int *)&obj;
    
    // *ptr = 99; // You can also modify the private members
    cout << "privateData is: " << *ptr << std::endl;
}
