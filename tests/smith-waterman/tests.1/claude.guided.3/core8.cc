
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
    
    // Calculate scores in sequence to help instruction pipelining
    const char a_char = a[j-1];
    const char b_char = b[i-1];
    const int match = matchMissmatchScore(a_char, b_char);
    
    const int score_diag = H[diag_index] + match;
    const int score_up = H[up_index] + gapScore;
    const int score_left = H[left_index] + gapScore;

    // Calculate max score using sequential comparisons
    int max = score_diag;
    int pred = DIAGONAL;

    if (score_up > max) {
        max = score_up;
        pred = UP;
    }
    
    if (score_left > max) {
        max = score_left;
        pred = LEFT;
    }
    
    if (max <= NONE) {
        max = NONE;
        pred = NONE;
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum if needed
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
    
    // Simple nested loops with predictable access patterns
    for (long long int i = 1; i < n; i++) {
        for (long long int j = 1; j < m; j++) {
            similarityScore(a, b, i, j, m, H, P, &max, &curr_max_val);
        }
    }
}
