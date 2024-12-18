
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char a, const char b) {
    return (a == b) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* const maxPos)
{
    // Pre-calculate the index to avoid repeated multiplication
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index_up - 1;

    // Prefetch next iteration's data
    __builtin_prefetch(&H[index + 1]);
    __builtin_prefetch(&P[index + 1]);

    // Load values first to help with instruction pipelining
    const int up = H[index_up] + gapScore;
    const int left = H[index_left] + gapScore;
    const int diag = H[index_diag] + matchMissmatchScore(b[i-1], a[j-1]);

    // Use branchless programming for max calculation
    int max = NONE;
    int pred = NONE;
    
    // Combine comparisons to reduce branches
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

    // Branchless max update
    const bool shouldUpdate = (max > H[*maxPos]);
    *maxPos = shouldUpdate ? index : *maxPos;
}

} // namespace

void compute(const char *a, const char* b, int* __restrict__ H, int* __restrict__ P, 
            const long long int n, const long long int m, long long int& max)
{
    // Loop tiling for better cache utilization
    const int TILE_SIZE = 64;  // Adjust based on cache size
    
    for (long long int i_tile = 1; i_tile < n; i_tile += TILE_SIZE) {
        const long long int i_end = ((i_tile + TILE_SIZE) < n) ? (i_tile + TILE_SIZE) : n;
        
        for (long long int j_tile = 1; j_tile < m; j_tile += TILE_SIZE) {
            const long long int j_end = ((j_tile + TILE_SIZE) < m) ? (j_tile + TILE_SIZE) : m;
            
            for (long long int i = i_tile; i < i_end; ++i) {
                #pragma unroll(4)
                for (long long int j = j_tile; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
