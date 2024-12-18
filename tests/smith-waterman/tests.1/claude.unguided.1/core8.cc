
#include "constants.h"
#include <algorithm>

namespace {

// Branchless score calculation
inline int getScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

inline void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos)
{
    // Pre-calculate all indices to minimize index computation
    const long long int row_offset = i * m;
    const long long int prev_row_offset = (i - 1) * m;
    const long long int index = row_offset + j;
    const long long int up_index = prev_row_offset + j;
    const long long int left_index = index - 1;
    const long long int diag_index = prev_row_offset + j - 1;
    
    // Load all matrix values at once to help with memory access patterns
    const int H_diag = H[diag_index];
    const int H_up = H[up_index];
    const int H_left = H[left_index];
    
    // Calculate all scores
    const int diag_score = H_diag + getScore(b[i-1], a[j-1]);
    const int up_score = H_up + gapScore;
    const int left_score = H_left + gapScore;
    
    // Optimized maximum finding
    int max = NONE;
    int pred = NONE;
    
    if (diag_score > max) {
        max = diag_score;
        pred = DIAGONAL;
    }
    if (up_score > max) {
        max = up_score;
        pred = UP;
    }
    if (left_score > max) {
        max = left_score;
        pred = LEFT;
    }
    
    // Store results
    H[index] = max;
    P[index] = pred;
    
    // Update maximum position if necessary
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Cache-friendly block sizes
    const long long int BLOCK_I = 64;
    const long long int BLOCK_J = 64;
    
    // Main computation with blocked processing
    for (long long int ii = 1; ii < n; ii += BLOCK_I) {
        const long long int i_end = std::min(ii + BLOCK_I, n);
        
        for (long long int jj = 1; jj < m; jj += BLOCK_J) {
            const long long int j_end = std::min(jj + BLOCK_J, m);
            
            // Process each block
            for (long long int i = ii; i < i_end; ++i) {
                // Manual loop unrolling
                long long int j = jj;
                for (; j + 3 < j_end; j += 4) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                    similarityScore(a, b, i, j+1, m, H, P, &max);
                    similarityScore(a, b, i, j+2, m, H, P, &max);
                    similarityScore(a, b, i, j+3, m, H, P, &max);
                }
                
                // Handle remaining elements
                for (; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
