
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b, long long int i, long long int j, long long int m,
                     int* H, int* P, long long int* maxPos) {

    long long int index = m * i + j;

    int up = H[index - m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

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

void compute(const char* a, const char* b, int* H, int* P, long long int n, long long int m, long long int& maxPos) {
    char prevA = a[0], prevB = b[0]; // hoist conditionally invariant parts
    for (long long int i = 1, indexI = m; i < n; ++i, indexI += m) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}

}
