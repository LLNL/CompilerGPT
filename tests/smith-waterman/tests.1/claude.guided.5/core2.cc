
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
    const long long int index = m * i + j;
    const long long int diag_index = index - m - 1;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;
    
    const int up = H[up_index] + gapScore;
    const int left = H[left_index] + gapScore;
    const int diag = H[diag_index] + matchMissmatchScore(a, b, i, j);

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

    H[index] = max;
    P[index] = pred;

    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            long long int n, long long int m, long long int& max)
{
    // Loop tiling for better cache utilization
    constexpr int TILE_SIZE = 64;
    for (long long int ii = 1; ii < n; ii += TILE_SIZE) {
        const long long int i_end = std::min(ii + TILE_SIZE, n);
        for (long long int jj = 1; jj < m; jj += TILE_SIZE) {
            const long long int j_end = std::min(jj + TILE_SIZE, m);
            
            for (long long int i = ii; i < i_end; ++i) {
                #pragma clang loop vectorize(enable)
                for (long long int j = jj; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
