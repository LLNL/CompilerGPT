
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a_char, char b_char) {
    return (a_char == b_char) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b,
                    long long int i, long long int j, long long int m,
                    int* H, int* P,
                    long long int* maxPos)
{
    // Pre-calculate indices
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;
    
    // Calculate scores
    const int up = H[index_up] + gapScore;
    const int left = H[index_left] + gapScore;
    const int diag = H[index_diag] + matchMissmatchScore(a[j-1], b[i-1]);

    // Initialize max and predecessor
    int max = NONE;
    int pred = NONE;
    
    // Optimized max calculation with fewer branches
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

    // Update maximum position
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // anonymous namespace

void compute(const char* a, const char* b,
            int* H, int* P,
            long long int n, long long int m, long long int& max)
{
    // Main computation loops with partial unrolling
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m - 1; j += 2) {
            similarityScore(a, b, i, j, m, H, P, &max);
            similarityScore(a, b, i, j + 1, m, H, P, &max);
        }
        // Handle last element if m is even
        if ((m - 1) % 2 == 1) {
            similarityScore(a, b, i, m - 1, m, H, P, &max);
        }
    }
}
