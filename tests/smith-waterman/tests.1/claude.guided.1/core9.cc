
#include "constants.h"

namespace
{
  
inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    // Pre-fetch the comparison result to help register allocation
    const bool chars_match = (a[j-1] == b[i-1]);
    return chars_match ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                    long long int i, long long int j, long long int m,
                    int* H, int* P, 
                    long long int* maxPos) 
{
    // Pre-compute index once
    const long long int index = m * i + j;
    
    // Pre-compute neighbor indices
    const long long int up_index = index - m;
    const long long int left_index = index - 1;
    const long long int diag_index = index - m - 1;

    // Calculate all scores
    const int up = H[up_index] + gapScore;
    const int left = H[left_index] + gapScore;
    const int diag = H[diag_index] + matchMissmatchScore(a, b, i, j);

    // Initialize max and pred as in original
    int max = NONE;
    int pred = NONE;
    
    // Keep original comparison order
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

    // Update maximum if needed
    const int current_max = H[*maxPos];
    if (max > current_max) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, int* H, int* P, long long int n, long long int m, long long int& max)
{
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
