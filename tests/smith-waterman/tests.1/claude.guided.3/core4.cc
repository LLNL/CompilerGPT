
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

    // Find maximum 
    int max = diag;
    int pred = DIAGONAL;
    
    if (up > max) {
        max = up;
        pred = UP;
    }
    
    if (left > max) {
        max = left;
        pred = LEFT;
    }

    if (max <= NONE) {
        max = NONE;
        pred = NONE;
    }

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
    // Track current maximum value to reduce memory access
    int curr_max_val = H[max];
    
    // Main computation loops
    for (long long int i = 1; i < n; i++) {
        for (long long int j = 1; j < m; j++) {
            similarityScore(a, b, i, j, m, H, P, &max, &curr_max_val);
        }
    }
}
