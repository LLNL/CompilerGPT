
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a_char, char b_char) {
    return (a_char == b_char) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos,
                    int* __restrict__ curr_max_val) {
    // Pre-calculate all indices at once to help vectorization
    const long long int index = m * i + j;
    const int up = H[index - m] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[index - m - 1] + matchMissmatchScore(a[j-1], b[i-1]);

    // Branchless maximum calculation
    int max, pred;
    
    // First comparison: diag vs NONE
    max = (diag > NONE) ? diag : NONE;
    pred = (diag > NONE) ? DIAGONAL : NONE;
    
    // Compare with up
    const bool is_up_better = up > max;
    max = is_up_better ? up : max;
    pred = is_up_better ? UP : pred;
    
    // Compare with left
    const bool is_left_better = left > max;
    max = is_left_better ? left : max;
    pred = is_left_better ? LEFT : pred;

    // Store results
    H[index] = max;
    P[index] = pred;

    // Branchless maximum update
    const bool is_new_max = max > *curr_max_val;
    *curr_max_val = is_new_max ? max : *curr_max_val;
    *maxPos = is_new_max ? index : *maxPos;
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max) {
    // Local variables to reduce memory access and register pressure
    int curr_max_val = H[max];
    const long long int row_size = m;
    const long long int n_bound = n;
    const long long int m_bound = m;
    
    // Unroll factor for the inner loop
    constexpr int UNROLL = 4;
    
    for (long long int i = 1; i < n_bound; i++) {
        // Main loop with manual unrolling
        long long int j;
        for (j = 1; j < m_bound - (UNROLL-1); j += UNROLL) {
            similarityScore(a, b, i, j, row_size, H, P, &max, &curr_max_val);
            similarityScore(a, b, i, j+1, row_size, H, P, &max, &curr_max_val);
            similarityScore(a, b, i, j+2, row_size, H, P, &max, &curr_max_val);
            similarityScore(a, b, i, j+3, row_size, H, P, &max, &curr_max_val);
        }
        
        // Handle remaining elements
        for (; j < m_bound; j++) {
            similarityScore(a, b, i, j, row_size, H, P, &max, &curr_max_val);
        }
    }
}
