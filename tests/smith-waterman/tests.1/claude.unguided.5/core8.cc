
#include "constants.h"

namespace {

// Use a lookup table for match/mismatch scores to eliminate branches
alignas(64) static const int SCORE_TABLE[2] = {missmatchScore, matchScore};

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    return SCORE_TABLE[a[j-1] == b[i-1]];
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, 
                    const long long int m,
                    int* __restrict__ H, 
                    int* __restrict__ P,
                    long long int* __restrict__ maxPos)
{
    // Pre-calculate indices and combine related calculations
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_diag = index_up - 1;
    
    // Group memory reads together for better cache utilization
    const int h_up = H[index_up];
    const int h_left = H[index - 1];
    const int h_diag = H[index_diag];
    const int current_max = H[*maxPos];
    
    // Calculate all scores at once
    const int match_score = matchMissmatchScore(a, b, i, j);
    const int up = h_up + gapScore;
    const int left = h_left + gapScore;
    const int diag = h_diag + match_score;

    // Optimized max calculation with fewer branches
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

    // Combine NONE check with max update
    max = (max >= NONE) ? max : NONE;
    pred = (max >= NONE) ? pred : NONE;

    // Store results - group writes together
    H[index] = max;
    P[index] = pred;

    // Update maxPos using conditional move pattern
    *maxPos = (max > current_max) ? index : *maxPos;
}

} // anonymous namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Constants for loop unrolling and vectorization
    constexpr long long int UNROLL = 8;
    constexpr long long int BLOCK_SIZE = 32;
    
    // Main computation loop with blocking and unrolling
    for (long long int i = 1; i < n; ++i) {
        // Block processing for better cache utilization
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int j_end = (jj + BLOCK_SIZE < m) ? jj + BLOCK_SIZE : m;
            
            // Unrolled loop processing
            long long int j = jj;
            for (; j + UNROLL <= j_end; j += UNROLL) {
                similarityScore(a, b, i, j, m, H, P, &max);
                similarityScore(a, b, i, j+1, m, H, P, &max);
                similarityScore(a, b, i, j+2, m, H, P, &max);
                similarityScore(a, b, i, j+3, m, H, P, &max);
                similarityScore(a, b, i, j+4, m, H, P, &max);
                similarityScore(a, b, i, j+5, m, H, P, &max);
                similarityScore(a, b, i, j+6, m, H, P, &max);
                similarityScore(a, b, i, j+7, m, H, P, &max);
            }
            
            // Handle remaining elements in block
            for (; j < j_end; ++j) {
                similarityScore(a, b, i, j, m, H, P, &max);
            }
        }
    }
}
