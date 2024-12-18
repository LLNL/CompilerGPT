
#include "constants.h"
#include <algorithm>

namespace {

// Force inline for this small, frequently-called function
__attribute__((always_inline))
inline int matchMissmatchScore(const char a_char, const char b_char) {
    return (a_char == b_char) ? matchScore : missmatchScore;
}

// Optimize the core computation function
__attribute__((always_inline))
inline void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos)
{
    // Pre-calculate index to avoid repeated computation
    const long long int index = m * i + j;
    const long long int idx_up = index - m;
    
    // Load all memory values at once to help with cache prefetching
    const int score_up = H[idx_up];
    const int score_left = H[index - 1];
    const int score_diag = H[idx_up - 1];
    const int current_max = H[*maxPos];
    
    // Calculate all scores
    const int up = score_up + gapScore;
    const int left = score_left + gapScore;
    const int diag = score_diag + matchMissmatchScore(a[j-1], b[i-1]);

    // Initialize max and pred with diagonal values to reduce branches
    int max = diag;
    int pred = DIAGONAL;
    
    // Use sequential comparison which is typically faster than branching
    if (up > max) {
        max = up;
        pred = UP;
    }
    if (left > max) {
        max = left;
        pred = LEFT;
    }
    
    // Ensure max is at least NONE
    max = (max > NONE) ? max : NONE;

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maxPos if needed
    if (max > current_max) {
        *maxPos = index;
    }
}

}  // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Optimize for cache line size (typical 64 bytes = 16 ints)
    constexpr int BLOCK_SIZE = 64;
    
    // Pre-fetch first row of data
    __builtin_prefetch(H, 0, 3);
    __builtin_prefetch(P, 1, 3);

    // Process blocks for better cache utilization
    for (long long int ii = 1; ii < n; ii += BLOCK_SIZE) {
        const long long int i_end = std::min(ii + BLOCK_SIZE, n);
        
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int j_end = std::min(jj + BLOCK_SIZE, m);
            
            // Process block with loop unrolling when possible
            for (long long int i = ii; i < i_end; ++i) {
                // Pre-fetch next row
                __builtin_prefetch(&H[m * (i + 1)], 0, 3);
                __builtin_prefetch(&P[m * (i + 1)], 1, 3);
                
                // Main loop with manual unrolling for small blocks
                long long int j = jj;
                for (; j + 4 <= j_end; j += 4) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                    similarityScore(a, b, i, j+1, m, H, P, &max);
                    similarityScore(a, b, i, j+2, m, H, P, &max);
                    similarityScore(a, b, i, j+3, m, H, P, &max);
                }
                
                // Handle remaining elements
                for (; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
