
#include "constants.h"
#include <algorithm>

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos)
{
    // Pre-calculate all indices to reduce register pressure
    const long long int index = m * i + j;
    const long long int diag_index = index - m - 1;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;
    
    // Load values once
    const int up_score = H[up_index];
    const int left_score = H[left_index];
    const int diag_score = H[diag_index];
    const int match_score = matchMissmatchScore(a, b, i, j);

    // Calculate scores
    const int up = up_score + gapScore;
    const int left = left_score + gapScore;
    const int diag = diag_score + match_score;

    // Branchless max calculation to help vectorization
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

    // Ensure max is at least NONE
    max = (max > NONE) ? max : NONE;
    pred = (max > NONE) ? pred : NONE;

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maxPos if necessary
    const int current_max = H[*maxPos];
    if (max > current_max) {
        *maxPos = index;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            long long int n, long long int m, long long int& max)
{
    constexpr int TILE_SIZE = 32; // Reduced tile size for better cache utilization
    
    // Process tiles
    #pragma clang loop vectorize(enable) interleave(enable)
    for (long long int ii = 1; ii < n; ii += TILE_SIZE) {
        const long long int i_end = std::min(ii + TILE_SIZE, n);
        
        for (long long int jj = 1; jj < m; jj += TILE_SIZE) {
            const long long int j_end = std::min(jj + TILE_SIZE, m);
            
            // Process each tile
            for (long long int i = ii; i < i_end; ++i) {
                // Hint for vectorization
                #pragma clang loop vectorize(enable) interleave(enable)
                for (long long int j = jj; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
