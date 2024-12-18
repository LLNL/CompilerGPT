
#include "constants.h"

namespace {

inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                    long long int i, long long int j, long long int m,
                    int* H, int* P, 
                    long long int* maxPos) 
{
    // Calculate current index once
    const long long int index = m * i + j;
    const long long int diag_index = index - m - 1;
    
    // Calculate scores
    const int up = H[index - m] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[diag_index] + matchMissmatchScore(a, b, i, j);

    // Find maximum score
    int max = diag;
    int pred = DIAGONAL;
    
    // Use simple comparisons instead of multiple if statements
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

    // Update maximum position if needed
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // end anonymous namespace

void compute(const char *a, const char* b, int* H, int* P, long long int n, long long int m, long long int& max)
{
    // Pre-calculate loop bounds
    const long long int imax = n;
    const long long int jmax = m;
    
    for (long long int i = 1; i < imax; ++i) {
        for (long long int j = 1; j < jmax; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
