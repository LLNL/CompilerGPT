
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b,
                    long long int i, long long int j, 
                    long long int m,
                    int* H, 
                    int* P,
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
    const int diag = H[index_diag] + matchMissmatchScore(a, b, i, j);

    // Initialize max and predecessor
    int max = NONE;
    int pred = NONE;
    
    // Optimized max calculation
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
    // Process two elements at a time where possible
    for (long long int i = 1; i < n; ++i) {
        long long int j;
        // Main loop with unrolling
        for (j = 1; j < m-1; j += 2) {
            similarityScore(a, b, i, j, m, H, P, &max);
            similarityScore(a, b, i, j+1, m, H, P, &max);
        }
        // Handle remaining element if any
        if (j < m) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
