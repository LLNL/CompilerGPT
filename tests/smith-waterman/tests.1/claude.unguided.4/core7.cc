
#include "constants.h"

namespace {

// Pre-computed match/mismatch lookup table - aligned for better memory access
alignas(64) static const int score_table[2] = {missmatchScore, matchScore};

__attribute__((always_inline)) 
inline int matchMissmatchScore(const char a, const char b) {
    return score_table[a == b];
}

__attribute__((hot)) __attribute__((always_inline))
inline void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos,
                    int& localMax) {
    // Pre-calculate indices
    const long long int index = m * i + j;
    const long long int idx_up = index - m;
    const long long int idx_left = index - 1;
    const long long int idx_diag = idx_up - 1;
    
    // Load all required values at once to start memory fetching
    const int curr_up = H[idx_up];
    const int curr_left = H[idx_left];
    const int curr_diag = H[idx_diag];
    const char char_a = a[j-1];
    const char char_b = b[i-1];
    
    // Calculate scores using loaded values
    const int up = curr_up + gapScore;
    const int left = curr_left + gapScore;
    const int diag = curr_diag + matchMissmatchScore(char_b, char_a);

    // Efficient max calculation
    int max, pred;
    
    if (diag >= up) {
        if (diag >= left) {
            max = diag;
            pred = DIAGONAL;
        } else {
            max = left;
            pred = LEFT;
        }
    } else {
        if (up >= left) {
            max = up;
            pred = UP;
        } else {
            max = left;
            pred = LEFT;
        }
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update local maximum
    if (max > localMax) {
        localMax = max;
        *maxPos = index;
    }
}

} // namespace

__attribute__((hot))
void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max) {
    // Optimize for cache line size and typical SIMD width
    constexpr long long int BLOCK_SIZE = 32;
    constexpr long long int MINI_BLOCK = 8;
    
    // Track maximum locally to reduce memory dependencies
    int localMax = H[max];
    
    // Main computation loop with two-level blocking
    for (long long int i = 1; i < n; ++i) {
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int block_end = (jj + BLOCK_SIZE < m) ? jj + BLOCK_SIZE : m;
            
            // Process mini-blocks for better cache utilization
            for (long long int j = jj; j < block_end; j += MINI_BLOCK) {
                const long long int mini_end = (j + MINI_BLOCK < block_end) ? j + MINI_BLOCK : block_end;
                
                // Tight inner loop
                for (long long int k = j; k < mini_end; ++k) {
                    similarityScore(a, b, i, k, m, H, P, &max, localMax);
                }
            }
        }
    }
    
    // Update global maximum
    H[max] = localMax;
}
