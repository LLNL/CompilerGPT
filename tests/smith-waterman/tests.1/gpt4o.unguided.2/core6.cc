
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a_char, char b_char) {
    return (a_char == b_char) ? matchScore : missmatchScore;
}

inline void similarityScore(const char *a, const char *b, 
                            long long i, long long j, long long m,
                            int *H, int *P, long long *maxPos) 
{
    const long long index = m * i + j;
    const int up = H[index - m] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[index - m - 1] + matchMissmatchScore(a[j - 1], b[i - 1]);

    // Compute the maximum score and antecedent direction
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

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update max position if current max score exceeds the known maximum
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char *a, const char *b, int *H, int *P, long long n, long long m, long long &max) {
    max = 0; // Ensure max is reset to a valid initial state

    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
