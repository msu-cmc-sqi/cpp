// test6_data_race.cpp
#include <thread>
#include <vector>
#include <iostream>

int counter = 0;

void inc() {
    for (int i = 0; i < 100000; ++i) ++counter;
}

int main() {
    std::thread t1(inc), t2(inc);
    t1.join();
    t2.join();
    std::cout << "counter=" << counter << "\n"; // data race on counter
    return 0;
}

