/*
 * LINUX SYSTEM LEVEL C++ - POSIX THREADS (PTHREADS)
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -pthread threads_with_pthreads.cpp -o pthreads_demo
 */

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

#define SECTION(name) std::cout << "\n=== " << name << " ===" << std::endl

// =============================================================================
// 1. BASIC THREAD CREATION AND JOINING
// =============================================================================

struct thread_data {
    int thread_id;
    std::string message;
};

void* basic_thread_function(void* arg) {
    thread_data* data = static_cast<thread_data*>(arg);
    
    std::cout << "Thread " << data->thread_id << " starting: " << data->message << std::endl;
    
    // Simulate work
    sleep(1);
    
    std::cout << "Thread " << data->thread_id << " finishing" << std::endl;
    
    // Return value
    int* result = new int(data->thread_id * 10);
    return static_cast<void*>(result);
}

void demonstrate_basic_threads() {
    SECTION("Basic Thread Creation and Joining");
    
    const int num_threads = 3;
    pthread_t threads[num_threads];
    thread_data data[num_threads];
    
    // Create threads
    for (int i = 0; i < num_threads; ++i) {
        data[i].thread_id = i;
        data[i].message = "Hello from thread " + std::to_string(i);
        
        int result = pthread_create(&threads[i], nullptr, basic_thread_function, &data[i]);
        if (result != 0) {
            std::cerr << "pthread_create failed: " << strerror(result) << std::endl;
            return;
        }
        
        std::cout << "Created thread " << i << std::endl;
    }
    
    // Join threads and get return values
    for (int i = 0; i < num_threads; ++i) {
        void* return_value;
        int result = pthread_join(threads[i], &return_value);
        
        if (result == 0) {
            int* value = static_cast<int*>(return_value);
            std::cout << "Thread " << i << " returned: " << *value << std::endl;
            delete value; // Clean up allocated memory
        } else {
            std::cerr << "pthread_join failed: " << strerror(result) << std::endl;
        }
    }
    
    std::cout << "All threads completed" << std::endl;
}

// =============================================================================
// 2. THREAD ATTRIBUTES
// =============================================================================

void* detached_thread_function(void* arg) {
    int thread_id = *static_cast<int*>(arg);
    
    std::cout << "Detached thread " << thread_id << " running" << std::endl;
    sleep(1);
    std::cout << "Detached thread " << thread_id << " exiting" << std::endl;
    
    return nullptr;
}

void demonstrate_thread_attributes() {
    SECTION("Thread Attributes");
    
    pthread_attr_t attr;
    pthread_t thread;
    
    // Initialize attributes
    if (pthread_attr_init(&attr) != 0) {
        std::cerr << "pthread_attr_init failed" << std::endl;
        return;
    }
    
    // Set detached state
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        std::cerr << "pthread_attr_setdetachstate failed" << std::endl;
        pthread_attr_destroy(&attr);
        return;
    }
    
    // Set stack size
    size_t stack_size = 1024 * 1024; // 1MB
    if (pthread_attr_setstacksize(&attr, stack_size) != 0) {
        std::cerr << "pthread_attr_setstacksize failed" << std::endl;
    } else {
        std::cout << "Set stack size to " << stack_size << " bytes" << std::endl;
    }
    
    // Get and display current attributes
    int detach_state;
    size_t current_stack_size;
    
    pthread_attr_getdetachstate(&attr, &detach_state);
    pthread_attr_getstacksize(&attr, &current_stack_size);
    
    std::cout << "Thread attributes:" << std::endl;
    std::cout << "  Detach state: " << (detach_state == PTHREAD_CREATE_DETACHED ? "Detached" : "Joinable") << std::endl;
    std::cout << "  Stack size: " << current_stack_size << " bytes" << std::endl;
    
    // Create detached thread
    int thread_id = 1;
    if (pthread_create(&thread, &attr, detached_thread_function, &thread_id) == 0) {
        std::cout << "Created detached thread" << std::endl;
        // Cannot join detached thread
        sleep(2); // Wait for it to complete
    }
    
    // Clean up attributes
    pthread_attr_destroy(&attr);
}

// =============================================================================
// 3. MUTEX SYNCHRONIZATION
// =============================================================================

pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
int shared_counter = 0;

void* mutex_thread_function(void* arg) {
    int thread_id = *static_cast<int*>(arg);
    
    for (int i = 0; i < 1000; ++i) {
        // Lock mutex
        pthread_mutex_lock(&counter_mutex);
        
        // Critical section
        int temp = shared_counter;
        temp++; // Simulate some processing
        shared_counter = temp;
        
        // Unlock mutex
        pthread_mutex_unlock(&counter_mutex);
    }
    
    std::cout << "Thread " << thread_id << " completed 1000 increments" << std::endl;
    return nullptr;
}

void demonstrate_mutex() {
    SECTION("Mutex Synchronization");
    
    const int num_threads = 5;
    pthread_t threads[num_threads];
    int thread_ids[num_threads];
    
    shared_counter = 0;
    
    std::cout << "Initial counter value: " << shared_counter << std::endl;
    
    // Create threads
    for (int i = 0; i < num_threads; ++i) {
        thread_ids[i] = i;
        pthread_create(&threads[i], nullptr, mutex_thread_function, &thread_ids[i]);
    }
    
    // Join all threads
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    
    std::cout << "Final counter value: " << shared_counter << std::endl;
    std::cout << "Expected value: " << (num_threads * 1000) << std::endl;
    
    // Destroy mutex
    pthread_mutex_destroy(&counter_mutex);
}

// =============================================================================
// 4. CONDITION VARIABLES
// =============================================================================

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_condition = PTHREAD_COND_INITIALIZER;
std::vector<int> shared_queue;
bool queue_shutdown = false;

void* producer_function(void* arg) {
    int producer_id = *static_cast<int*>(arg);
    
    for (int i = 0; i < 5; ++i) {
        pthread_mutex_lock(&queue_mutex);
        
        int item = producer_id * 100 + i;
        shared_queue.push_back(item);
        std::cout << "Producer " << producer_id << " produced: " << item << std::endl;
        
        // Signal waiting consumers
        pthread_cond_signal(&queue_condition);
        
        pthread_mutex_unlock(&queue_mutex);
        
        usleep(100000); // 100ms delay
    }
    
    return nullptr;
}

void* consumer_function(void* arg) {
    int consumer_id = *static_cast<int*>(arg);
    
    while (true) {
        pthread_mutex_lock(&queue_mutex);
        
        // Wait for items in queue
        while (shared_queue.empty() && !queue_shutdown) {
            pthread_cond_wait(&queue_condition, &queue_mutex);
        }
        
        if (queue_shutdown && shared_queue.empty()) {
            pthread_mutex_unlock(&queue_mutex);
            break;
        }
        
        if (!shared_queue.empty()) {
            int item = shared_queue.front();
            shared_queue.erase(shared_queue.begin());
            std::cout << "Consumer " << consumer_id << " consumed: " << item << std::endl;
        }
        
        pthread_mutex_unlock(&queue_mutex);
        
        usleep(150000); // 150ms processing time
    }
    
    std::cout << "Consumer " << consumer_id << " shutting down" << std::endl;
    return nullptr;
}

void demonstrate_condition_variables() {
    SECTION("Condition Variables (Producer-Consumer)");
    
    const int num_producers = 2;
    const int num_consumers = 2;
    
    pthread_t producers[num_producers];
    pthread_t consumers[num_consumers];
    int producer_ids[num_producers];
    int consumer_ids[num_consumers];
    
    shared_queue.clear();
    queue_shutdown = false;
    
    // Create consumers first
    for (int i = 0; i < num_consumers; ++i) {
        consumer_ids[i] = i;
        pthread_create(&consumers[i], nullptr, consumer_function, &consumer_ids[i]);
    }
    
    // Create producers
    for (int i = 0; i < num_producers; ++i) {
        producer_ids[i] = i;
        pthread_create(&producers[i], nullptr, producer_function, &producer_ids[i]);
    }
    
    // Wait for producers to finish
    for (int i = 0; i < num_producers; ++i) {
        pthread_join(producers[i], nullptr);
    }
    
    // Signal consumers to shutdown
    pthread_mutex_lock(&queue_mutex);
    queue_shutdown = true;
    pthread_cond_broadcast(&queue_condition);
    pthread_mutex_unlock(&queue_mutex);
    
    // Wait for consumers to finish
    for (int i = 0; i < num_consumers; ++i) {
        pthread_join(consumers[i], nullptr);
    }
    
    std::cout << "Producer-Consumer demonstration complete" << std::endl;
    
    // Cleanup
    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&queue_condition);
}

// =============================================================================
// 5. READ-WRITE LOCKS
// =============================================================================

pthread_rwlock_t rw_lock = PTHREAD_RWLOCK_INITIALIZER;
int shared_data = 0;

void* reader_function(void* arg) {
    int reader_id = *static_cast<int*>(arg);
    
    for (int i = 0; i < 3; ++i) {
        pthread_rwlock_rdlock(&rw_lock);
        
        std::cout << "Reader " << reader_id << " read: " << shared_data << std::endl;
        sleep(1); // Simulate reading time
        
        pthread_rwlock_unlock(&rw_lock);
        
        usleep(500000); // 500ms between reads
    }
    
    return nullptr;
}

void* writer_function(void* arg) {
    int writer_id = *static_cast<int*>(arg);
    
    for (int i = 0; i < 2; ++i) {
        pthread_rwlock_wrlock(&rw_lock);
        
        shared_data += 10;
        std::cout << "Writer " << writer_id << " wrote: " << shared_data << std::endl;
        sleep(1); // Simulate writing time
        
        pthread_rwlock_unlock(&rw_lock);
        
        sleep(1); // Delay between writes
    }
    
    return nullptr;
}

void demonstrate_rwlock() {
    SECTION("Read-Write Locks");
    
    const int num_readers = 3;
    const int num_writers = 2;
    
    pthread_t readers[num_readers];
    pthread_t writers[num_writers];
    int reader_ids[num_readers];
    int writer_ids[num_writers];
    
    shared_data = 0;
    
    // Create readers
    for (int i = 0; i < num_readers; ++i) {
        reader_ids[i] = i;
        pthread_create(&readers[i], nullptr, reader_function, &reader_ids[i]);
    }
    
    // Create writers
    for (int i = 0; i < num_writers; ++i) {
        writer_ids[i] = i;
        pthread_create(&writers[i], nullptr, writer_function, &writer_ids[i]);
    }
    
    // Wait for all threads
    for (int i = 0; i < num_readers; ++i) {
        pthread_join(readers[i], nullptr);
    }
    
    for (int i = 0; i < num_writers; ++i) {
        pthread_join(writers[i], nullptr);
    }
    
    std::cout << "Read-Write lock demonstration complete" << std::endl;
    
    pthread_rwlock_destroy(&rw_lock);
}

// =============================================================================
// 6. THREAD-SPECIFIC DATA
// =============================================================================

pthread_key_t thread_key;

void cleanup_thread_data(void* data) {
    std::cout << "Cleaning up thread-specific data: " << *static_cast<int*>(data) << std::endl;
    delete static_cast<int*>(data);
}

void* thread_specific_function(void* arg) {
    int thread_id = *static_cast<int*>(arg);
    
    // Allocate thread-specific data
    int* thread_data = new int(thread_id * 1000);
    pthread_setspecific(thread_key, thread_data);
    
    std::cout << "Thread " << thread_id << " set thread-specific data: " << *thread_data << std::endl;
    
    // Simulate work
    sleep(1);
    
    // Retrieve thread-specific data
    int* retrieved_data = static_cast<int*>(pthread_getspecific(thread_key));
    if (retrieved_data) {
        std::cout << "Thread " << thread_id << " retrieved data: " << *retrieved_data << std::endl;
    }
    
    return nullptr;
}

void demonstrate_thread_specific_data() {
    SECTION("Thread-Specific Data");
    
    // Create key for thread-specific data
    if (pthread_key_create(&thread_key, cleanup_thread_data) != 0) {
        std::cerr << "pthread_key_create failed" << std::endl;
        return;
    }
    
    const int num_threads = 3;
    pthread_t threads[num_threads];
    int thread_ids[num_threads];
    
    // Create threads
    for (int i = 0; i < num_threads; ++i) {
        thread_ids[i] = i;
        pthread_create(&threads[i], nullptr, thread_specific_function, &thread_ids[i]);
    }
    
    // Join threads
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    
    // Delete key
    pthread_key_delete(thread_key);
    
    std::cout << "Thread-specific data demonstration complete" << std::endl;
}

// =============================================================================
// 7. THREAD CANCELLATION
// =============================================================================

void* cancellable_thread_function(void* arg) {
    int thread_id = *static_cast<int*>(arg);
    
    // Set cancellation state and type
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr);
    
    std::cout << "Cancellable thread " << thread_id << " starting" << std::endl;
    
    for (int i = 0; i < 10; ++i) {
        std::cout << "Thread " << thread_id << " working... " << i << std::endl;
        
        // Test for cancellation
        pthread_testcancel();
        
        sleep(1);
    }
    
    std::cout << "Thread " << thread_id << " completed normally" << std::endl;
    return nullptr;
}

void demonstrate_thread_cancellation() {
    SECTION("Thread Cancellation");
    
    pthread_t thread;
    int thread_id = 1;
    
    // Create cancellable thread
    if (pthread_create(&thread, nullptr, cancellable_thread_function, &thread_id) != 0) {
        std::cerr << "pthread_create failed" << std::endl;
        return;
    }
    
    // Let thread run for a bit
    sleep(3);
    
    // Cancel the thread
    std::cout << "Main thread cancelling worker thread" << std::endl;
    if (pthread_cancel(thread) == 0) {
        std::cout << "Cancellation request sent" << std::endl;
    }
    
    // Wait for thread to complete
    void* result;
    int join_result = pthread_join(thread, &result);
    
    if (join_result == 0) {
        if (result == PTHREAD_CANCELED) {
            std::cout << "Thread was cancelled" << std::endl;
        } else {
            std::cout << "Thread completed normally" << std::endl;
        }
    }
}

int main() {
    std::cout << "POSIX THREADS (PTHREADS) TUTORIAL\n";
    std::cout << "=================================\n";
    
    demonstrate_basic_threads();
    demonstrate_thread_attributes();
    demonstrate_mutex();
    demonstrate_condition_variables();
    demonstrate_rwlock();
    demonstrate_thread_specific_data();
    demonstrate_thread_cancellation();
    
    std::cout << "\nKey Pthread Concepts:" << std::endl;
    std::cout << "- pthread_create() creates new threads" << std::endl;
    std::cout << "- pthread_join() waits for thread completion" << std::endl;
    std::cout << "- Mutexes provide mutual exclusion" << std::endl;
    std::cout << "- Condition variables enable thread coordination" << std::endl;
    std::cout << "- Read-write locks allow multiple readers" << std::endl;
    std::cout << "- Thread-specific data provides per-thread storage" << std::endl;
    std::cout << "- Thread cancellation enables graceful termination" << std::endl;
    
    return 0;
}

/*
KEY INSIGHTS:
- Pthreads provide low-level thread control in C/C++
- Always check return values from pthread functions
- Mutexes prevent race conditions in shared data access
- Condition variables enable efficient thread synchronization
- Read-write locks optimize concurrent read access
- Thread attributes control stack size, scheduling, etc.
- Thread-specific data provides isolated per-thread storage
- Proper cleanup prevents resource leaks
- Thread cancellation requires careful design
*/
