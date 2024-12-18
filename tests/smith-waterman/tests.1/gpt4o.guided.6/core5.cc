
#include "constants.h"
#include <cstddef> // Include for std::size_t

namespace {

// Helper function to calculate match/mismatch score.
inline int matchMissmatchScore(const char *a, const char *b, long long i, long long j) {
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

// Function to compute the similarity score matrix and predecessor matrix.
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

} // unnamed namespace

// Function to compute similarity scores for the entire matrix.
void compute(const char *a, const char *b, int *H, int *P, long long n, long long m, long long &maxPos) 
{
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
