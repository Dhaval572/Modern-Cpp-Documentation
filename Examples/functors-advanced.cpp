// Make the array of functors
#include <iostream>
#include <array>
#include <functional>
using namespace std;

void func1()
{
	cout << "Function 1\n";
}

void func2()
{
	cout << "Function 2\n";
}

void func3()
{
	cout << "Function 3\n";
}

int main()
{
	// Using the c type array

	// function<void()> funcArr[3];
	// funcArr[0] = func1;
	// funcArr[1] = func1;
	// funcArr[2] = func1;

	// funcArr[0]();

	array<function<void()>, 3> funcArr = {func1, func2, func3};

	funcArr.at(0)();
	funcArr.at(1)();
	funcArr.at(2)();

	return 0;
}
