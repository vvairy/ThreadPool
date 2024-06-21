#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <vector>
#include <cmath>
#include "ThreadPool.hpp"

class Results
{
    std::mutex primeDividorsMutex;
    std::map<uint64_t, std::vector<uint64_t>> primeDividors;
public:
    auto getDividors() const -> const std::map<uint64_t, std::vector<uint64_t>>&
    {
        return primeDividors;
    }

    void insert(std::pair<uint64_t, std::vector<uint64_t>>&& divs) {
        std::lock_guard<std::mutex> lock(primeDividorsMutex);
        primeDividors.insert(std::move(divs));
    }
} res;


static std::vector<uint64_t> calculateDividors(uint64_t num)
{
    std::vector<uint64_t> dividors;

    while (num % 2 == 0) {
        dividors.push_back(2);
        num /= 2;
    }
    while (num % 3 == 0) {
        dividors.push_back(3);
        num /= 3;
    }

    for (uint64_t i = 5; i < sqrt(num); i += 6)
    {
        while (num % i == 0) {
            dividors.push_back(i);
            num /= i;
        }
        while (num % (i + 2) == 0) {
            dividors.push_back(i + 2);
            num /= (i + 2);
        }
    }
    dividors.push_back(num);
    return dividors;
}

int main() {
    ThreadPool pool(2);

    std::string input;
    uint64_t number;
    int priority;

    while (true)
    {
        std::getline(std::cin, input);
        if (input == "exit")
            break;
        if (input == "restart")
        {
            pool.resize(pool.size() * 2);
            std::cout << "restarted with new size: " << pool.size() << '\n';
        }
        else if (std::istringstream iss(input); iss >> number >> priority)
        {
            pool.enqueueTask(Task{ [number] {
               res.insert(std::make_pair(number, calculateDividors(number)));
            }, priority });
        }
        else
            std::cout << "ERR\n";
    }

    //print results
    for (const auto& [num, dividors] : res.getDividors())
    {
        std::cout << num << ": ";
        for (const auto& el : dividors)
            std::cout << el << ' ';
        std::cout << '\n';
    }
    
    return 0;
}
