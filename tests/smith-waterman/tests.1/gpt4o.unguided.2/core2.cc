
#include "constants.h"

namespace
{

inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

inline void similarityScore(const char *a, const char* b, 
                            long long int i, long long int j, long long int m,
                            int* H, int* P, 
                            long long int* maxPos) 
{
    const long long int index = m * i + j;

    const int up = H[index - m] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

    // Initialize max with NONE for safe comparison
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

    // Insert the value in the similarity and predecessor matrices
    H[index] = max;
    P[index] = pred;

    // Update maximum score to be used as seed on backtrack
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, int* H, int* P, long long int n, long long int m, long long int& max)
{
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
