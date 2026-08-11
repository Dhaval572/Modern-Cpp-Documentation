// Constexper in c++
#include <iostream>
#include <array>
using namespace std;

// Recursive constexpr function
constexpr int factorial(int n)
{
	return (n == 0) ? 1 : n * factorial(n - 1);
}

// constexpr function
constexpr int square(int n)
{
	return n * n;
}

// constexpr containers
constexpr std::array<int, 3> getArray()
{
	return {1, 2, 3}; // Evaluated at compile time
}

// Template function that calculates the sum of numbers from 0 to N at compile time
template <int N>
constexpr int sum()
{
	// Recursive case: Add N to the sum of the numbers from 0 to (N-1)
	return N + sum<N - 1>();
}

// This is required to stop the recursion
template <>
constexpr int sum<0>()
{
	return 0;
}

// constexpr constructor
class Point {
public:
    // Define a constexpr constructor
    constexpr Point(int x, int y) : x_(x), y_(y) {}

    // Getters for the coordinates
    constexpr int getX() const { return x_; }
    constexpr int getY() const { return y_; }

private:
    int x_, y_;
};

int main()
{
	constexpr int val = square(5); // Computed at compile time
	cout << "Square of 5 is: " << val << endl;

	constexpr int fact5 = factorial(5); // Computed at compile time
	cout << "Factorial of 5 is: " << fact5 << endl;

	constexpr auto arr = getArray();
	for (auto num : arr)
		cout << num << ' ';

	constexpr int result = sum<10>();  // Compile-time computation
    cout << "Sum from 0 to 10 is: " << result << endl;

	constexpr Point p(10, 20);  
    cout << "Point coordinates: (" << p.getX() << ", " << p.getY() << ")\n";
}

/*
Explanation of constexpr:

	This keyword in c++ is used to define constant expressions, which are expressions that can be evaluated at compile time.

	A function and variable that declared as constexpr indicates to the compiler that it should be evaluated during the compilation process.

Uses:

	1. When Defining Compile-Time Constants:

		Example: constexpr int MAX_BUFFER_SIZE = 1024;

	2. When Using Functions That Can Be Evaluated at Compile Time:

		Example: ( Line no. 12 to 16 )

	3. When Declaring Arrays with Constant Sizes

		Example:

			constexpr size_t SIZE = 100;
			int arr[SIZE];

	4. When Using constexpr Constructors:

		Example: ( Line no. 40 to 51 )

	5. When Using Recursive Functions with Constant Expressions

		Example: ( Line no. 6 to 10 )

	6. When Working with constexpr Containers

		Example: ( Line no. 18 to 22 )

	7. For Efficient Code in Templates and Metaprogramming

		Example: ( Line no 24 to 30 )


Difference between const and constexpr:

	const: 

		The value is fixed at runtime, but not necessarily computed during compilation.

	constexpr:

		The value is known and computed at compile time.


	So, this was simple explanation about constexpr in c++.

	Enjoy advance c++!👩‍💻
*/
