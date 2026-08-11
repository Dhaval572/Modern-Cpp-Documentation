// Example of operator overloading
#include <iostream>
using namespace std;

class Vector
{
public:
    int x, y;

    Vector(int a, int b) : x(a), y(b){}

    // Overloading of + operator
    Vector operator+(const Vector &other)
    {
        return Vector(x + other.x, y + other.y);
    }

    // Overloading of == operator
    bool operator==(const Vector &other) const
    {
        return (x == other.x) && (y == other.y);
    }
};

class Array
{
    int data[10];

public:
    // Overloading of subscript operator
    int &operator[](int index)
    {
        return data[index];
    }
};

int main()
{
    Vector v1(2, 3), v2(4, 5);
    Vector v3 = v1 + v2;

    cout << "v3:\nx : " << v3.x << "\ny : " << v3.y << '\n';

    if (v1 == v2)
        cout << "v1 and v2 are equal\n";
    else
        cout << "v1 and v2 are not equal\n";

    Vector v4(2, 3), v5(2, 3);

    if (v4 == v5)
        cout << "v4 and v5 are equal\n";
    else
        cout << "v4 and v5 are not equal\n";

    Array arr;
    arr[3] = 44;

    cout << arr[3] << '\n';

    return 0;
}
