// Example of functor
#include <iostream>
#include <functional>
using namespace std;

struct greet
{
	// This is like constructure😅 which can return value 
	void operator()(string name) { cout << "hello, " << name << "!" << endl; }
};

int main()
{
	greet g;
	g("Manthan");
	g("Dhaval");
	g("Xyz");

	return 0;
}
