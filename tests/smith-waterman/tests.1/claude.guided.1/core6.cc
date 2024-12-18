
#include "constants.h"

namespace
{
  
inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    // Single pointer dereference for each character
    char a_char = a[j-1];
    char b_char = b[i-1];
    return (a_char == b_char) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                    long long int i, long long int j, long long int m,
                    int* H, int* P, 
                    long long int* maxPos) 
{
    // Calculate all indices once
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;
    
    // Load values once
    const int score_up = H[index_up];
    const int score_left = H[index_left];
    const int score_diag = H[index_diag];
    const int current_max = H[*maxPos];

    // Calculate all scores
    const int up = score_up + gapScore;
    const int left = score_left + gapScore;
    const int diag = score_diag + matchMissmatchScore(a, b, i, j);

    // Calculate maximum
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

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum if needed
    if (max > current_max) {
        *maxPos = index;
    }
}

}  // end anonymous namespace

void compute(const char *a, const char* b, int* H, int* P, 
            long long int n, long long int m, long long int& max)
{
    // Main computation loops
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
