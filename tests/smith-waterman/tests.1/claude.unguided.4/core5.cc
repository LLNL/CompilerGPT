
#include "constants.h"

namespace {

// Pre-computed match/mismatch lookup table - aligned for better memory access
alignas(64) static const int score_table[2] = {missmatchScore, matchScore};

// Prefetch distance tuned for common CPU architectures
constexpr int PREFETCH_DISTANCE = 8;

// Force inline to eliminate function call overhead
__attribute__((always_inline)) 
inline int matchMissmatchScore(const char a, const char b) {
    return score_table[a == b];
}

__attribute__((always_inline))
inline void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos,
                    int& current_max) {
    // Pre-calculate indices
    const long long int index = m * i + j;
    const long long int idx_up = index - m;
    
    // Prefetch next iterations data
    __builtin_prefetch(&H[index + PREFETCH_DISTANCE]);
    __builtin_prefetch(&P[index + PREFETCH_DISTANCE]);
    
    // Load values into local variables
    const int up = H[idx_up] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[idx_up - 1] + matchMissmatchScore(b[i-1], a[j-1]);

    // Optimized max calculation using branchless operations
    int max = diag;
    int pred = DIAGONAL;
    
    // Branchless max using bit manipulation
    const unsigned int up_mask = -(up > max);
    max = (up & up_mask) | (max & ~up_mask);
    pred = ((UP & up_mask) | (pred & ~up_mask));
    
    const unsigned int left_mask = -(left > max);
    max = (left & left_mask) | (max & ~left_mask);
    pred = ((LEFT & left_mask) | (pred & ~left_mask));

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum tracking
    const unsigned int max_mask = -(max > current_max);
    current_max = (max & max_mask) | (current_max & ~max_mask);
    *maxPos = (index & max_mask) | (*maxPos & ~max_mask);
}

} // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max) {
    // Optimize for cache line size and vector registers
    constexpr long long int BLOCK_SIZE = 64 / sizeof(int);
    
    // Loop unrolling factor - tuned for common architectures
    constexpr int UNROLL = 8;
    
    // Track current maximum score to avoid memory access
    int current_max = H[max];
    
    // Main computation loop with blocking and partial unrolling
    for (long long int i = 1; i < n; ++i) {
        // Process blocks for better cache utilization
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int j_end = (jj + BLOCK_SIZE < m) ? jj + BLOCK_SIZE : m;
            
            // Handle main loop with unrolling
            long long int j;
            for (j = jj; j + UNROLL <= j_end; j += UNROLL) {
                // Manual loop unrolling
                similarityScore(a, b, i, j, m, H, P, &max, current_max);
                similarityScore(a, b, i, j+1, m, H, P, &max, current_max);
                similarityScore(a, b, i, j+2, m, H, P, &max, current_max);
                similarityScore(a, b, i, j+3, m, H, P, &max, current_max);
                similarityScore(a, b, i, j+4, m, H, P, &max, current_max);
                similarityScore(a, b, i, j+5, m, H, P, &max, current_max);
                similarityScore(a, b, i, j+6, m, H, P, &max, current_max);
                similarityScore(a, b, i, j+7, m, H, P, &max, current_max);
            }
            
            // Handle remaining elements
            for (; j < j_end; ++j) {
                similarityScore(a, b, i, j, m, H, P, &max, current_max);
            }
        }
    }
    
    // Update final max score
    H[max] = current_max;
}
