// Solution for calculating love efforts in the context of one-sided love

// Problem Description:

/*
	Someone once said:

	"Any number multiplied by zero is still zero,"
	just like the result of one-sided love efforts.

	So, I came up with a solution for that 🤣
*/
#include <iostream>
using namespace std;

class Love
{
public:
	double value;

	Love(double val)
	{
		value = val;
	}

	// Overload * as +
	Love operator*(const Love &other) const
	{
		return Love(this->value + other.value);
	}
};

// Overload << to print Love objects
ostream &operator<<(ostream &os, const Love &l)
{
	os << l.value;
	return os;
}

int main()
{
	Love efforts(99999.99);
	Love response(0);

	cout << efforts * response << endl;

	// So you can see that efforts never fail,
	// even if the other side gives nothing.

	// Conclusion: Don't stop loving them; love unconditionally.
	// In the end, love always returns a value 😁

	return 0;
}
