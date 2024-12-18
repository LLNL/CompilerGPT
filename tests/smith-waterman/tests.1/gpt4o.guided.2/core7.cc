
#include "constants.h"

namespace {

// Function to compute score based on character match/mismatch
inline int matchMissmatchScore(const char *a, const char* b, long long i, long long j) {
    return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

// Function to update the similarity and predecessor matrices
void similarityScore(const char *a, const char* b, long long i, long long j, long long m,
                     int* H, int* P, long long* maxPos) 
{
    long long index = m * i + j;
    long long index_m = index - m;
    long long index_diag = index_m - 1;

    int up = H[index_m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index_diag] + matchMissmatchScore(a, b, i, j);

    // Determine the maximum score and direction
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

    // Update maximum position if needed
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // unnamed namespace

// Main function to compute the matching score and build traceback matrices
void compute(const char *a, const char* b, int* H, int* P, long long n, long long m, long long& maxPos)
{
    maxPos = 0;

    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
