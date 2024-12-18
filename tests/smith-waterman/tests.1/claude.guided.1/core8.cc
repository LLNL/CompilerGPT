
#include "constants.h"

namespace
{
  
inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    // Pre-compute offsets to reduce register pressure
    const char a_char = a[j-1];
    const char b_char = b[i-1];
    return (a_char == b_char) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                    long long int i, long long int j, long long int m,
                    int* H, int* P, 
                    long long int* maxPos) 
{
    // Pre-compute all indices to reduce register pressure
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;
    
    // Cache max value to reduce memory access
    const long long int curr_max_pos = *maxPos;
    const int curr_max_val = H[curr_max_pos];

    // Load all required values at once
    const int score_up = H[index_up];
    const int score_left = H[index_left];
    const int score_diag = H[index_diag];

    // Calculate scores
    const int up = score_up + gapScore;
    const int left = score_left + gapScore;
    const int diag = score_diag + matchMissmatchScore(a, b, i, j);

    // Initialize with first comparison
    int max = diag;
    int pred = DIAGONAL;

    // Use simple comparisons to help compiler optimize
    const bool is_up_larger = up > max;
    max = is_up_larger ? up : max;
    pred = is_up_larger ? UP : pred;

    const bool is_left_larger = left > max;
    max = is_left_larger ? left : max;
    pred = is_left_larger ? LEFT : pred;

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum position if needed
    if (max > curr_max_val) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, int* H, int* P, long long int n, long long int m, long long int& max)
{
    // Cache loop bounds
    const long long int i_max = n;
    const long long int j_max = m;
    
    // Main computation loops
    for (long long int i = 1; i < i_max; ++i) {
        for (long long int j = 1; j < j_max; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
