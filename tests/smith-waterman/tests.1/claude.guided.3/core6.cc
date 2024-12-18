
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
    // Calculate indices - keep it simple
    const long long int index = m * i + j;
    
    // Load values - group loads together
    const int score_up = H[index - m];
    const int score_left = H[index - 1];
    const int score_diag = H[index - m - 1];
    
    // Calculate scores
    const int up = score_up + gapScore;
    const int left = score_left + gapScore;
    const int diag = score_diag + matchMissmatchScore(a[j-1], b[i-1]);

    // Simple max calculation with minimal branching
    int max = NONE;
    int pred = NONE;
    
    if (diag > max) {
        max = diag;
        pred = DIAGONAL;
    }
    if (up > max) {
        max = up;
        pred = UP;
    }
    if (left > max) {
        max = left;
        pred = LEFT;
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum if needed
    if (max > *curr_max_val) {
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
    
    // Simple nested loops without unrolling
    for (long long int i = 1; i < n; i++) {
        for (long long int j = 1; j < m; j++) {
            similarityScore(a, b, i, j, m, H, P, &max, &curr_max_val);
        }
    }
}
