
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                    long long int i, long long int j, long long int m,
                    int* __restrict__ H, int* __restrict__ P, 
                    long long int* __restrict__ maxPos) 
{
    // Pre-compute indices to help vectorization
    const long long int index = m * i + j;
    const long long int diag_index = index - m - 1;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;
    
    // Load values once
    const char char_a = a[j-1];
    const char char_b = b[i-1];
    const int score = matchMissmatchScore(char_a, char_b);
    
    // Load matrix values
    const int h_diag = H[diag_index];
    const int h_up = H[up_index];
    const int h_left = H[left_index];
    
    // Compute scores
    const int diag = h_diag + score;
    const int up = h_up + gapScore;
    const int left = h_left + gapScore;

    // Find maximum using branchless operations
    int max = diag;
    int pred = DIAGONAL;
    
    const bool up_better = up > max;
    max = up_better ? up : max;
    pred = up_better ? UP : pred;
    
    const bool left_better = left > max;
    max = left_better ? left : max;
    pred = left_better ? LEFT : pred;

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum position
    const int curr_max = H[*maxPos];
    const bool is_new_max = max > curr_max;
    *maxPos = is_new_max ? index : *maxPos;
}

} // namespace

void compute(const char *__restrict__ a, const char *__restrict__ b, 
            int* __restrict__ H, int* __restrict__ P, 
            long long int n, long long int m, long long int& maxPos)
{
    // Align the tile size to cache line size (typically 64 bytes)
    constexpr long long int TILE_SIZE = 64;
    constexpr long long int INNER_TILE = 8; // For register blocking
    
    #ifdef __GNUC__
    #pragma GCC ivdep
    #endif
    for (long long int ii = 1; ii < n; ii += TILE_SIZE) {
        const long long int iEnd = (ii + TILE_SIZE < n) ? ii + TILE_SIZE : n;
        
        for (long long int jj = 1; jj < m; jj += TILE_SIZE) {
            const long long int jEnd = (jj + TILE_SIZE < m) ? jj + TILE_SIZE : m;
            
            // Inner blocking for better register utilization
            for (long long int i = ii; i < iEnd; i += INNER_TILE) {
                const long long int iLimit = (i + INNER_TILE < iEnd) ? i + INNER_TILE : iEnd;
                
                for (long long int j = jj; j < jEnd; j += INNER_TILE) {
                    const long long int jLimit = (j + INNER_TILE < jEnd) ? j + INNER_TILE : jEnd;
                    
                    #ifdef __GNUC__
                    #pragma GCC ivdep
                    #pragma GCC unroll 8
                    #endif
                    for (long long int ii2 = i; ii2 < iLimit; ++ii2) {
                        #ifdef __GNUC__
                        #pragma GCC ivdep
                        #pragma GCC unroll 8
                        #endif
                        for (long long int jj2 = j; jj2 < jLimit; ++jj2) {
                            similarityScore(a, b, ii2, jj2, m, H, P, &maxPos);
                        }
                    }
                }
            }
        }
    }
}
