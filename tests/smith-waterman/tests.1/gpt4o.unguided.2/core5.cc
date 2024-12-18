
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a_char, char b_char) {
    // Simple comparison, removing the need for array indexing
    return (a_char == b_char) ? matchScore : missmatchScore;
}

inline void similarityScore(const char *a, const char* b, 
                            long long i, long long j, long long m,
                            int* H, int* P, long long* maxPos) 
{
    const long long index = m * i + j;
    const long long index_up = index - m;
    const long long index_diag = index_up - 1;

    // Compute scores using precomputed match score
    const int up = H[index_up] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[index_diag] + matchMissmatchScore(a[j - 1], b[i - 1]);

    // Determine the maximum score and corresponding direction
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

    // Store the result in the matrices
    H[index] = max;
    P[index] = pred;

    // Update maxPos if the current max score exceeds the known max score
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, int* H, int* P, long long n, long long m, long long& max) {
    max = 0; // Initialize max position
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
