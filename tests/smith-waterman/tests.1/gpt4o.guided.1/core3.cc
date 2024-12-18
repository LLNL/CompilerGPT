
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char* a, const char* b, char aj, char bi) {
    // We can use direct parameters instead of pointer arithmetic to potentially improve performance
    return (aj == bi) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b, long long int i, long long int j, long long int m,
                     int* H, int* P, long long int* maxPos) {

    long long int index = m * i + j;
    long long int indexPrevRow = index - m;

    // Use pre-computed indices to avoid repeated arithmetic
    int up = H[indexPrevRow] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[indexPrevRow - 1] + matchMissmatchScore(a, b, a[j - 1], b[i - 1]);

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

}  // end of anonymous namespace

void compute(const char* a, const char* b, int* H, int* P, long long int n, long long int m, long long int& maxPos) {
    for (long long int i = 1; i < n; ++i) {
        const char* b_i = &b[i - 1]; // Pointer to current row char in b

        for (long long int j = 1; j < m; ++j) {
            // Prefetch data so 'a[j-1]' can be used directly
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}

