// Track memory allocation easily
#include <iostream>
#include <memory>
using namespace std;

struct AllocationMetrics
{
	uint32_t TotalAllocated = 0;
	uint32_t TotalFreed = 0;

	uint32_t CurrentUsage() { return TotalAllocated - TotalFreed; }
};

static AllocationMetrics s_AllocationMetrics;

void *operator new(size_t size)
{
	s_AllocationMetrics.TotalAllocated += size;
	return malloc(size);
}

void operator delete(void *memory, size_t size)
{
	s_AllocationMetrics.TotalFreed += size;
	free(memory);
}

struct Object
{
	int x, y, z;
};

static void PrintMemoryUsage()
{
	cout << "Memory usage: " << s_AllocationMetrics.CurrentUsage() << " Bytes\n";
}

int main()
{
	PrintMemoryUsage();
	string *str = new string("hello"); // Allocates the 24 bytes

	// This won't show up in memory tracking due to SSO (stored inside the object on the stack)
	// Your operator new is not triggered — no heap allocation happens here.
	// ( Note: Allocates 0 bytes cause of static allocation )
	string str2 = "Hello"; 
	string strArr[3] = {"Cherry", "Apple", "Banana"};

	PrintMemoryUsage();

	{
		unique_ptr<Object> obj = make_unique<Object>(); // Allocates 12
		PrintMemoryUsage();
	}

	PrintMemoryUsage();

	delete str;
	PrintMemoryUsage();

	return 0;
}
