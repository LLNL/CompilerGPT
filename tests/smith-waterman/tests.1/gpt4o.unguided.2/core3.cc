
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char *a, const char* b, long long i, long long j) {
    // Utilizing ternary operator for simplicity and efficiency
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

inline void similarityScore(const char *a, const char* b, 
                            long long i, long long j, long long m,
                            int* H, int* P, long long* maxPos) 
{
    const long long index = m * i + j;

    // Compute previous indices once
    const int up = H[index - m] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

    // Determine the maximum score and predecessor
    int max = NONE, pred = NONE;
    if (diag > max) { max = diag; pred = DIAGONAL; }
    if (up > max) { max = up; pred = UP; }
    if (left > max) { max = left; pred = LEFT; }

    // Store results in matrices
    H[index] = max;
    P[index] = pred;

    // Update maxPos if max is a new maximum
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, int* H, int* P, long long n, long long m, long long& max) {
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
