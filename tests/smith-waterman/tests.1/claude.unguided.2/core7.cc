
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
    // Pre-calculate all indices at once to avoid redundant calculations
    const long long int index = m * i + j;
    const long long int row_offset = index - m;
    const long long int diag_index = row_offset - 1;
    const long long int left_index = index - 1;
    
    // Pre-fetch next memory locations to improve cache utilization
    const int up_score = H[row_offset];
    const int left_score = H[left_index];
    const int diag_score = H[diag_index];
    
    // Calculate all scores at once
    const int up = up_score + gapScore;
    const int left = left_score + gapScore;
    const int diag = diag_score + matchMissmatchScore(a, b, i, j);

    // Initialize with diagonal to reduce one comparison
    int max = diag;
    int pred = DIAGONAL;
    
    // Combine condition checks to help branch prediction
    if (up > max) {
        max = up;
        pred = UP;
    }
    
    if (left > max) {
        max = left;
        pred = LEFT;
    }

    // Single write per array
    H[index] = max;
    P[index] = pred;

    // Cache current maximum value
    const int currentMax = H[*maxPos];
    if (max > currentMax) {
        *maxPos = index;
    }
}

}

void compute(const char *a, const char* b, 
            int* H, int* P, 
            long long int n, long long int m, 
            long long int& max)
{
    // Process in blocks for better cache utilization
    const long long int block_size = 4;
    const long long int m_adjusted = m - (m % block_size);
    
    for (long long int i = 1; i < n; ++i) {
        // Main block processing
        long long int j = 1;
        for (; j < m_adjusted; j += block_size) {
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
