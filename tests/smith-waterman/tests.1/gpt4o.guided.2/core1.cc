
#include "constants.h"

// Use static inline for force inlining across translation units if needed
static inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, long long int i, long long int j, long long int m,
                     int* H, int* P, long long int* maxPos)
{
    long long index = m * i + j;
    int up = H[index - m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

    // Determine the max score and its direction
    int max = NONE;
    int pred = NONE;

    if (diag >= up && diag >= left) {
        max = diag;
        pred = DIAGONAL;
    } else if (up >= left) {
        max = up;
        pred = UP;
    } else {
        max = left;
        pred = LEFT;
    }

    H[index] = max;
    P[index] = pred;

    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

void compute(const char *a, const char* b, int* H, int* P, long long n, long long m, long long& maxPos)
{
    for (long long i = 1; i < n; ++i) {
        long long base_index = i * m;
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
