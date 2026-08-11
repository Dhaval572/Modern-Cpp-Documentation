#include <iostream>
#include <vector>
#include <algorithm>
#define Sort Sorter
using namespace std;

struct Sorter
{
    vector<int> elements;

    Sorter(initializer_list<int> list) : elements(list)
    {
        sort(elements.begin(), elements.end());
    }

    friend ostream &operator<<(ostream &os, const Sorter &s)
    {
        os << "Sorted { ";

        for (int num : s.elements)
            os << num << " ";

        return os << "}";
    }

    operator vector<int>() const
    {
        return elements;
    }
};

int main()
{
    cout << Sort{43, 234, 4235, 45, 6, 56, 5654, 654, 654} << endl;

    // Add sorted elements in other vector
    vector<int> newSortBlock = Sort{343, 4, 324, 3, 343, 43, 434, 34, 3, 355};

    for (int ele : newSortBlock)
        cout << ele << ' ';

    return 0;
}
