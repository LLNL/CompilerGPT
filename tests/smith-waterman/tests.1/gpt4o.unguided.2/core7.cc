
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
    const int current_char_score = matchMissmatchScore(a[j - 1], b[i - 1]);

    // Directly calculate the score potentials.
    const int up = H[index - m] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[index - m - 1] + current_char_score;
    
    // Determine maximum score and pred direction using temporary local variables.
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

    // Update matrices with the computed values.
    H[index] = maxScore;
    P[index] = pred;

    // Check and update maximum score position.
    if (maxScore > H[*maxPos]) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char *a, const char *b, int *H, int *P, long long n, long long m, long long &maxPos) {
    // Initialize max position explicitly
    maxPos = 0;

    // Loop over matrix excluding first row and column
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
