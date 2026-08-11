// Uses of auto keyword in modern c++
#include <iostream>
using namespace std;

// Use of auto in UDF function
auto sum(auto a, auto b)
{
	cout << a + b;
}

int main()
{

	auto a = 45; 		// a is inferred as int
	auto b = 34.23;		// b is inferred as float
	auto c = 'c';		// c is inferred as char
	auto d = 'string';	// d is inferred as string
	auto e = 4344.5235;	// e is inferred as double

	sum(3,4);		// sum function for integers
	sum(5.2,2.5);	// sum function for floats
	sum(54324.2344,2344.3534);	// sum for double

	return 0;
}
/*
auto:

	In modern C++, the auto keyword is used for type inference. It allows the compiler to automatically determine the type of a variable based on the value assigned to it.

	Using auto, you don't need to specify the type of a variable; the compiler automatically determines it for you.

Usage of auto: 

	'auto' Keyword in c++ is used when you actually don't know that which type of data (values) user will be provide

	It is also used as return type in function and parameters

	syntax:

		auto function_name (auto para_1, auto para_2, ... ,para_n)
		{
			// Statement
		}
*/
