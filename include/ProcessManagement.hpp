#pragma once
#include <cstddef>
#include <queue>
#include "Cipher.hpp"
#include "Task.hpp"

namespace fenc {

struct Summary {
    std::size_t succeeded = 0;
    std::size_t failed = 0;
};

// Task queue + process executor. Each task runs in its own forked child,
// with at most `maxWorkers` children alive at once.
class ProcessManagement {
public:
    explicit ProcessManagement(const CaesarCipher& cipher) : cipher_(cipher) {}

    void enqueue(const Task& task) { queue_.push(task); }
    std::size_t pending() const { return queue_.size(); }

    // Drains the queue; blocks until all workers finish.
    Summary run(std::size_t maxWorkers);

private:
    int executeTask(const Task& task) const;  // runs inside the child process

    const CaesarCipher& cipher_;
    std::queue<Task> queue_;
};

}  // namespace fenc
