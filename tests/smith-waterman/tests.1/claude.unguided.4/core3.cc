
#include "constants.h"

namespace {

// Pre-computed match/mismatch lookup table - aligned for better memory access
alignas(64) static const int score_table[2] = {missmatchScore, matchScore};

// Force inline to eliminate function call overhead
__attribute__((always_inline)) 
inline int matchMissmatchScore(const char a, const char b) {
    return score_table[a == b];
}

__attribute__((always_inline))
inline void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos) {
    // Pre-calculate base index once
    const long long int index = m * i + j;
    const long long int idx_up = index - m;
    
    // Load values into registers
    const int up = H[idx_up] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[idx_up - 1] + matchMissmatchScore(b[i-1], a[j-1]);

    // Optimized max calculation using branchless operations
    int max = (diag > NONE) ? diag : NONE;
    int pred = (diag > NONE) ? DIAGONAL : NONE;
    
    const int max_temp = (up > max) ? up : max;
    const int pred_temp = (up > max) ? UP : pred;
    
    max = (left > max_temp) ? left : max_temp;
    pred = (left > max_temp) ? LEFT : pred_temp;

    // Store results
    H[index] = max;
    P[index] = pred;

    // Branchless maximum position update
    const bool is_new_max = max > H[*maxPos];
    *maxPos = is_new_max ? index : *maxPos;
}

} // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max) {
    // Optimize for cache line size and vector registers
    constexpr long long int BLOCK_SIZE = 64 / sizeof(int);
    
    // Loop unrolling factor
    constexpr int UNROLL = 4;
    
    // Main computation loop with blocking and partial unrolling
    for (long long int i = 1; i < n; ++i) {
        // Process blocks for better cache utilization
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int j_end = (jj + BLOCK_SIZE < m) ? jj + BLOCK_SIZE : m;
            
            // Ensure we have enough iterations for unrolling
            long long int j;
            for (j = jj; j + UNROLL <= j_end; j += UNROLL) {
                // Manual loop unrolling
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
