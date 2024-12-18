
#include "constants.h"
#include <algorithm>

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    int* __restrict__ maxH, long long int* __restrict__ maxPos)
{
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;

    // Load values first to reduce memory dependencies
    const int score_up = H[index_up] + gapScore;
    const int score_left = H[index_left] + gapScore;
    const int score_diag = H[index_diag] + matchMissmatchScore(a, b, i, j);
    const int current_max = *maxH;

    // Compute maximum score
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

    // Handle NONE case
    if (max <= NONE) {
        max = NONE;
        pred = NONE;
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum if necessary
    if (max > current_max) {
        *maxH = max;
        *maxPos = index;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            long long int n, long long int m, long long int& max)
{
    // Cache current maximum H value
    int maxH = H[max];
    
    constexpr long long int BLOCK_SIZE = 16;

    // Process matrix in blocks for better cache utilization
    for (long long int i = 1; i < n; ++i) {
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int j_end = std::min(jj + BLOCK_SIZE, m);
            
            #pragma clang loop vectorize(enable) interleave(enable)
            for (long long int j = jj; j < j_end; ++j) {
                similarityScore(a, b, i, j, m, H, P, &maxH, &max);
            }
        }
    }
}
