
#include "constants.h"
#include <algorithm>

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j,
                    const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    int& current_max, long long int& max_pos)
{
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;

    // Load all values first
    const int score_up = H[index_up] + gapScore;
    const int score_left = H[index_left] + gapScore;
    const int score_diag = H[index_diag] + matchMissmatchScore(a, b, i, j);

    // Compute maximum score
    int score = score_diag;
    int pred = DIAGONAL;

    if (score_up > score) {
        score = score_up;
        pred = UP;
    }
    if (score_left > score) {
        score = score_left;
        pred = LEFT;
    }
    if (score <= NONE) {
        score = NONE;
        pred = NONE;
    }

    // Store results
    H[index] = score;
    P[index] = pred;

    // Update maximum if necessary
    if (score > current_max) {
        current_max = score;
        max_pos = index;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            long long int n, long long int m, long long int& max)
{
    // Track maximum score
    int current_max = H[max];
    long long int max_pos = max;

    constexpr long long int BLOCK_SIZE = 32;

    // Process matrix in blocks
    for (long long int i = 1; i < n; ++i) {
        #pragma clang loop vectorize(enable) interleave(enable)
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, current_max, max_pos);
        }
    }

    // Update final maximum position
    max = max_pos;
}
