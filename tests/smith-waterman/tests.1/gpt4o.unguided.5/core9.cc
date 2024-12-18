
#include "constants.h"

namespace {

// Forward declare these constants if they are missing from "constants.h"
// const int matchScore = 1;
// const int missmatchScore = -1;
// const int gapScore = -2;
// const int NONE = 0;
// const int DIAGONAL = 1;
// const int UP = 2;
// const int LEFT = 3;

inline int matchMissmatchScore(const char* a, const char* b, long long i, long long j) {
    // Ensure the indices are valid with respect to expected input arrays
    if (j > 0 && i > 0) {
        return (a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
    }
    return missmatchScore;  // Default mismatch score if out-of-bound, ensure i, j >= 1 in calling context
}

void similarityScore(const char* a, const char* b, 
                     long long i, long long j, long long m,
                     int* H, int* P, 
                     long long* maxPos)
{
    // Calculate the current linear index
    long long index = m * i + j;

    // Ensure indices do not underflow; This assumes i, j >= 1
    long long index_m = index - m;        // Top element index
    long long index_m1 = index_m - 1;     // Top-left diagonal
    long long index_1 = index - 1;        // Left element index

    int up = H[index_m] + gapScore;
    int left = H[index_1] + gapScore;
    int diag = H[index_m1] + matchMissmatchScore(a, b, i, j);

    // Initialize with diagonal values to prioritize matches
    int max = diag;
    int pred = DIAGONAL;

    if (up > max) {  // If the value from up exceeds the current max
        max = up;
        pred = UP;
    }

    if (left > max) {  // If the value from left exceeds the current max
        max = left;
        pred = LEFT;
    }

    // Store calculated values into the matrices
    H[index] = max;
    P[index] = pred;

    // Update the maximum score position if necessary
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

}

void compute(const char* a, const char* b, 
             int* H, int* P, 
             long long n, long long m, long long& maxPos)
{
    // Ensure that maxPos is initialized correctly before use
    maxPos = 0;  // Initialize to a known base position

    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
}
