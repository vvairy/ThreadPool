#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <vector>
#include <cmath>
#include "ThreadPool.hpp"

struct Results
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

bool static isPrime(uint64_t num)
{
    if (num <= 1) return false;
    if (num == 2 || num == 3) return true;
    if (num % 2 == 0 || num % 3 == 0) return false;
    for (uint64_t i = 5; i <= sqrt(num); i += 6)
        if (num % i == 0 || num % (i + 2) == 0) return false;
    return true;
}

int main() {
    ThreadPool pool(2);

    std::string input;
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
        else 
        {
            std::istringstream iss(input);
            uint64_t number; int priority;
            if (!(iss >> number >> priority))
            {
                std::cout << "ERR\n";
                continue;
            }
            else
            {
                pool.enqueueTask(Task{ [number]
                {
                    uint64_t num = number;
                    std::vector<uint64_t> dividors;
                    for (uint64_t i = 2; i <= sqrt(num);)
                    {
                        if (num % i == 0 && isPrime(i))
                        {
                            dividors.push_back(i);
                            num /= i;
                        }
                        else { ++i; }
                    }
                    dividors.push_back(num);
                    res.insert(std::make_pair(number, dividors));
                }, priority });
            }
        }
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
