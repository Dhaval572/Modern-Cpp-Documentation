/**
 * @file multithreading.cpp
 * @brief Demonstrates basic multithreading in C++ using std::thread and std::mutex.
 * 
 * This example shows how to launch threads, pass arguments to thread functions,
 * and use mutexes to prevent data races when accessing shared resources (like std::cout).
 * 
 * @usage Compile with C++11 or later: g++ -pthread multithreading.cpp -o multithreading
 */

#include <iostream>
#include <thread>
#include <mutex>

/**
 * @brief Global mutex to synchronize access to std::cout.
 */
std::mutex cout_mutex;

/**
 * @brief Simple function to be executed by a thread.
 * @param n An integer passed to the thread.
 */
void threadExample(int n)
{
    std::lock_guard<std::mutex> guard(cout_mutex);
    std::cout << "Hello thread! Received number: " << n << "\n";
}

/**
 * @brief Prints a multiplication table for a given number.
 * @param n The base number for the table.
 */
void printTableUsingThread(int n)
{
    std::lock_guard<std::mutex> guard(cout_mutex);
    for (size_t i = 1; i <= 10; i++)
    {
        std::cout << n << " x " << i << " = " << n * i << "\n";
    }
}

int main()
{
    // Launching threads
    std::thread t1(threadExample, 42); 
    std::thread t2(printTableUsingThread, 25);

    // Waiting for threads to finish
    t1.join();
    t2.join();

    return 0;
}
