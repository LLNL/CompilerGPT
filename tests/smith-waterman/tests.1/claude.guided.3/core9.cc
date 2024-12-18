
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
    // Calculate base index once
    const long long int index = m * i + j;
    const long long int row_offset = m;
    
    // Calculate match score first to separate memory operations
    const int match = matchMissmatchScore(a[j-1], b[i-1]);
    
    // Group memory reads together
    const int diag_score = H[index - row_offset - 1];
    const int up_score = H[index - row_offset];
    const int left_score = H[index - 1];
    
    // Calculate all scores before comparisons
    const int score_diag = diag_score + match;
    const int score_up = up_score + gapScore;
    const int score_left = left_score + gapScore;
    
    // Independent max calculation with minimal branching
    int max, pred;
    
    // Find highest score and corresponding direction
    if (score_diag >= score_up && score_diag >= score_left && score_diag > NONE) {
        max = score_diag;
        pred = DIAGONAL;
    } 
    else if (score_up >= score_left && score_up > NONE) {
        max = score_up;
        pred = UP;
    }
    else if (score_left > NONE) {
        max = score_left;
        pred = LEFT;
    }
    else {
        max = NONE;
        pred = NONE;
    }

    // Write results
    H[index] = max;
    P[index] = pred;

    // Single comparison for maximum update
    if (max > *curr_max_val) {
        *curr_max_val = max;
        *maxPos = index;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max) {
    // Local variable for maximum tracking
    int curr_max_val = H[max];
    
    // Main computation loop
    const long long int row_bound = n;
    const long long int col_bound = m;
    
    for (long long int i = 1; i < row_bound; i++) {
        // Hint for the compiler about alignment and dependencies
        const char* const row_a = a;
        const char* const row_b = &b[i-1];
        
        for (long long int j = 1; j < col_bound; j++) {
            similarityScore(row_a, row_b, i, j, m, H, P, &max, &curr_max_val);
        }
    }
}
