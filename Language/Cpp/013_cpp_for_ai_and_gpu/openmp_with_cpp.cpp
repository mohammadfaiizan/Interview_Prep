/*
 * C++ FOR AI AND GPU - OPENMP PARALLEL COMPUTING
 * 
 * Comprehensive guide to OpenMP for parallel computing in AI/ML applications.
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -fopenmp openmp_with_cpp.cpp -o openmp_demo
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath>

// Mock OpenMP for demonstration (in real code, use #include <omp.h>)
namespace omp {
    int get_num_threads() { return 4; }
    int get_thread_num() { return 0; }
    int get_max_threads() { return 4; }
    void set_num_threads(int num) { std::cout << "Setting threads to " << num << std::endl; }
    double get_wtime() { 
        return std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(); 
    }
}

// =============================================================================
// BASIC PARALLEL CONSTRUCTS
// =============================================================================

void demonstrate_basic_parallelism() {
    std::cout << "\n=== OpenMP Basic Parallelism ===" << std::endl;
    
    std::cout << "\n1. Parallel Regions:" << std::endl;
    std::cout << "Available threads: " << omp::get_max_threads() << std::endl;
    
    // Simulate parallel region
    std::cout << "Executing parallel region..." << std::endl;
    for (int i = 0; i < 4; ++i) {
        std::cout << "Thread " << i << " executing" << std::endl;
    }
    
    std::cout << "\n2. Parallel For Loops:" << std::endl;
    std::vector<int> data(1000);
    std::iota(data.begin(), data.end(), 0);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Simulate parallel for
    std::cout << "Processing " << data.size() << " elements in parallel..." << std::endl;
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = data[i] * data[i]; // Square each element
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Parallel processing completed in " << duration.count() << " microseconds" << std::endl;
    std::cout << "First 10 results: ";
    for (int i = 0; i < 10; ++i) {
        std::cout << data[i] << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// MATRIX OPERATIONS WITH OPENMP
// =============================================================================

class ParallelMatrix {
private:
    std::vector<std::vector<double>> data_;
    size_t rows_, cols_;
    
public:
    ParallelMatrix(size_t rows, size_t cols) : rows_(rows), cols_(cols) {
        data_.resize(rows, std::vector<double>(cols, 0.0));
    }
    
    double& operator()(size_t row, size_t col) { return data_[row][col]; }
    const double& operator()(size_t row, size_t col) const { return data_[row][col]; }
    
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }
    
    void randomize() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-1.0, 1.0);
        
        std::cout << "Randomizing matrix in parallel..." << std::endl;
        // Simulate parallel randomization
        for (size_t i = 0; i < rows_; ++i) {
            for (size_t j = 0; j < cols_; ++j) {
                data_[i][j] = dis(gen);
            }
        }
    }
    
    ParallelMatrix multiply(const ParallelMatrix& other) const {
        if (cols_ != other.rows_) {
            throw std::invalid_argument("Matrix dimensions don't match for multiplication");
        }
        
        ParallelMatrix result(rows_, other.cols_);
        
        std::cout << "Performing parallel matrix multiplication..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        
        // Simulate parallel matrix multiplication
        for (size_t i = 0; i < rows_; ++i) {
            for (size_t j = 0; j < other.cols_; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < cols_; ++k) {
                    sum += data_[i][k] * other(k, j);
                }
                result(i, j) = sum;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Matrix multiplication completed in " << duration.count() << " ms" << std::endl;
        return result;
    }
    
    void print_sample() const {
        std::cout << "Matrix sample (top-left 5x5):" << std::endl;
        for (size_t i = 0; i < std::min(size_t(5), rows_); ++i) {
            for (size_t j = 0; j < std::min(size_t(5), cols_); ++j) {
                std::cout << std::fixed << std::setprecision(3) << data_[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
};

void demonstrate_matrix_operations() {
    std::cout << "\n=== Parallel Matrix Operations ===" << std::endl;
    
    std::cout << "\n1. Creating Matrices:" << std::endl;
    ParallelMatrix A(512, 512);
    ParallelMatrix B(512, 512);
    
    A.randomize();
    B.randomize();
    
    std::cout << "Matrix A:" << std::endl;
    A.print_sample();
    
    std::cout << "\n2. Matrix Multiplication:" << std::endl;
    auto C = A.multiply(B);
    
    std::cout << "Result matrix C:" << std::endl;
    C.print_sample();
    
    // Performance comparison
    std::cout << "\n3. Performance Analysis:" << std::endl;
    std::cout << "• Sequential version would take longer" << std::endl;
    std::cout << "• Parallel version utilizes multiple CPU cores" << std::endl;
    std::cout << "• Speedup depends on problem size and thread count" << std::endl;
}

// =============================================================================
// NEURAL NETWORK TRAINING WITH OPENMP
// =============================================================================

class ParallelNeuralNetwork {
private:
    std::vector<std::vector<double>> weights1_;
    std::vector<std::vector<double>> weights2_;
    std::vector<double> bias1_;
    std::vector<double> bias2_;
    
    int input_size_;
    int hidden_size_;
    int output_size_;
    
public:
    ParallelNeuralNetwork(int input_size, int hidden_size, int output_size)
        : input_size_(input_size), hidden_size_(hidden_size), output_size_(output_size) {
        
        // Initialize weights and biases
        weights1_.resize(hidden_size, std::vector<double>(input_size));
        weights2_.resize(output_size, std::vector<double>(hidden_size));
        bias1_.resize(hidden_size);
        bias2_.resize(output_size);
        
        initialize_weights();
        
        std::cout << "Created Neural Network: " << input_size << " -> " 
                  << hidden_size << " -> " << output_size << std::endl;
    }
    
    void initialize_weights() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-0.1, 0.1);
        
        std::cout << "Initializing weights in parallel..." << std::endl;
        
        // Parallel weight initialization
        for (int i = 0; i < hidden_size_; ++i) {
            for (int j = 0; j < input_size_; ++j) {
                weights1_[i][j] = dis(gen);
            }
            bias1_[i] = dis(gen);
        }
        
        for (int i = 0; i < output_size_; ++i) {
            for (int j = 0; j < hidden_size_; ++j) {
                weights2_[i][j] = dis(gen);
            }
            bias2_[i] = dis(gen);
        }
    }
    
    std::vector<double> forward(const std::vector<double>& input) {
        // Hidden layer computation
        std::vector<double> hidden(hidden_size_, 0.0);
        
        std::cout << "Computing hidden layer in parallel..." << std::endl;
        // Simulate parallel computation
        for (int i = 0; i < hidden_size_; ++i) {
            double sum = bias1_[i];
            for (int j = 0; j < input_size_; ++j) {
                sum += weights1_[i][j] * input[j];
            }
            hidden[i] = std::tanh(sum); // Activation function
        }
        
        // Output layer computation
        std::vector<double> output(output_size_, 0.0);
        
        std::cout << "Computing output layer in parallel..." << std::endl;
        // Simulate parallel computation
        for (int i = 0; i < output_size_; ++i) {
            double sum = bias2_[i];
            for (int j = 0; j < hidden_size_; ++j) {
                sum += weights2_[i][j] * hidden[j];
            }
            output[i] = 1.0 / (1.0 + std::exp(-sum)); // Sigmoid activation
        }
        
        return output;
    }
    
    void train_batch(const std::vector<std::vector<double>>& inputs,
                    const std::vector<std::vector<double>>& targets,
                    double learning_rate) {
        
        std::cout << "Training batch of " << inputs.size() << " samples in parallel..." << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        // Simulate parallel batch processing
        for (size_t sample = 0; sample < inputs.size(); ++sample) {
            auto prediction = forward(inputs[sample]);
            
            // Compute loss (simplified)
            double loss = 0.0;
            for (size_t i = 0; i < prediction.size(); ++i) {
                double error = targets[sample][i] - prediction[i];
                loss += error * error;
            }
            
            // Simplified gradient update (in real implementation, would compute gradients)
            std::cout << "Sample " << sample << " - Loss: " << loss << std::endl;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Batch training completed in " << duration.count() << " ms" << std::endl;
    }
};

void demonstrate_neural_network_training() {
    std::cout << "\n=== Parallel Neural Network Training ===" << std::endl;
    
    std::cout << "\n1. Creating Network:" << std::endl;
    ParallelNeuralNetwork network(784, 128, 10); // MNIST-like architecture
    
    std::cout << "\n2. Generating Training Data:" << std::endl;
    const int batch_size = 32;
    std::vector<std::vector<double>> inputs(batch_size, std::vector<double>(784));
    std::vector<std::vector<double>> targets(batch_size, std::vector<double>(10));
    
    // Generate random training data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    for (int i = 0; i < batch_size; ++i) {
        for (int j = 0; j < 784; ++j) {
            inputs[i][j] = dis(gen);
        }
        for (int j = 0; j < 10; ++j) {
            targets[i][j] = (j == i % 10) ? 1.0 : 0.0; // One-hot encoding
        }
    }
    
    std::cout << "Generated " << batch_size << " training samples" << std::endl;
    
    std::cout << "\n3. Training Network:" << std::endl;
    network.train_batch(inputs, targets, 0.01);
    
    std::cout << "\n4. Testing Forward Pass:" << std::endl;
    auto test_input = inputs[0];
    auto prediction = network.forward(test_input);
    
    std::cout << "Prediction: ";
    for (size_t i = 0; i < prediction.size(); ++i) {
        std::cout << std::fixed << std::setprecision(3) << prediction[i] << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// PARALLEL ALGORITHMS FOR AI
// =============================================================================

void demonstrate_parallel_algorithms() {
    std::cout << "\n=== Parallel Algorithms for AI ===" << std::endl;
    
    std::cout << "\n1. Parallel Reduction:" << std::endl;
    std::vector<double> data(1000000);
    std::iota(data.begin(), data.end(), 1.0);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Parallel sum reduction
    double sum = 0.0;
    std::cout << "Computing parallel sum..." << std::endl;
    // Simulate parallel reduction
    sum = std::accumulate(data.begin(), data.end(), 0.0);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Sum: " << sum << std::endl;
    std::cout << "Parallel reduction time: " << duration.count() << " microseconds" << std::endl;
    
    std::cout << "\n2. Parallel Transform:" << std::endl;
    std::vector<double> input_data(100000);
    std::vector<double> output_data(100000);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-10.0, 10.0);
    
    for (auto& val : input_data) {
        val = dis(gen);
    }
    
    start = std::chrono::high_resolution_clock::now();
    
    // Parallel transform (e.g., activation function)
    std::cout << "Applying ReLU activation in parallel..." << std::endl;
    std::transform(input_data.begin(), input_data.end(), output_data.begin(),
                   [](double x) { return std::max(0.0, x); });
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Parallel transform completed in " << duration.count() << " microseconds" << std::endl;
    
    std::cout << "\n3. Parallel Sorting:" << std::endl;
    std::vector<double> sort_data(100000);
    for (auto& val : sort_data) {
        val = dis(gen);
    }
    
    start = std::chrono::high_resolution_clock::now();
    
    std::cout << "Performing parallel sort..." << std::endl;
    std::sort(sort_data.begin(), sort_data.end());
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Parallel sort completed in " << duration.count() << " microseconds" << std::endl;
    std::cout << "First 10 sorted values: ";
    for (int i = 0; i < 10; ++i) {
        std::cout << std::fixed << std::setprecision(2) << sort_data[i] << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// PERFORMANCE OPTIMIZATION
// =============================================================================

void demonstrate_optimization_techniques() {
    std::cout << "\n=== OpenMP Optimization Techniques ===" << std::endl;
    
    std::cout << "\n1. Load Balancing:" << std::endl;
    std::cout << "• Use dynamic scheduling for irregular workloads" << std::endl;
    std::cout << "• Static scheduling for uniform work distribution" << std::endl;
    std::cout << "• Guided scheduling for decreasing work sizes" << std::endl;
    
    std::cout << "\n2. Memory Access Optimization:" << std::endl;
    std::cout << "• Ensure data locality for cache efficiency" << std::endl;
    std::cout << "• Use first-touch policy for NUMA systems" << std::endl;
    std::cout << "• Minimize false sharing between threads" << std::endl;
    
    std::cout << "\n3. Thread Management:" << std::endl;
    std::cout << "• Set optimal thread count based on hardware" << std::endl;
    std::cout << "• Use thread affinity for consistent performance" << std::endl;
    std::cout << "• Minimize thread creation overhead" << std::endl;
    
    std::cout << "\n4. Synchronization:" << std::endl;
    std::cout << "• Use atomic operations for simple updates" << std::endl;
    std::cout << "• Minimize critical sections" << std::endl;
    std::cout << "• Use reduction clauses for accumulation" << std::endl;
}

int main() {
    std::cout << "OPENMP PARALLEL COMPUTING FOR AI\n";
    std::cout << "=================================\n";
    
    demonstrate_basic_parallelism();
    demonstrate_matrix_operations();
    demonstrate_neural_network_training();
    demonstrate_parallel_algorithms();
    demonstrate_optimization_techniques();
    
    std::cout << "\nKey OpenMP Concepts:" << std::endl;
    std::cout << "• Shared memory parallelism for multi-core CPUs" << std::endl;
    std::cout << "• Pragma-based parallel programming model" << std::endl;
    std::cout << "• Excellent for data-parallel computations in AI" << std::endl;
    std::cout << "• Automatic load balancing and work distribution" << std::endl;
    std::cout << "• High-level abstraction with fine-grained control" << std::endl;
    std::cout << "• Portable across different platforms and compilers" << std::endl;
    
    return 0;
}
