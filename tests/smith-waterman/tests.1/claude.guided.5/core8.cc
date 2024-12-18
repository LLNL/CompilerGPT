
#include "constants.h"
#include <algorithm>

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

// Simpler function focusing on core computation
void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j,
                    const long long int m,
                    const int* __restrict__ H_prev_row,
                    const int* __restrict__ H_curr_row,
                    int* __restrict__ H_out,
                    int* __restrict__ P_out)
{
    // Load all required values first
    const int up = H_curr_row[j] + gapScore;
    const int left = H_out[j-1] + gapScore;
    const int diag = H_prev_row[j-1] + matchMissmatchScore(a, b, i, j);
    
    // Initial max computation
    int score = diag;
    int pred = DIAGONAL;
    
    // Simple max comparisons
    if (up > score) {
        score = up;
        pred = UP;
    }
    if (left > score) {
        score = left;
        pred = LEFT;
    }
    
    // Handle NONE case
    if (score <= NONE) {
        score = NONE;
        pred = NONE;
    }
    
    // Store results
    H_out[j] = score;
    P_out[j] = pred;
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            long long int n, long long int m, long long int& max)
{
    // Track maximum score
    int max_score = H[max];
    long long int max_pos = max;
    
    // Process matrix row by row
    for (long long int i = 1; i < n; ++i) {
        const long long int curr_row = i * m;
        const long long int prev_row = (i-1) * m;
        
        // Process each row in blocks
        constexpr long long int BLOCK_SIZE = 32;
        
        #pragma clang loop vectorize(enable) interleave(enable)
        for (long long int j = 1; j < m; ++j) {
            // Compute similarity score
            similarityScore(a, b, i, j, m,
                          &H[prev_row], &H[curr_row], 
                          &H[curr_row], &P[curr_row]);
            
            // Update maximum if necessary
            const int curr_score = H[curr_row + j];
            if (curr_score > max_score) {
                max_score = curr_score;
                max_pos = curr_row + j;
            }
        }
    }
    
    // Update final maximum position
    max = max_pos;
}
