
#include "constants.h"
#include <cstddef> // To allow std::size_t usage.

namespace {

// Determine match or mismatch score.
inline int matchMissmatchScore(const char *a, const char *b, long long i, long long j) {
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

// Update similarity scores and predecessors.
void similarityScore(const char *a, const char *b, 
                     long long i, long long j, long long m,
                     int *H, int *P, long long *maxPos) 
{
    long long index = m * i + j;
    long long indexUp = index - m;
    long long indexLeft = index - 1;
    long long indexDiag = index - m - 1;

    const int up = H[indexUp] + gapScore;
    const int left = H[indexLeft] + gapScore;
    const int diag = H[indexDiag] + matchMissmatchScore(a, b, i, j);

    int max = diag;
    int pred = DIAGONAL;

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

} // End of unnamed namespace

// Assumed function that needs to be defined within the project.
void compute(const char *a, const char *b, int *H, int *P, long long n, long long m, long long &maxPos) 
{
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
