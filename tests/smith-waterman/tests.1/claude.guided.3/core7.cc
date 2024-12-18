
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a_char, char b_char) {
    return (a_char == b_char) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, 
                    const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos,
                    int* __restrict__ curr_max_val) {
    // Pre-calculate indices
    const long long int index = m * i + j;
    const long long int diag_index = index - m - 1;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;
    
    // Single comparison for match/mismatch
    const int match = matchMissmatchScore(a[j-1], b[i-1]);
    
    // Load values in sequence for better pipelining
    const int score_diag = H[diag_index] + match;
    const int score_up = H[up_index] + gapScore;
    const int score_left = H[left_index] + gapScore;

    // Calculate max score using temporary variables to aid vectorization
    const int max1 = (score_diag > NONE) ? score_diag : NONE;
    const int max2 = (score_up > max1) ? score_up : max1;
    const int max = (score_left > max2) ? score_left : max2;

    // Calculate predecessor using the max value
    int pred;
    if (max == score_left) {
        pred = LEFT;
    } else if (max == score_up) {
        pred = UP;
    } else if (max == score_diag) {
        pred = DIAGONAL;
    } else {
        pred = NONE;
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum if needed using temporary to avoid multiple dereferences
    const int current_max = *curr_max_val;
    if (max > current_max) {
        *curr_max_val = max;
        *maxPos = index;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max) {
    // Track current maximum value
    int curr_max_val = H[max];
    
    // Align data access pattern for vectorization
    #ifdef __clang__
    #pragma clang loop vectorize(enable) interleave(enable) distribute(enable)
    #endif
    for (long long int i = 1; i < n; i++) {
        #ifdef __clang__
        #pragma clang loop vectorize(enable) interleave(enable)
        #endif
        for (long long int j = 1; j < m; j++) {
            similarityScore(a, b, i, j, m, H, P, &max, &curr_max_val);
        }
    }
}
