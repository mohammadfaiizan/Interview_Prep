/*
 * C++ FOR AI AND GPU - EIGEN C++ LIBRARY
 * 
 * Comprehensive guide to Eigen library for linear algebra operations
 * commonly used in AI/ML applications.
 * 
 * Compilation: g++ -std=c++17 -Wall -Wextra -I/path/to/eigen3 eigen_cpp_example.cpp -o eigen_demo
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <random>

// Mock Eigen library for demonstration
namespace Eigen {
    // Matrix types
    template<typename Scalar, int Rows, int Cols>
    class Matrix {
    private:
        std::vector<Scalar> data_;
        int rows_, cols_;
        
    public:
        Matrix(int rows = Rows, int cols = Cols) : rows_(rows), cols_(cols) {
            data_.resize(rows_ * cols_, Scalar(0));
        }
        
        Scalar& operator()(int row, int col) { return data_[row * cols_ + col]; }
        const Scalar& operator()(int row, int col) const { return data_[row * cols_ + col]; }
        
        int rows() const { return rows_; }
        int cols() const { return cols_; }
        
        Matrix operator+(const Matrix& other) const {
            Matrix result(rows_, cols_);
            for (int i = 0; i < rows_ * cols_; ++i) {
                result.data_[i] = data_[i] + other.data_[i];
            }
            return result;
        }
        
        Matrix operator*(const Matrix& other) const {
            Matrix result(rows_, other.cols_);
            for (int i = 0; i < rows_; ++i) {
                for (int j = 0; j < other.cols_; ++j) {
                    Scalar sum = 0;
                    for (int k = 0; k < cols_; ++k) {
                        sum += (*this)(i, k) * other(k, j);
                    }
                    result(i, j) = sum;
                }
            }
            return result;
        }
        
        void setRandom() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(-1.0, 1.0);
            for (auto& val : data_) {
                val = dis(gen);
            }
        }
        
        void setZero() { std::fill(data_.begin(), data_.end(), Scalar(0)); }
        void setOnes() { std::fill(data_.begin(), data_.end(), Scalar(1)); }
        
        void print() const {
            for (int i = 0; i < rows_; ++i) {
                for (int j = 0; j < cols_; ++j) {
                    std::cout << (*this)(i, j) << " ";
                }
                std::cout << std::endl;
            }
        }
    };
    
    // Common matrix types
    using MatrixXd = Matrix<double, -1, -1>;
    using MatrixXf = Matrix<float, -1, -1>;
    using Matrix3d = Matrix<double, 3, 3>;
    using Matrix4f = Matrix<float, 4, 4>;
    
    // Vector types
    template<typename Scalar, int Size>
    using Vector = Matrix<Scalar, Size, 1>;
    
    using VectorXd = Vector<double, -1>;
    using VectorXf = Vector<float, -1>;
    using Vector3d = Vector<double, 3>;
    using Vector4f = Vector<float, 4>;
}

// =============================================================================
// BASIC MATRIX OPERATIONS
// =============================================================================

void demonstrate_basic_operations() {
    std::cout << "\n=== Eigen Basic Matrix Operations ===" << std::endl;
    
    // Matrix creation
    std::cout << "\n1. Matrix Creation:" << std::endl;
    Eigen::MatrixXd A(3, 3);
    Eigen::MatrixXd B(3, 3);
    
    A.setRandom();
    B.setOnes();
    
    std::cout << "Matrix A (random):" << std::endl;
    A.print();
    
    std::cout << "Matrix B (ones):" << std::endl;
    B.print();
    
    // Basic operations
    std::cout << "\n2. Basic Operations:" << std::endl;
    auto C = A + B;
    auto D = A * B;
    
    std::cout << "A + B:" << std::endl;
    C.print();
    
    std::cout << "A * B:" << std::endl;
    D.print();
}

// =============================================================================
// NEURAL NETWORK LAYER IMPLEMENTATION
// =============================================================================

class DenseLayer {
private:
    Eigen::MatrixXf weights_;
    Eigen::VectorXf biases_;
    int input_size_;
    int output_size_;
    
public:
    DenseLayer(int input_size, int output_size) 
        : input_size_(input_size), output_size_(output_size),
          weights_(output_size, input_size), biases_(output_size, 1) {
        
        weights_.setRandom();
        biases_.setZero();
        
        std::cout << "Created Dense Layer: " << input_size << " -> " << output_size << std::endl;
    }
    
    Eigen::MatrixXf forward(const Eigen::MatrixXf& input) {
        std::cout << "Dense layer forward pass" << std::endl;
        return weights_ * input; // Simplified - should add biases
    }
    
    void print_weights() const {
        std::cout << "Weights:" << std::endl;
        weights_.print();
    }
};

class ActivationLayer {
public:
    static Eigen::MatrixXf relu(const Eigen::MatrixXf& input) {
        std::cout << "ReLU activation" << std::endl;
        Eigen::MatrixXf output(input.rows(), input.cols());
        for (int i = 0; i < input.rows(); ++i) {
            for (int j = 0; j < input.cols(); ++j) {
                output(i, j) = std::max(0.0f, input(i, j));
            }
        }
        return output;
    }
    
    static Eigen::MatrixXf sigmoid(const Eigen::MatrixXf& input) {
        std::cout << "Sigmoid activation" << std::endl;
        Eigen::MatrixXf output(input.rows(), input.cols());
        for (int i = 0; i < input.rows(); ++i) {
            for (int j = 0; j < input.cols(); ++j) {
                output(i, j) = 1.0f / (1.0f + std::exp(-input(i, j)));
            }
        }
        return output;
    }
};

void demonstrate_neural_network() {
    std::cout << "\n=== Neural Network with Eigen ===" << std::endl;
    
    // Create network layers
    std::cout << "\n1. Creating Network:" << std::endl;
    DenseLayer layer1(784, 128);
    DenseLayer layer2(128, 64);
    DenseLayer layer3(64, 10);
    
    // Forward pass
    std::cout << "\n2. Forward Pass:" << std::endl;
    Eigen::MatrixXf input(784, 1);
    input.setRandom();
    
    std::cout << "Input shape: " << input.rows() << "x" << input.cols() << std::endl;
    
    auto h1 = ActivationLayer::relu(layer1.forward(input));
    auto h2 = ActivationLayer::relu(layer2.forward(h1));
    auto output = layer3.forward(h2);
    
    std::cout << "Output shape: " << output.rows() << "x" << output.cols() << std::endl;
    std::cout << "Network output:" << std::endl;
    output.print();
}

// =============================================================================
// CONVOLUTIONAL OPERATIONS
// =============================================================================

class ConvolutionLayer {
private:
    Eigen::MatrixXf kernel_;
    int kernel_size_;
    
public:
    ConvolutionLayer(int kernel_size) : kernel_size_(kernel_size), kernel_(kernel_size, kernel_size) {
        kernel_.setRandom();
        std::cout << "Created Convolution Layer with kernel size: " << kernel_size << std::endl;
    }
    
    Eigen::MatrixXf convolve(const Eigen::MatrixXf& input) {
        std::cout << "Performing convolution operation" << std::endl;
        
        int output_rows = input.rows() - kernel_size_ + 1;
        int output_cols = input.cols() - kernel_size_ + 1;
        
        Eigen::MatrixXf output(output_rows, output_cols);
        output.setZero();
        
        // Simplified convolution
        for (int i = 0; i < output_rows; ++i) {
            for (int j = 0; j < output_cols; ++j) {
                float sum = 0;
                for (int ki = 0; ki < kernel_size_; ++ki) {
                    for (int kj = 0; kj < kernel_size_; ++kj) {
                        sum += input(i + ki, j + kj) * kernel_(ki, kj);
                    }
                }
                output(i, j) = sum;
            }
        }
        
        return output;
    }
};

void demonstrate_convolution() {
    std::cout << "\n=== Convolution Operations ===" << std::endl;
    
    // Create input image
    std::cout << "\n1. Creating Input Image:" << std::endl;
    Eigen::MatrixXf image(28, 28);
    image.setRandom();
    
    std::cout << "Input image size: " << image.rows() << "x" << image.cols() << std::endl;
    
    // Apply convolution
    std::cout << "\n2. Applying Convolution:" << std::endl;
    ConvolutionLayer conv(3);
    auto feature_map = conv.convolve(image);
    
    std::cout << "Feature map size: " << feature_map.rows() << "x" << feature_map.cols() << std::endl;
    
    // Multiple filters
    std::cout << "\n3. Multiple Filters:" << std::endl;
    std::vector<ConvolutionLayer> filters;
    for (int i = 0; i < 32; ++i) {
        filters.emplace_back(3);
    }
    
    std::cout << "Created " << filters.size() << " convolutional filters" << std::endl;
    
    for (size_t i = 0; i < std::min(filters.size(), size_t(3)); ++i) {
        auto fm = filters[i].convolve(image);
        std::cout << "Filter " << i << " output: " << fm.rows() << "x" << fm.cols() << std::endl;
    }
}

// =============================================================================
// PERFORMANCE BENCHMARKING
// =============================================================================

void benchmark_matrix_operations() {
    std::cout << "\n=== Performance Benchmarking ===" << std::endl;
    
    const std::vector<int> sizes = {100, 500, 1000};
    
    for (int size : sizes) {
        std::cout << "\n--- Matrix Size: " << size << "x" << size << " ---" << std::endl;
        
        // Create matrices
        Eigen::MatrixXf A(size, size);
        Eigen::MatrixXf B(size, size);
        A.setRandom();
        B.setRandom();
        
        // Benchmark matrix multiplication
        auto start = std::chrono::high_resolution_clock::now();
        auto C = A * B;
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Matrix multiplication time: " << duration.count() << " ms" << std::endl;
        
        // Calculate FLOPS
        double flops = 2.0 * size * size * size; // Approximate FLOPS for matrix multiplication
        double gflops = flops / (duration.count() / 1000.0) / 1e9;
        
        std::cout << "Performance: " << gflops << " GFLOPS" << std::endl;
    }
}

// =============================================================================
// ADVANCED FEATURES
// =============================================================================

void demonstrate_advanced_features() {
    std::cout << "\n=== Advanced Eigen Features ===" << std::endl;
    
    std::cout << "\n1. SIMD Vectorization:" << std::endl;
    std::cout << "• Eigen automatically uses SIMD instructions (SSE, AVX)" << std::endl;
    std::cout << "• Compile with -march=native for optimal performance" << std::endl;
    std::cout << "• Use aligned memory allocation for best performance" << std::endl;
    
    std::cout << "\n2. Lazy Evaluation:" << std::endl;
    std::cout << "• Complex expressions are evaluated efficiently" << std::endl;
    std::cout << "• Temporary objects are minimized automatically" << std::endl;
    std::cout << "• Expression templates optimize computation chains" << std::endl;
    
    std::cout << "\n3. Block Operations:" << std::endl;
    std::cout << "• Access submatrices with .block() operations" << std::endl;
    std::cout << "• Efficient memory access patterns" << std::endl;
    std::cout << "• Support for in-place operations" << std::endl;
    
    std::cout << "\n4. Sparse Matrices:" << std::endl;
    std::cout << "• SparseMatrix class for memory-efficient storage" << std::endl;
    std::cout << "• Specialized algorithms for sparse linear algebra" << std::endl;
    std::cout << "• Integration with sparse solvers" << std::endl;
}

void demonstrate_cmake_integration() {
    std::cout << "\n=== CMake Integration ===" << std::endl;
    
    std::cout << R"(
# CMakeLists.txt for Eigen project
cmake_minimum_required(VERSION 3.16)
project(eigen_ai_project)

set(CMAKE_CXX_STANDARD 17)

# Find Eigen
find_package(Eigen3 REQUIRED)

# Alternative: Use FetchContent
# include(FetchContent)
# FetchContent_Declare(
#   Eigen
#   GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
#   GIT_TAG 3.4.0
# )
# FetchContent_MakeAvailable(Eigen)

# Create executable
add_executable(${PROJECT_NAME} main.cpp neural_net.cpp)

# Link Eigen (header-only)
target_link_libraries(${PROJECT_NAME} Eigen3::Eigen)

# Optimization flags
target_compile_options(${PROJECT_NAME} PRIVATE
    -O3
    -march=native
    -DEIGEN_NO_DEBUG
    -DNDEBUG
)
)" << std::endl;
}

int main() {
    std::cout << "EIGEN C++ LIBRARY FOR AI\n";
    std::cout << "========================\n";
    
    demonstrate_basic_operations();
    demonstrate_neural_network();
    demonstrate_convolution();
    benchmark_matrix_operations();
    demonstrate_advanced_features();
    demonstrate_cmake_integration();
    
    std::cout << "\nKey Eigen Concepts:" << std::endl;
    std::cout << "• Header-only library with excellent performance" << std::endl;
    std::cout << "• Automatic SIMD vectorization and optimization" << std::endl;
    std::cout << "• Expression templates for efficient computations" << std::endl;
    std::cout << "• Perfect for neural network implementations" << std::endl;
    std::cout << "• Extensive support for linear algebra operations" << std::endl;
    std::cout << "• Memory-efficient sparse matrix support" << std::endl;
    
    return 0;
}
