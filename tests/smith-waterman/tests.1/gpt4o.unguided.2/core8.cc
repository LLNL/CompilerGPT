
#include "constants.h"

namespace {

inline int matchMismatchScore(char a_char, char b_char) {
    return (a_char == b_char) ? matchScore : mismatchScore; // Ensure correct spelling
}

inline void similarityScore(const char *a, const char *b,
                            long long i, long long j, long long m,
                            int *H, int *P, long long *maxPos)
{
    const long long index = m * i + j;
    const int currentCharScore = matchMismatchScore(a[j - 1], b[i - 1]);

    // Calculate score candidates
    const int up = H[index - m] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[index - m - 1] + currentCharScore;
    
    // Determine maximum score and direction
    int maxScore = diag;
    int pred = DIAGONAL;

    if (up > maxScore) {
        maxScore = up;
        pred = UP;
    }
    if (left > maxScore) {
        maxScore = left;
        pred = LEFT;
    }

    // Populate the matrices with results
    H[index] = maxScore;
    P[index] = pred;

    // Update the maximum position if applicable
    if (maxScore > H[*maxPos]) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char *a, const char *b, int *H, int *P, long long n, long long m, long long &maxPos) {
    maxPos = 0; // Explicitly initialize max position

    // Iterate over matrix space, excluding first row/column
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
