/*
 * LINUX SYSTEM LEVEL C++ - SIGNALS AND FORK
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra signals_and_fork.cpp -o signals_fork
 */

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// Global variables for signal handling
volatile sig_atomic_t signal_count = 0;
volatile sig_atomic_t last_signal = 0;

// =============================================================================
// 1. BASIC SIGNAL HANDLING
// =============================================================================

void basic_signal_handler(int sig) {
    signal_count++;
    last_signal = sig;
    
    // Only async-signal-safe functions should be called here
    const char* msg = "Signal received\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

void demonstrate_basic_signals() {
    SECTION("Basic Signal Handling");
    
    // Install signal handlers
    signal(SIGINT, basic_signal_handler);
    signal(SIGTERM, basic_signal_handler);
    signal(SIGUSR1, basic_signal_handler);
    
    std::cout << "Process PID: " << getpid() << std::endl;
    std::cout << "Send SIGUSR1 to this process: kill -USR1 " << getpid() << std::endl;
    std::cout << "Waiting for signals (10 seconds)..." << std::endl;
    
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(3)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (signal_count > 0) {
            std::cout << "Received " << signal_count << " signals, last was " << last_signal << std::endl;
        }
    }
    
    std::cout << "Signal demonstration complete" << std::endl;
}

// =============================================================================
// 2. ADVANCED SIGNAL HANDLING WITH SIGACTION
// =============================================================================

void advanced_signal_handler(int sig, siginfo_t* info, void* context) {
    (void)context; // Suppress unused parameter warning
    
    const char* sig_name;
    switch (sig) {
        case SIGUSR1: sig_name = "SIGUSR1"; break;
        case SIGUSR2: sig_name = "SIGUSR2"; break;
        case SIGCHLD: sig_name = "SIGCHLD"; break;
        default: sig_name = "UNKNOWN"; break;
    }
    
    // Use async-signal-safe functions only
    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer), 
                      "Advanced handler: %s from PID %d\n", 
                      sig_name, info->si_pid);
    write(STDOUT_FILENO, buffer, len);
}

void demonstrate_advanced_signals() {
    SECTION("Advanced Signal Handling with sigaction");
    
    struct sigaction sa;
    sa.sa_sigaction = advanced_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO; // Use extended signal handler
    
    // Install handlers
    sigaction(SIGUSR1, &sa, nullptr);
    sigaction(SIGUSR2, &sa, nullptr);
    sigaction(SIGCHLD, &sa, nullptr);
    
    std::cout << "Advanced signal handlers installed" << std::endl;
    std::cout << "Process PID: " << getpid() << std::endl;
    
    // Test with fork to generate SIGCHLD
    pid_t child_pid = fork();
    if (child_pid == 0) {
        // Child process
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        exit(0);
    } else if (child_pid > 0) {
        // Parent process
        std::cout << "Child PID: " << child_pid << std::endl;
        
        // Send signals to self
        kill(getpid(), SIGUSR1);
        kill(getpid(), SIGUSR2);
        
        // Wait for child and SIGCHLD
        int status;
        wait(&status);
        std::cout << "Child exited with status: " << status << std::endl;
    }
}

// =============================================================================
// 3. SIGNAL MASKING AND BLOCKING
// =============================================================================

void demonstrate_signal_masking() {
    SECTION("Signal Masking and Blocking");
    
    sigset_t new_mask, old_mask, pending_mask;
    
    // Create signal set
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGUSR1);
    sigaddset(&new_mask, SIGUSR2);
    
    std::cout << "Blocking SIGUSR1 and SIGUSR2" << std::endl;
    
    // Block signals
    if (sigprocmask(SIG_BLOCK, &new_mask, &old_mask) == -1) {
        std::cerr << "sigprocmask failed: " << strerror(errno) << std::endl;
        return;
    }
    
    // Send signals to self (they will be blocked)
    std::cout << "Sending blocked signals..." << std::endl;
    kill(getpid(), SIGUSR1);
    kill(getpid(), SIGUSR2);
    
    // Check pending signals
    if (sigpending(&pending_mask) == 0) {
        std::cout << "Pending signals:" << std::endl;
        if (sigismember(&pending_mask, SIGUSR1)) {
            std::cout << "  SIGUSR1 is pending" << std::endl;
        }
        if (sigismember(&pending_mask, SIGUSR2)) {
            std::cout << "  SIGUSR2 is pending" << std::endl;
        }
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Unblock signals
    std::cout << "Unblocking signals..." << std::endl;
    sigprocmask(SIG_SETMASK, &old_mask, nullptr);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Signals should now be delivered" << std::endl;
}

// =============================================================================
// 4. FORK AND PROCESS CREATION
// =============================================================================

void demonstrate_basic_fork() {
    SECTION("Basic Fork and Process Creation");
    
    std::cout << "Before fork - PID: " << getpid() << std::endl;
    
    pid_t pid = fork();
    
    if (pid == -1) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        return;
    } else if (pid == 0) {
        // Child process
        std::cout << "Child process - PID: " << getpid() 
                  << ", Parent PID: " << getppid() << std::endl;
        
        // Child does some work
        for (int i = 0; i < 3; ++i) {
            std::cout << "Child working... " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        std::cout << "Child exiting" << std::endl;
        exit(42); // Exit with specific code
    } else {
        // Parent process
        std::cout << "Parent process - PID: " << getpid() 
                  << ", Child PID: " << pid << std::endl;
        
        // Parent does some work
        for (int i = 0; i < 2; ++i) {
            std::cout << "Parent working... " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        
        // Wait for child
        int status;
        pid_t waited_pid = wait(&status);
        
        std::cout << "Child " << waited_pid << " exited with status: ";
        if (WIFEXITED(status)) {
            std::cout << WEXITSTATUS(status) << std::endl;
        } else {
            std::cout << "abnormal termination" << std::endl;
        }
    }
}

// =============================================================================
// 5. MULTIPLE CHILDREN AND PROCESS MANAGEMENT
// =============================================================================

void demonstrate_multiple_children() {
    SECTION("Multiple Children and Process Management");
    
    const int num_children = 3;
    std::vector<pid_t> children;
    
    for (int i = 0; i < num_children; ++i) {
        pid_t pid = fork();
        
        if (pid == -1) {
            std::cerr << "Fork failed: " << strerror(errno) << std::endl;
            break;
        } else if (pid == 0) {
            // Child process
            int child_id = i;
            std::cout << "Child " << child_id << " (PID: " << getpid() << ") starting" << std::endl;
            
            // Each child does different amount of work
            int work_time = (child_id + 1) * 200;
            std::this_thread::sleep_for(std::chrono::milliseconds(work_time));
            
            std::cout << "Child " << child_id << " finishing" << std::endl;
            exit(child_id + 10); // Exit with unique code
        } else {
            // Parent process
            children.push_back(pid);
            std::cout << "Parent created child " << i << " (PID: " << pid << ")" << std::endl;
        }
    }
    
    // Parent waits for all children
    std::cout << "Parent waiting for all children..." << std::endl;
    
    for (size_t i = 0; i < children.size(); ++i) {
        int status;
        pid_t finished_pid = wait(&status);
        
        std::cout << "Child " << finished_pid << " finished";
        if (WIFEXITED(status)) {
            std::cout << " with exit code: " << WEXITSTATUS(status);
        }
        std::cout << std::endl;
    }
    
    std::cout << "All children completed" << std::endl;
}

// =============================================================================
// 6. EXEC FAMILY FUNCTIONS
// =============================================================================

void demonstrate_exec() {
    SECTION("Exec Family Functions");
    
    pid_t pid = fork();
    
    if (pid == -1) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        return;
    } else if (pid == 0) {
        // Child process - execute external command
        std::cout << "Child executing 'ls -l /tmp'" << std::endl;
        
        // Replace child process image with ls command
        execl("/bin/ls", "ls", "-l", "/tmp", nullptr);
        
        // This line should never be reached if exec succeeds
        std::cerr << "exec failed: " << strerror(errno) << std::endl;
        exit(1);
    } else {
        // Parent process
        std::cout << "Parent waiting for child to execute command..." << std::endl;
        
        int status;
        wait(&status);
        
        std::cout << "Child command completed" << std::endl;
    }
}

// =============================================================================
// 7. SIGNAL COMMUNICATION BETWEEN PROCESSES
// =============================================================================

void child_signal_handler(int sig) {
    const char* msg;
    switch (sig) {
        case SIGUSR1:
            msg = "Child received SIGUSR1\n";
            break;
        case SIGUSR2:
            msg = "Child received SIGUSR2\n";
            break;
        default:
            msg = "Child received unknown signal\n";
            break;
    }
    write(STDOUT_FILENO, msg, strlen(msg));
}

void demonstrate_signal_communication() {
    SECTION("Signal Communication Between Processes");
    
    pid_t pid = fork();
    
    if (pid == -1) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        return;
    } else if (pid == 0) {
        // Child process
        signal(SIGUSR1, child_signal_handler);
        signal(SIGUSR2, child_signal_handler);
        
        std::cout << "Child (PID: " << getpid() << ") waiting for signals..." << std::endl;
        
        // Wait for signals
        for (int i = 0; i < 5; ++i) {
            pause(); // Wait for signal
        }
        
        std::cout << "Child received all signals, exiting" << std::endl;
        exit(0);
    } else {
        // Parent process
        std::cout << "Parent sending signals to child (PID: " << pid << ")" << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Send various signals
        kill(pid, SIGUSR1);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        kill(pid, SIGUSR2);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        kill(pid, SIGUSR1);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        kill(pid, SIGUSR2);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        kill(pid, SIGUSR1);
        
        // Wait for child to complete
        wait(nullptr);
        std::cout << "Parent: Signal communication complete" << std::endl;
    }
}

// =============================================================================
// 8. ZOMBIE PROCESSES AND CLEANUP
// =============================================================================

void demonstrate_zombie_prevention() {
    SECTION("Zombie Process Prevention");
    
    // Install SIGCHLD handler to automatically reap children
    struct sigaction sa;
    sa.sa_handler = SIG_IGN; // Ignore SIGCHLD to prevent zombies
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, nullptr);
    
    std::cout << "SIGCHLD handler installed to prevent zombies" << std::endl;
    
    // Create several short-lived children
    for (int i = 0; i < 3; ++i) {
        pid_t pid = fork();
        
        if (pid == 0) {
            // Child process - exits quickly
            std::cout << "Short-lived child " << i << " (PID: " << getpid() << ") exiting" << std::endl;
            exit(i);
        } else if (pid > 0) {
            std::cout << "Created child " << i << " (PID: " << pid << ")" << std::endl;
        }
    }
    
    // Parent doesn't explicitly wait - SIGCHLD handler prevents zombies
    std::cout << "Parent not explicitly waiting - no zombies should be created" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "Check for zombie processes with: ps aux | grep defunct" << std::endl;
}

int main() {
    std::cout << "LINUX SIGNALS AND FORK TUTORIAL\n";
    std::cout << "================================\n";
    
    demonstrate_basic_signals();
    demonstrate_advanced_signals();
    demonstrate_signal_masking();
    demonstrate_basic_fork();
    demonstrate_multiple_children();
    demonstrate_exec();
    demonstrate_signal_communication();
    demonstrate_zombie_prevention();
    
    std::cout << "\nKey Signal and Fork Concepts:" << std::endl;
    std::cout << "- Signals provide asynchronous event notification" << std::endl;
    std::cout << "- fork() creates identical child processes" << std::endl;
    std::cout << "- exec() replaces process image with new program" << std::endl;
    std::cout << "- wait() prevents zombie processes" << std::endl;
    std::cout << "- Signal handlers must use async-signal-safe functions" << std::endl;
    std::cout << "- Signal masking controls signal delivery" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Signals are asynchronous events that interrupt normal program flow
- fork() creates exact copies of the calling process
- Child processes inherit signal handlers from parent
- exec() family replaces process image while keeping PID
- Zombie processes occur when parent doesn't wait() for children
- Signal handlers must be async-signal-safe
- sigaction() provides more control than signal()
- Signal masking allows temporary blocking of signals
- Proper process management prevents resource leaks
*/
