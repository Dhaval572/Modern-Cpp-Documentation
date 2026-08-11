// Measure speed of optimized cout and printf
#include <iostream>
#include <cstdio>
#include <chrono>

int main()
{
	constexpr int N = 100000;

	// auto start = std::chrono::high_resolution_clock::now();
	// for (int i = 0; i < N; ++i)
	// 	printf("%d\n", i);
	// auto end = std::chrono::high_resolution_clock::now();
	// std::cout << "printf time: "
	// 		  << std::chrono::duration<double>(end - start).count() << "s\n";

	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < N; ++i)
		std::cout << i << '\n';
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "cout time: "
			  << std::chrono::duration<double>(end - start).count() << "s\n";

	return 0;
}
