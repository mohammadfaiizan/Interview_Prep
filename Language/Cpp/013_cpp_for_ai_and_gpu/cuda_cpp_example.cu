/*
 * C++ FOR AI AND GPU - CUDA C++ PROGRAMMING
 * 
 * Comprehensive CUDA C++ tutorial for GPU acceleration in AI/ML applications.
 * 
 * Compilation: nvcc -std=c++17 -arch=sm_70 cuda_cpp_example.cu -o cuda_demo
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cmath>

// CUDA headers (mocked for demonstration)
#ifndef __CUDA_RUNTIME_H__
#define __global__
#define __device__
#define __host__
#define __shared__
typedef int cudaError_t;
#define cudaSuccess 0
#define cudaMemcpyHostToDevice 1
#define cudaMemcpyDeviceToHost 2

// Mock CUDA functions
cudaError_t cudaMalloc(void** ptr, size_t size) {
    *ptr = malloc(size);
    std::cout << "cudaMalloc: Allocated " << size << " bytes on GPU" << std::endl;
    return cudaSuccess;
}

cudaError_t cudaMemcpy(void* dst, const void* src, size_t size, int kind) {
    memcpy(dst, src, size);
    if (kind == cudaMemcpyHostToDevice) {
        std::cout << "cudaMemcpy: Host to Device (" << size << " bytes)" << std::endl;
    } else {
        std::cout << "cudaMemcpy: Device to Host (" << size << " bytes)" << std::endl;
    }
    return cudaSuccess;
}

cudaError_t cudaFree(void* ptr) {
    free(ptr);
    std::cout << "cudaFree: Freed GPU memory" << std::endl;
    return cudaSuccess;
}

void cudaDeviceSynchronize() {
    std::cout << "cudaDeviceSynchronize: Waiting for GPU to finish" << std::endl;
}

struct dim3 {
    unsigned int x, y, z;
    dim3(unsigned int x = 1, unsigned int y = 1, unsigned int z = 1) : x(x), y(y), z(z) {}
};

#define blockIdx dim3(0, 0, 0)
#define blockDim dim3(256, 1, 1)
#define threadIdx dim3(0, 0, 0)
#define gridDim dim3(1, 1, 1)

template<typename Kernel>
void launch_kernel(Kernel kernel, dim3 grid, dim3 block, float* args...) {
    std::cout << "Launching kernel with grid(" << grid.x << "," << grid.y << "," << grid.z 
              << ") block(" << block.x << "," << block.y << "," << block.z << ")" << std::endl;
}
#endif

// =============================================================================
// BASIC CUDA KERNELS
// =============================================================================

__global__ void vector_add(float* a, float* b, float* c, int n) {
    // Mock kernel execution
    std::cout << "Vector addition kernel executing..." << std::endl;
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

__global__ void vector_scale(float* data, float scale, int n) {
    std::cout << "Vector scaling kernel executing..." << std::endl;
    for (int i = 0; i < n; ++i) {
        data[i] *= scale;
    }
}

void demonstrate_basic_kernels() {
    std::cout << "\n=== CUDA Basic Kernels ===" << std::endl;
    
    const int N = 1024;
    const size_t size = N * sizeof(float);
    
    // Host data
    std::vector<float> h_a(N), h_b(N), h_c(N);
    
    // Initialize host data
    std::cout << "\n1. Initializing Host Data:" << std::endl;
    for (int i = 0; i < N; ++i) {
        h_a[i] = static_cast<float>(i);
        h_b[i] = static_cast<float>(i * 2);
    }
    
    // Device data
    std::cout << "\n2. Allocating GPU Memory:" << std::endl;
    float *d_a, *d_b, *d_c;
    cudaMalloc(&d_a, size);
    cudaMalloc(&d_b, size);
    cudaMalloc(&d_c, size);
    
    // Copy to device
    std::cout << "\n3. Copying Data to GPU:" << std::endl;
    cudaMemcpy(d_a, h_a.data(), size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, h_b.data(), size, cudaMemcpyHostToDevice);
    
    // Launch kernel
    std::cout << "\n4. Launching Vector Addition Kernel:" << std::endl;
    dim3 block(256);
    dim3 grid((N + block.x - 1) / block.x);
    
    auto start = std::chrono::high_resolution_clock::now();
    vector_add<<<grid, block>>>(d_a, d_b, d_c, N);
    cudaDeviceSynchronize();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Kernel execution time: " << duration.count() << " microseconds" << std::endl;
    
    // Copy result back
    std::cout << "\n5. Copying Result from GPU:" << std::endl;
    cudaMemcpy(h_c.data(), d_c, size, cudaMemcpyDeviceToHost);
    
    // Verify results
    std::cout << "First 10 results: ";
    for (int i = 0; i < 10; ++i) {
        std::cout << h_c[i] << " ";
    }
    std::cout << std::endl;
    
    // Cleanup
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);
}

// =============================================================================
// MATRIX OPERATIONS ON GPU
// =============================================================================

__global__ void matrix_multiply(float* A, float* B, float* C, int m, int n, int k) {
    std::cout << "Matrix multiplication kernel executing..." << std::endl;
    
    // Simplified matrix multiplication
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            float sum = 0.0f;
            for (int l = 0; l < k; ++l) {
                sum += A[i * k + l] * B[l * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}

__global__ void matrix_transpose(float* input, float* output, int rows, int cols) {
    std::cout << "Matrix transpose kernel executing..." << std::endl;
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            output[j * rows + i] = input[i * cols + j];
        }
    }
}

void demonstrate_matrix_operations() {
    std::cout << "\n=== CUDA Matrix Operations ===" << std::endl;
    
    const int M = 512, N = 512, K = 512;
    const size_t size_A = M * K * sizeof(float);
    const size_t size_B = K * N * sizeof(float);
    const size_t size_C = M * N * sizeof(float);
    
    // Host matrices
    std::cout << "\n1. Initializing Matrices:" << std::endl;
    std::vector<float> h_A(M * K), h_B(K * N), h_C(M * N);
    
    // Initialize with random values
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    
    for (int i = 0; i < M * K; ++i) h_A[i] = dis(gen);
    for (int i = 0; i < K * N; ++i) h_B[i] = dis(gen);
    
    std::cout << "Matrix A: " << M << "x" << K << std::endl;
    std::cout << "Matrix B: " << K << "x" << N << std::endl;
    
    // Device matrices
    std::cout << "\n2. GPU Memory Allocation:" << std::endl;
    float *d_A, *d_B, *d_C;
    cudaMalloc(&d_A, size_A);
    cudaMalloc(&d_B, size_B);
    cudaMalloc(&d_C, size_C);
    
    // Copy to device
    cudaMemcpy(d_A, h_A.data(), size_A, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B.data(), size_B, cudaMemcpyHostToDevice);
    
    // Launch matrix multiplication
    std::cout << "\n3. Matrix Multiplication on GPU:" << std::endl;
    dim3 block(16, 16);
    dim3 grid((N + block.x - 1) / block.x, (M + block.y - 1) / block.y);
    
    auto start = std::chrono::high_resolution_clock::now();
    matrix_multiply<<<grid, block>>>(d_A, d_B, d_C, M, N, K);
    cudaDeviceSynchronize();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Matrix multiplication time: " << duration.count() << " ms" << std::endl;
    
    // Calculate FLOPS
    double flops = 2.0 * M * N * K;
    double gflops = flops / (duration.count() / 1000.0) / 1e9;
    std::cout << "Performance: " << gflops << " GFLOPS" << std::endl;
    
    // Copy result back
    cudaMemcpy(h_C.data(), d_C, size_C, cudaMemcpyDeviceToHost);
    
    std::cout << "Result matrix C: " << M << "x" << N << std::endl;
    
    // Cleanup
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
}

// =============================================================================
// NEURAL NETWORK KERNELS
// =============================================================================

__global__ void relu_activation(float* data, int n) {
    std::cout << "ReLU activation kernel executing..." << std::endl;
    for (int i = 0; i < n; ++i) {
        data[i] = fmaxf(0.0f, data[i]);
    }
}

__global__ void sigmoid_activation(float* data, int n) {
    std::cout << "Sigmoid activation kernel executing..." << std::endl;
    for (int i = 0; i < n; ++i) {
        data[i] = 1.0f / (1.0f + expf(-data[i]));
    }
}

__global__ void softmax_kernel(float* input, float* output, int n) {
    std::cout << "Softmax kernel executing..." << std::endl;
    
    // Find maximum
    float max_val = input[0];
    for (int i = 1; i < n; ++i) {
        max_val = fmaxf(max_val, input[i]);
    }
    
    // Compute exp and sum
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        output[i] = expf(input[i] - max_val);
        sum += output[i];
    }
    
    // Normalize
    for (int i = 0; i < n; ++i) {
        output[i] /= sum;
    }
}

class CudaNeuralNetwork {
private:
    float *d_weights1_, *d_weights2_;
    float *d_bias1_, *d_bias2_;
    float *d_hidden_, *d_output_;
    
    int input_size_, hidden_size_, output_size_;
    
public:
    CudaNeuralNetwork(int input_size, int hidden_size, int output_size)
        : input_size_(input_size), hidden_size_(hidden_size), output_size_(output_size) {
        
        std::cout << "Creating CUDA Neural Network: " << input_size 
                  << " -> " << hidden_size << " -> " << output_size << std::endl;
        
        // Allocate GPU memory
        cudaMalloc(&d_weights1_, input_size * hidden_size * sizeof(float));
        cudaMalloc(&d_weights2_, hidden_size * output_size * sizeof(float));
        cudaMalloc(&d_bias1_, hidden_size * sizeof(float));
        cudaMalloc(&d_bias2_, output_size * sizeof(float));
        cudaMalloc(&d_hidden_, hidden_size * sizeof(float));
        cudaMalloc(&d_output_, output_size * sizeof(float));
        
        initialize_weights();
    }
    
    ~CudaNeuralNetwork() {
        cudaFree(d_weights1_);
        cudaFree(d_weights2_);
        cudaFree(d_bias1_);
        cudaFree(d_bias2_);
        cudaFree(d_hidden_);
        cudaFree(d_output_);
    }
    
    void initialize_weights() {
        std::cout << "Initializing network weights on GPU..." << std::endl;
        
        // Initialize weights on host then copy to device
        std::vector<float> h_weights1(input_size_ * hidden_size_);
        std::vector<float> h_weights2(hidden_size_ * output_size_);
        std::vector<float> h_bias1(hidden_size_);
        std::vector<float> h_bias2(output_size_);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-0.1, 0.1);
        
        for (auto& w : h_weights1) w = dis(gen);
        for (auto& w : h_weights2) w = dis(gen);
        for (auto& b : h_bias1) b = dis(gen);
        for (auto& b : h_bias2) b = dis(gen);
        
        cudaMemcpy(d_weights1_, h_weights1.data(), h_weights1.size() * sizeof(float), cudaMemcpyHostToDevice);
        cudaMemcpy(d_weights2_, h_weights2.data(), h_weights2.size() * sizeof(float), cudaMemcpyHostToDevice);
        cudaMemcpy(d_bias1_, h_bias1.data(), h_bias1.size() * sizeof(float), cudaMemcpyHostToDevice);
        cudaMemcpy(d_bias2_, h_bias2.data(), h_bias2.size() * sizeof(float), cudaMemcpyHostToDevice);
    }
    
    std::vector<float> forward(const std::vector<float>& input) {
        std::cout << "Forward pass on GPU..." << std::endl;
        
        // Copy input to device
        float* d_input;
        cudaMalloc(&d_input, input_size_ * sizeof(float));
        cudaMemcpy(d_input, input.data(), input_size_ * sizeof(float), cudaMemcpyHostToDevice);
        
        // Layer 1: input -> hidden
        dim3 block(256);
        dim3 grid((hidden_size_ + block.x - 1) / block.x);
        
        // Matrix multiplication + bias (simplified)
        matrix_multiply<<<grid, block>>>(d_weights1_, d_input, d_hidden_, hidden_size_, 1, input_size_);
        
        // ReLU activation
        relu_activation<<<grid, block>>>(d_hidden_, hidden_size_);
        
        // Layer 2: hidden -> output
        grid = dim3((output_size_ + block.x - 1) / block.x);
        matrix_multiply<<<grid, block>>>(d_weights2_, d_hidden_, d_output_, output_size_, 1, hidden_size_);
        
        // Softmax activation
        softmax_kernel<<<1, 1>>>(d_output_, d_output_, output_size_);
        
        cudaDeviceSynchronize();
        
        // Copy result back to host
        std::vector<float> output(output_size_);
        cudaMemcpy(output.data(), d_output_, output_size_ * sizeof(float), cudaMemcpyDeviceToHost);
        
        cudaFree(d_input);
        return output;
    }
};

void demonstrate_neural_network() {
    std::cout << "\n=== CUDA Neural Network ===" << std::endl;
    
    // Create network
    CudaNeuralNetwork network(784, 256, 10);
    
    // Test forward pass
    std::cout << "\n1. Testing Forward Pass:" << std::endl;
    std::vector<float> input(784);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    for (auto& val : input) {
        val = dis(gen);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    auto output = network.forward(input);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Forward pass time: " << duration.count() << " microseconds" << std::endl;
    
    std::cout << "Output probabilities: ";
    for (size_t i = 0; i < std::min(output.size(), size_t(10)); ++i) {
        std::cout << std::fixed << std::setprecision(4) << output[i] << " ";
    }
    std::cout << std::endl;
}

// =============================================================================
// ADVANCED CUDA FEATURES
// =============================================================================

void demonstrate_advanced_features() {
    std::cout << "\n=== Advanced CUDA Features ===" << std::endl;
    
    std::cout << "\n1. Shared Memory:" << std::endl;
    std::cout << "• Use __shared__ memory for fast inter-thread communication" << std::endl;
    std::cout << "• Implement tiled matrix multiplication for better performance" << std::endl;
    std::cout << "• Reduce global memory accesses with shared memory caching" << std::endl;
    
    std::cout << "\n2. Memory Coalescing:" << std::endl;
    std::cout << "• Ensure contiguous memory access patterns" << std::endl;
    std::cout << "• Use structure-of-arrays instead of array-of-structures" << std::endl;
    std::cout << "• Optimize memory bandwidth utilization" << std::endl;
    
    std::cout << "\n3. Streams and Concurrency:" << std::endl;
    std::cout << "• Use CUDA streams for overlapping computation and data transfer" << std::endl;
    std::cout << "• Implement asynchronous memory operations" << std::endl;
    std::cout << "• Utilize multiple GPUs with peer-to-peer communication" << std::endl;
    
    std::cout << "\n4. cuDNN Integration:" << std::endl;
    std::cout << "• Use cuDNN for optimized deep learning primitives" << std::endl;
    std::cout << "• Implement convolution, pooling, and normalization layers" << std::endl;
    std::cout << "• Leverage Tensor Cores for mixed-precision training" << std::endl;
}

int main() {
    std::cout << "CUDA C++ FOR AI AND GPU COMPUTING\n";
    std::cout << "==================================\n";
    
    demonstrate_basic_kernels();
    demonstrate_matrix_operations();
    demonstrate_neural_network();
    demonstrate_advanced_features();
    
    std::cout << "\nKey CUDA Concepts:" << std::endl;
    std::cout << "• Massive parallelism with thousands of threads" << std::endl;
    std::cout << "• Memory hierarchy optimization crucial for performance" << std::endl;
    std::cout << "• Kernel design patterns for efficient GPU utilization" << std::endl;
    std::cout << "• Integration with AI frameworks like cuDNN and TensorRT" << std::endl;
    std::cout << "• Asynchronous execution for overlapping operations" << std::endl;
    
    return 0;
}
