
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
    
    // Load values once
    const int h_up = H[index_up];
    const int h_left = H[index_left];
    const int h_diag = H[index_diag];
    const int current_max = H[*maxPos];
    
    // Calculate scores
    const int up = h_up + gapScore;
    const int left = h_left + gapScore;
    const int diag = h_diag + matchMissmatchScore(a, b, i, j);

    // Find max and predecessor using optimized logic
    int max = NONE;
    int pred = NONE;
    
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

    // Update maximum position with single comparison
    if (max > current_max) {
        *maxPos = index;
    }
}

} // anonymous namespace

void compute(const char* a, const char* b,
            int* H, int* P,
            long long int n, long long int m, long long int& max)
{
    // Process multiple elements at a time
    const long long int block_size = 4;
    
    for (long long int i = 1; i < n; ++i) {
        long long int j = 1;
        
        // Main loop with 4x unrolling
        for (; j <= m-4; j += 4) {
            similarityScore(a, b, i, j, m, H, P, &max);
            similarityScore(a, b, i, j+1, m, H, P, &max);
            similarityScore(a, b, i, j+2, m, H, P, &max);
            similarityScore(a, b, i, j+3, m, H, P, &max);
        }
        
        // Handle remaining elements
        for (; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
