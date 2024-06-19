#include  <compare>
#include <functional>

struct Task
{
    std::function<void()> func;
    int priority;

    std::strong_ordering operator<=>(const Task& other) const {
        return priority <=> other.priority;
    }
};