
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
    // Pre-calculate index to avoid repeated computation
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;
    
    // Calculate scores
    const int up = H[index_up] + gapScore;
    const int left = H[index_left] + gapScore;
    const int diag = H[index_diag] + matchMissmatchScore(a[j-1], b[i-1]);

    // Initialize max and pred
    int max = NONE;
    int pred = NONE;
    
    // Use temporary variables to help vectorization
    const bool diag_greater = diag > max;
    const bool up_greater = up > max;
    const bool left_greater = left > max;
    
    // Update max and pred based on conditions
    max = diag_greater ? diag : max;
    pred = diag_greater ? DIAGONAL : pred;
    
    max = up_greater ? up : max;
    pred = up_greater ? UP : pred;
    
    max = left_greater ? left : max;
    pred = left_greater ? LEFT : pred;

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum score if necessary
    if (max > *curr_max_val) {
        *curr_max_val = max;
        *maxPos = index;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max) {
    // Pre-calculate loop bounds
    const long long int n_bound = n;
    const long long int m_bound = m;
    
    // Track current maximum value to reduce memory access
    int curr_max_val = H[max];
    
    // Main computation loops
    #ifdef __clang__
    #pragma clang loop vectorize(enable) interleave(enable)
    #endif
    for (long long int i = 1; i < n_bound; i++) {
        #ifdef __clang__
        #pragma clang loop vectorize(enable) interleave(enable)
        #endif
        for (long long int j = 1; j < m_bound; j++) {
            similarityScore(a, b, i, j, m, H, P, &max, &curr_max_val);
        }
    }
    
    // Update final maximum value if needed
    if (H[max] < curr_max_val) {
        H[max] = curr_max_val;
    }
}
