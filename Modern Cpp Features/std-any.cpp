// Use of any in c++
#include <iostream>
#include <vector>
#include <any>    
#include <string> 
using namespace std;

int main()
{
    // You can store any type of elements in vector using any 
    vector<any> vec;

    vec.push_back(string("hello")); 
    vec.push_back(1344);            
    vec.push_back(344.324f);        
    vec.push_back(true);            

    for (const any &ele : vec)
    {
        if (ele.type() == typeid(string))
        {
            cout << any_cast<string>(ele) << ' ';
        }
        else if (ele.type() == typeid(int))
        {
            cout << any_cast<int>(ele) << ' ';
        }
        else if (ele.type() == typeid(float))
        {
            cout << any_cast<float>(ele) << ' ';
        }
        else if (ele.type() == typeid(bool))
        {
            cout << boolalpha << any_cast<bool>(ele) << ' ';
        }
    }

    return 0;
}
