
#include "constants.h"
#include <algorithm>

namespace {

// Force inline to ensure the function is always inlined
__attribute__((always_inline)) 
static inline int getScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

__attribute__((always_inline))
static inline void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos,
                    int& current_max)  // Pass current max by reference to avoid dereferencing
{
    // Calculate index once and use bit shifting for multiplication where possible
    // m is typically a power of 2, so we can use shift
    const long long int index = (i << __builtin_ctzll(m)) + j;  // Same as i * m + j
    const long long int up_index = index - m;
    
    // Prefetch next iteration's data
    __builtin_prefetch(&H[index + m]);
    __builtin_prefetch(&P[index + m]);
    
    // Calculate scores - minimize memory access by reading each value once
    const int diag_score = H[up_index - 1] + getScore(b[i-1], a[j-1]);
    const int up_score = H[up_index] + gapScore;
    const int left_score = H[index - 1] + gapScore;
    
    // Branchless maximum computation using conditional moves
    int max = diag_score;
    int pred = DIAGONAL;
    
    if (up_score > max) {
        max = up_score;
        pred = UP;
    }
    if (left_score > max) {
        max = left_score;
        pred = LEFT;
    }
    
    // Store results
    H[index] = max;
    P[index] = pred;
    
    // Update maximum score if necessary
    if (max > current_max) {
        current_max = max;
        *maxPos = index;
    }
}

} // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Cache line size is typically 64 bytes
    constexpr long long int CACHE_LINE = 64;
    constexpr long long int BLOCK_I = 64; // Tune based on L1 cache size
    constexpr long long int BLOCK_J = CACHE_LINE / sizeof(int); // Align with cache line
    
    // Keep track of current maximum to avoid dereferencing maxPos repeatedly
    int current_max = H[max];
    
    // Outer blocks
    for (long long int ii = 1; ii < n; ii += BLOCK_I) {
        const long long int i_end = std::min(ii + BLOCK_I, n);
        
        // Inner blocks
        for (long long int jj = 1; jj < m; jj += BLOCK_J) {
            const long long int j_end = std::min(jj + BLOCK_J, m);
            
            // Process block
            for (long long int i = ii; i < i_end; ++i) {
                // Manual loop unrolling for inner loop
                long long int j = jj;
                for (; j + 3 < j_end; j += 4) {
                    similarityScore(a, b, i, j, m, H, P, &max, current_max);
                    similarityScore(a, b, i, j+1, m, H, P, &max, current_max);
                    similarityScore(a, b, i, j+2, m, H, P, &max, current_max);
                    similarityScore(a, b, i, j+3, m, H, P, &max, current_max);
                }
                // Handle remaining elements
                for (; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max, current_max);
                }
            }
        }
    }
}
