# LLMTools Example: Code Optimizer

This is a minimal example demonstrating how to use the LLMTools library to create a code optimization assistant. The example sends a matrix multiplication function to an LLM and asks for optimization suggestions.

## Prerequisites

- Boost libraries (JSON, Filesystem, Process, etc.)
- OpenAI API key
- CompilerGPT repository

## Building

To build the example:

```bash
make
```

## Running

To run the example:

```bash
# Set the required environment variables
export LLMTOOLS_PATH=/path/to/CompilerGPT
export OPENAI_API_KEY=your_api_key_here

# Run the example
make check
```

Or you can run it directly:

```bash
./code_optimizer.bin
```

## What it does

1. Sets up the LLMTools library with OpenAI as the provider
2. Creates a conversation with a system prompt
3. Adds a user prompt with a matrix multiplication function to optimize
4. Sends the query to the LLM
5. Displays the optimized code suggestion from the LLM

## Expected Output

```
Running llmtools example...
Make sure LLMTOOLS_PATH and OPENAI_API_KEY environment variables are set.
Querying LLM for optimization advice...

Optimized code suggestion:
To optimize the given matrix multiplication function for better cache locality, you can employ a technique called "loop tiling" or "loop blocking." This technique breaks down the matrices into smaller blocks or tiles that fit better into the cache. By doing so, you increase data reuse and reduce the number of cache misses. Here's how you can apply loop tiling to your matrix multiplication function:

```cpp
#include <algorithm>

// This example assumes n is a multiple of BLOCK_SIZE for simplicity.
// In practice, you should handle edge cases where n is not a multiple
// of BLOCK_SIZE by adjusting dimensions as necessary.
const int BLOCK_SIZE = 64; // Adjust BLOCK_SIZE depending on your system's cache size.

void matrix_multiply(double* A, double* B, double* C, int n) {
    for (int ii = 0; ii < n; ii += BLOCK_SIZE) {
        for (int jj = 0; jj < n; jj += BLOCK_SIZE) {
            // Initialize C blocks to zero
            for (int i = ii; i < std::min(ii + BLOCK_SIZE, n); i++) {
                for (int j = jj; j < std::min(jj + BLOCK_SIZE, n); j++) {
                    C[i * n + j] = 0;
                }
            }
            for (int kk = 0; kk < n; kk += BLOCK_SIZE) {
                for (int i = ii; i < std::min(ii + BLOCK_SIZE, n); i++) {
                    for (int j = jj; j < std::min(jj + BLOCK_SIZE, n); j++) {
                        double sum = C[i * n + j];
                        for (int k = kk; k < std::min(kk + BLOCK_SIZE, n); k++) {
                            sum += A[i * n + k] * B[k * n + j];
                        }
                        C[i * n + j] = sum;
                    }
                }
            }
        }
    }
}
```

### Key Points:

1. **Block Size**:
   - Choose a block size (`BLOCK_SIZE`) that allows a block of the matrix to fit into the CPU's cache. A typical block size might be 32, 64, or 128, but you should experiment with different sizes depending on your specific hardware characteristics.

2. **Initialization**:
   - The inner loops initialize the elements of `C` blocks to zero before starting the innermost multiplication.

3. **Edge Case Handling**:
   - The loop bounds are adjusted with `std::min()` to handle cases where `n` is not an exact multiple of `BLOCK_SIZE`. This ensures that the loops do not go out of the bounds of the matrix when `n` is not perfectly divisible by `BLOCK_SIZE`.

4. **Data Reuse**:
   - The tiling approach is intended to maximize the reuse of data that is already loaded in the cache, thus minimizing cache misses and improving performance.

Experimenting with different block sizes and testing this approach on your specific hardware will help ensure that you achieve the best performance gains.
```

Note: The actual output may vary depending on the LLM model and version used.

## Cleaning Up

To clean up generated files:

```bash
make clean
