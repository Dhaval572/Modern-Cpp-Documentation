// Timer in c++
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

struct Timer
{
	chrono::time_point<chrono::steady_clock> start, end;
	chrono::duration<float> duration;

	Timer()
	{
		start = chrono::steady_clock::now();
	}

	~Timer()
	{
		end = chrono::steady_clock::now();
		duration = end - start;

		float ms = duration.count() * 1000.0f;
		cout << "Timer took " << ms << " ms\n";
	}
};

void function()
{
	Timer timer;
	for (int i = 0; i < 9999; ++i)
	{
		cout << "Hello" << endl;
	}
}

int main()
{
	function();
	return 0;
}
