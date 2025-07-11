/*
 * LINUX SYSTEM LEVEL C++ - INTERPROCESS COMMUNICATION (IPC)
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread interprocess_communication.cpp -o ipc
 */

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <signal.h>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. PIPES - ANONYMOUS AND NAMED
// =============================================================================

void demonstrate_anonymous_pipes() {
    SECTION("Anonymous Pipes");
    
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cerr << "Pipe creation failed: " << strerror(errno) << std::endl;
        return;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        return;
    }
    
    if (pid == 0) {
        // Child process - reader
        close(pipefd[1]); // Close write end
        
        char buffer[256];
        ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::cout << "Child received: " << buffer << std::endl;
        }
        
        close(pipefd[0]);
        exit(0);
    } else {
        // Parent process - writer
        close(pipefd[0]); // Close read end
        
        const char* message = "Hello from parent!";
        write(pipefd[1], message, strlen(message));
        
        close(pipefd[1]);
        wait(nullptr); // Wait for child to complete
        
        std::cout << "Parent sent message through pipe" << std::endl;
    }
}

void demonstrate_named_pipes() {
    SECTION("Named Pipes (FIFOs)");
    
    const char* fifo_path = "/tmp/test_fifo";
    
    // Create named pipe
    if (mkfifo(fifo_path, 0666) == -1 && errno != EEXIST) {
        std::cerr << "FIFO creation failed: " << strerror(errno) << std::endl;
        return;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        return;
    }
    
    if (pid == 0) {
        // Child process - reader
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        int fd = open(fifo_path, O_RDONLY);
        if (fd != -1) {
            char buffer[256];
            ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                std::cout << "Child read from FIFO: " << buffer << std::endl;
            }
            close(fd);
        }
        exit(0);
    } else {
        // Parent process - writer
        int fd = open(fifo_path, O_WRONLY);
        if (fd != -1) {
            const char* message = "Hello through FIFO!";
            write(fd, message, strlen(message));
            close(fd);
            std::cout << "Parent wrote to FIFO" << std::endl;
        }
        
        wait(nullptr);
        unlink(fifo_path); // Clean up
    }
}

// =============================================================================
// 2. MESSAGE QUEUES
// =============================================================================

struct message_buffer {
    long msg_type;
    char msg_text[256];
};

void demonstrate_message_queues() {
    SECTION("System V Message Queues");
    
    key_t key = ftok("/tmp", 'A');
    if (key == -1) {
        std::cerr << "ftok failed: " << strerror(errno) << std::endl;
        return;
    }
    
    // Create message queue
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) {
        std::cerr << "msgget failed: " << strerror(errno) << std::endl;
        return;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        return;
    }
    
    if (pid == 0) {
        // Child process - receiver
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        message_buffer msg;
        if (msgrcv(msgid, &msg, sizeof(msg.msg_text), 1, 0) != -1) {
            std::cout << "Child received message: " << msg.msg_text << std::endl;
        }
        exit(0);
    } else {
        // Parent process - sender
        message_buffer msg;
        msg.msg_type = 1;
        strcpy(msg.msg_text, "Hello from message queue!");
        
        if (msgsnd(msgid, &msg, strlen(msg.msg_text) + 1, 0) == 0) {
            std::cout << "Parent sent message to queue" << std::endl;
        }
        
        wait(nullptr);
        
        // Clean up message queue
        msgctl(msgid, IPC_RMID, nullptr);
    }
}

// =============================================================================
// 3. SHARED MEMORY
// =============================================================================

struct shared_data {
    int counter;
    char message[256];
    bool ready;
};

void demonstrate_shared_memory() {
    SECTION("System V Shared Memory");
    
    key_t key = ftok("/tmp", 'B');
    if (key == -1) {
        std::cerr << "ftok failed: " << strerror(errno) << std::endl;
        return;
    }
    
    // Create shared memory segment
    int shmid = shmget(key, sizeof(shared_data), IPC_CREAT | 0666);
    if (shmid == -1) {
        std::cerr << "shmget failed: " << strerror(errno) << std::endl;
        return;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        return;
    }
    
    if (pid == 0) {
        // Child process - reader
        shared_data* data = (shared_data*)shmat(shmid, nullptr, 0);
        if (data == (void*)-1) {
            std::cerr << "shmat failed: " << strerror(errno) << std::endl;
            exit(1);
        }
        
        // Wait for parent to write data
        while (!data->ready) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        std::cout << "Child read from shared memory:" << std::endl;
        std::cout << "  Counter: " << data->counter << std::endl;
        std::cout << "  Message: " << data->message << std::endl;
        
        shmdt(data);
        exit(0);
    } else {
        // Parent process - writer
        shared_data* data = (shared_data*)shmat(shmid, nullptr, 0);
        if (data == (void*)-1) {
            std::cerr << "shmat failed: " << strerror(errno) << std::endl;
            return;
        }
        
        // Write data to shared memory
        data->counter = 42;
        strcpy(data->message, "Hello from shared memory!");
        data->ready = true;
        
        std::cout << "Parent wrote to shared memory" << std::endl;
        
        wait(nullptr);
        
        shmdt(data);
        shmctl(shmid, IPC_RMID, nullptr); // Clean up
    }
}

// =============================================================================
// 4. SEMAPHORES
// =============================================================================

void demonstrate_semaphores() {
    SECTION("System V Semaphores");
    
    key_t key = ftok("/tmp", 'C');
    if (key == -1) {
        std::cerr << "ftok failed: " << strerror(errno) << std::endl;
        return;
    }
    
    // Create semaphore set with one semaphore
    int semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        std::cerr << "semget failed: " << strerror(errno) << std::endl;
        return;
    }
    
    // Initialize semaphore to 1 (binary semaphore)
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        std::cerr << "semctl SETVAL failed: " << strerror(errno) << std::endl;
        return;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        return;
    }
    
    if (pid == 0) {
        // Child process
        std::cout << "Child waiting for semaphore..." << std::endl;
        
        // Wait (P operation)
        struct sembuf sem_op = {0, -1, 0};
        if (semop(semid, &sem_op, 1) == 0) {
            std::cout << "Child acquired semaphore" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::cout << "Child doing work..." << std::endl;
            
            // Signal (V operation)
            sem_op.sem_op = 1;
            semop(semid, &sem_op, 1);
            std::cout << "Child released semaphore" << std::endl;
        }
        exit(0);
    } else {
        // Parent process
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "Parent waiting for semaphore..." << std::endl;
        
        // Wait (P operation)
        struct sembuf sem_op = {0, -1, 0};
        if (semop(semid, &sem_op, 1) == 0) {
            std::cout << "Parent acquired semaphore" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            std::cout << "Parent doing work..." << std::endl;
            
            // Signal (V operation)
            sem_op.sem_op = 1;
            semop(semid, &sem_op, 1);
            std::cout << "Parent released semaphore" << std::endl;
        }
        
        wait(nullptr);
        
        // Clean up semaphore
        semctl(semid, 0, IPC_RMID);
    }
}

// =============================================================================
// 5. SIGNALS
// =============================================================================

volatile sig_atomic_t signal_received = 0;

void signal_handler(int sig) {
    signal_received = sig;
    std::cout << "Received signal: " << sig << std::endl;
}

void demonstrate_signals() {
    SECTION("Signal Communication");
    
    // Install signal handler
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
    
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        return;
    }
    
    if (pid == 0) {
        // Child process
        std::cout << "Child process (PID: " << getpid() << ") waiting for signals..." << std::endl;
        
        // Wait for signals
        for (int i = 0; i < 3; ++i) {
            pause(); // Wait for signal
            std::cout << "Child received signal " << signal_received << std::endl;
        }
        
        exit(0);
    } else {
        // Parent process
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "Parent sending signals to child (PID: " << pid << ")" << std::endl;
        
        // Send signals to child
        kill(pid, SIGUSR1);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        kill(pid, SIGUSR2);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        kill(pid, SIGUSR1);
        
        wait(nullptr);
        std::cout << "Parent finished sending signals" << std::endl;
    }
}

// =============================================================================
// 6. MEMORY MAPPED FILES
// =============================================================================

void demonstrate_memory_mapped_files() {
    SECTION("Memory Mapped Files");
    
    const char* filename = "/tmp/mmap_test.txt";
    const size_t file_size = 1024;
    
    // Create and initialize file
    int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "File creation failed: " << strerror(errno) << std::endl;
        return;
    }
    
    // Extend file to desired size
    if (ftruncate(fd, file_size) == -1) {
        std::cerr << "ftruncate failed: " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    if (pid == 0) {
        // Child process - reader
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        char* mapped = (char*)mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
        if (mapped == MAP_FAILED) {
            std::cerr << "mmap failed: " << strerror(errno) << std::endl;
            exit(1);
        }
        
        std::cout << "Child read from memory mapped file: " << mapped << std::endl;
        
        munmap(mapped, file_size);
        close(fd);
        exit(0);
    } else {
        // Parent process - writer
        char* mapped = (char*)mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mapped == MAP_FAILED) {
            std::cerr << "mmap failed: " << strerror(errno) << std::endl;
            close(fd);
            return;
        }
        
        strcpy(mapped, "Hello from memory mapped file!");
        msync(mapped, strlen(mapped), MS_SYNC); // Ensure data is written
        
        std::cout << "Parent wrote to memory mapped file" << std::endl;
        
        wait(nullptr);
        
        munmap(mapped, file_size);
        close(fd);
        unlink(filename);
    }
}

// =============================================================================
// 7. PERFORMANCE COMPARISON
// =============================================================================

void performance_comparison() {
    SECTION("IPC Performance Comparison");
    
    std::cout << "IPC Mechanism Performance Characteristics:" << std::endl;
    std::cout << "\n1. Pipes (Anonymous):" << std::endl;
    std::cout << "   - Fast for parent-child communication" << std::endl;
    std::cout << "   - Limited to related processes" << std::endl;
    std::cout << "   - Unidirectional (need two pipes for bidirectional)" << std::endl;
    
    std::cout << "\n2. Named Pipes (FIFOs):" << std::endl;
    std::cout << "   - Can be used by unrelated processes" << std::endl;
    std::cout << "   - Filesystem-based, persistent" << std::endl;
    std::cout << "   - Slower than anonymous pipes" << std::endl;
    
    std::cout << "\n3. Message Queues:" << std::endl;
    std::cout << "   - Structured message passing" << std::endl;
    std::cout << "   - Multiple message types" << std::endl;
    std::cout << "   - Moderate performance" << std::endl;
    
    std::cout << "\n4. Shared Memory:" << std::endl;
    std::cout << "   - Fastest IPC mechanism" << std::endl;
    std::cout << "   - Requires synchronization" << std::endl;
    std::cout << "   - Direct memory access" << std::endl;
    
    std::cout << "\n5. Semaphores:" << std::endl;
    std::cout << "   - Synchronization primitive" << std::endl;
    std::cout << "   - Used with shared memory" << std::endl;
    std::cout << "   - Atomic operations" << std::endl;
    
    std::cout << "\n6. Signals:" << std::endl;
    std::cout << "   - Asynchronous notification" << std::endl;
    std::cout << "   - Limited data transfer" << std::endl;
    std::cout << "   - Event-driven communication" << std::endl;
    
    std::cout << "\n7. Memory Mapped Files:" << std::endl;
    std::cout << "   - File-based shared memory" << std::endl;
    std::cout << "   - Persistent across reboots" << std::endl;
    std::cout << "   - Good for large data sets" << std::endl;
}

int main() {
    std::cout << "LINUX INTERPROCESS COMMUNICATION\n";
    std::cout << "=================================\n";
    
    demonstrate_anonymous_pipes();
    demonstrate_named_pipes();
    demonstrate_message_queues();
    demonstrate_shared_memory();
    demonstrate_semaphores();
    demonstrate_signals();
    demonstrate_memory_mapped_files();
    performance_comparison();
    
    std::cout << "\nKey IPC Concepts:" << std::endl;
    std::cout << "- Choose IPC mechanism based on requirements" << std::endl;
    std::cout << "- Shared memory + semaphores = fastest combination" << std::endl;
    std::cout << "- Pipes are simple and efficient for related processes" << std::endl;
    std::cout << "- Message queues provide structured communication" << std::endl;
    std::cout << "- Signals are for asynchronous notifications" << std::endl;
    std::cout << "- Always clean up IPC resources" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Different IPC mechanisms serve different purposes
- Shared memory is fastest but requires synchronization
- Pipes are simple and efficient for parent-child communication
- Message queues provide structured, typed communication
- Semaphores are essential for synchronizing shared resources
- Signals enable asynchronous event notification
- Memory mapped files combine file I/O with shared memory
- Performance vs complexity trade-offs exist
- Always handle errors and clean up resources
*/
