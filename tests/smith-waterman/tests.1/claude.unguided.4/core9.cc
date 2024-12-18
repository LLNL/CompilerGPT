
#include "constants.h"

namespace {

static const int score_table[2] = {missmatchScore, matchScore};

inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    return score_table[a[j-1] == b[i-1]];
}

inline void similarityScore(const char *a, const char* b, 
                          long long int i, long long int j, long long int m,
                          int* H, int* P, 
                          long long int* maxPos) {
    const long long int index = m * i + j;
    const long long int idx_up = index - m;
    
    const int up = H[idx_up] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[idx_up - 1] + matchMissmatchScore(a, b, i, j);

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

} // namespace

void compute(const char *a, const char* b,
            int* H, int* P,
            long long int n, long long int m, long long int& max) {
    constexpr long long int BLOCK_SIZE = 16;
    
    for (long long int i = 1; i < n; ++i) {
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int j_end = (jj + BLOCK_SIZE < m) ? jj + BLOCK_SIZE : m;
            for (long long int j = jj; j < j_end; ++j) {
                similarityScore(a, b, i, j, m, H, P, &max);
            }
        }
    }
}
