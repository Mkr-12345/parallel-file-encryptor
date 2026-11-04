#include "ProcessManagement.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>

#include "FileIO.hpp"
#include "Logger.hpp"

namespace fenc {

int ProcessManagement::executeTask(const Task& task) const {
    std::string err;
    const char* verb = (task.mode == Mode::Encrypt) ? "encrypted " : "decrypted ";
    if (!transformFile(task, cipher_, err)) {
        Logger::error(err);
        return 1;
    }
    Logger::info(std::string(verb) + task.input.string() + " -> " + task.output.string());
    return 0;
}

Summary ProcessManagement::run(std::size_t maxWorkers) {
    if (maxWorkers == 0) maxWorkers = 1;
    Summary s;
    std::size_t running = 0;

    while (!queue_.empty() || running > 0) {
        // Spawn workers up to the limit.
        while (!queue_.empty() && running < maxWorkers) {
            Task t = queue_.front();
            queue_.pop();

            pid_t pid = fork();
            if (pid < 0) {
                Logger::error("fork failed for " + t.input.string());
                ++s.failed;
                continue;
            }
            if (pid == 0) {
                int code = executeTask(t);
                _exit(code);  // child never returns into the parent's control flow
            }
            ++running;
        }

        // Reap one finished worker.
        if (running > 0) {
            int status = 0;
            pid_t p = wait(&status);
            if (p < 0) {
                if (errno == EINTR) continue;
                break;  // no children left (ECHILD) - avoid spinning
            }
            --running;
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) ++s.succeeded;
            else ++s.failed;
        }
    }
    return s;
}

}  // namespace fenc
