// About methods of limits lib
#include <iostream>
#include <limits>
using namespace std;

int main()
{
	// Find the range of data type
	cout << "Range of int is: " << numeric_limits<int>::min() << " to " << numeric_limits<int>::max() << endl;

	cout << "Range of bool is: " << numeric_limits<bool>::min() << " to " << numeric_limits<bool>::max() << endl;

	cout << "Range of unsigned int is: " << numeric_limits<unsigned int>::min() << " to " << numeric_limits<unsigned int>::max() << endl;
	
	auto i = 43223213.23;

	cout << "Is (i) is integer? " << numeric_limits<decltype(i)> :: is_integer << endl; // Will return 0 means false

	auto j = 'h';
	cout << "Is (j) is signed? " << numeric_limits<decltype(j)> :: is_signed << endl; // Will return 1 means true

	return 0;
}
/*
About limits:

	The <limits> header in C++ provides a set of constants and functions that are useful for querying the properties of numeric types (such as int, float, double etc.)

	There are many functions in the limits lib.

	1).	min():

		This function returns the minimum range to store value:

		If data type is unsigned than it will return 0; ( Line no. 13 )

		Syntax: 

			numeric_limits<data_type>::min();

	2).	max():

		This function returns the maximum range to store value:

		For integer types, this is the largest positive value, and for floating-point types, it is the largest representable value.

		Syntax:

			numeric_limits<data_type>::max();

	3). is_integer():

		This function checks that given data type or variable is integer or not. If yes then it will return 1 else it will returns 0

		Syntax for data type: 

			numeric_limits<data_type> :: is_integer;

		Syntax for variable:

			numeric_limits<decltype(variable_name)> :: is_integer;

	
	So, it was some methods of limits lib.

	I hope you understand everything.
*/
