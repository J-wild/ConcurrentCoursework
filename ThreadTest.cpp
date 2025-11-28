#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

// Global communication objects
bool status = false;
bool finished = false;
std::mutex m;
std::condition_variable cv;
long long total_time_ns = 0;
const int N = 1000000; // 1 million iterations for fine-grained timing

void receiver() {
    while (!finished){
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, []{ return (status = true); }); // Wait until the num has been updated
        status = false;
    }
}

void sender() {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < N; ++i) {
        std::unique_lock<std::mutex> lock(m);
        status= true;
        lock.unlock(); // Release lock before notifying
        cv.notify_one(); // Signal receiver
    }
    finished = true;

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
    total_time_ns = duration.count();
}

// ... main function to start threads and print results
int main(){

    std::thread t1(sender);
    std::thread t2(receiver);

    t1.join();
    t2.join();

    std::cout << "Time Taken: "<< total_time_ns << " ns" <<std::endl;
    std::cout << "Time Taken per full round: "<< total_time_ns / N << " ns" <<std::endl;
}