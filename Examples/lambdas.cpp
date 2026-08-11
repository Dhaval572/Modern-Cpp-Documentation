// Lambda expression
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int main()
{

    // Lambda expressions in C++ are used to define anonymous functions—functions without a name—that can be created inline within your code.

    // Example:
    auto add = [](int a, int b)
    {
        return a + b;
    };

    cout << "Sum of 5 and 4 is: " << add(5, 4) << endl;

    /*-----------------------------------------------------------------------------------*/

    // Use 1: Passing Functions as Arguments
    vector<int> vec = {5, 1, 4, 6, 3};

    // Sort in decending order
    sort(vec.begin(), vec.end(), [](int a, int b)
         { return a > b; });

    cout << "Elements are sorted in decending: ";
    for (int ele : vec)
        cout << ele << ' ';

    /*-----------------------------------------------------------------------------------*/

    // Use 2: When defining custom behavior for operations like sorting, filtering, or searching:
    vector<int> nums = {1, 2, 3, 4, 5};

    // Filtering even numbers
    nums.erase(remove_if(nums.begin(), nums.end(),
        [](int num) { return num % 2 != 0; }), nums.end());

    for (int num : nums)
    {
        cout << num << " "; // Outputs: 2 4
    }

    return 0;
}
