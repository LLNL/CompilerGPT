
#include "constants.h"
#include <algorithm>

namespace {

// Branchless score calculation
inline int getScore(char a, char b) {
    return matchScore - ((a != b) * (matchScore - missmatchScore));
}

inline void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int row_offset,
                    const long long int prev_row_offset,
                    const long long int j,
                    const long long int m,
                    int* __restrict__ H, 
                    int* __restrict__ P,
                    long long int* __restrict__ maxPos,
                    const int i_minus_1,
                    const int j_minus_1)
{
    // Pre-calculated indices
    const long long int index = row_offset + j;
    const long long int up_index = prev_row_offset + j;
    const long long int left_index = index - 1;
    const long long int diag_index = prev_row_offset + j - 1;
    
    // Load all required values at once
    const int H_diag = H[diag_index];
    const int H_up = H[up_index];
    const int H_left = H[left_index];
    
    // Calculate all scores - use pre-calculated i-1, j-1 values
    const int diag_score = H_diag + getScore(b[i_minus_1], a[j_minus_1]);
    const int up_score = H_up + gapScore;
    const int left_score = H_left + gapScore;
    
    // Find maximum using sequential comparison
    int max = diag_score;
    int pred = DIAGONAL;
    
    if (up_score > max) {
        max = up_score;
        pred = UP;
    }
    if (left_score > max) {
        max = left_score;
        pred = LEFT;
    }
    if (max <= NONE) {
        max = NONE;
        pred = NONE;
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
    // Optimal block sizes for common L1 cache sizes
    const long long int BLOCK_I = 64;
    const long long int BLOCK_J = 64;
    
    // Main computation with blocked processing
    for (long long int ii = 1; ii < n; ii += BLOCK_I) {
        const long long int i_end = std::min(ii + BLOCK_I, n);
        
        for (long long int jj = 1; jj < m; jj += BLOCK_J) {
            const long long int j_end = std::min(jj + BLOCK_J, m);
            
            // Process each block
            for (long long int i = ii; i < i_end; ++i) {
                // Pre-calculate row offsets
                const long long int row_offset = i * m;
                const long long int prev_row_offset = (i - 1) * m;
                const int i_minus_1 = i - 1;
                
                // Manual loop unrolling (8x)
                long long int j = jj;
                for (; j + 7 < j_end; j += 8) {
                    similarityScore(a, b, row_offset, prev_row_offset, j, m, H, P, &max, i_minus_1, j-1);
                    similarityScore(a, b, row_offset, prev_row_offset, j+1, m, H, P, &max, i_minus_1, j);
                    similarityScore(a, b, row_offset, prev_row_offset, j+2, m, H, P, &max, i_minus_1, j+1);
                    similarityScore(a, b, row_offset, prev_row_offset, j+3, m, H, P, &max, i_minus_1, j+2);
                    similarityScore(a, b, row_offset, prev_row_offset, j+4, m, H, P, &max, i_minus_1, j+3);
                    similarityScore(a, b, row_offset, prev_row_offset, j+5, m, H, P, &max, i_minus_1, j+4);
                    similarityScore(a, b, row_offset, prev_row_offset, j+6, m, H, P, &max, i_minus_1, j+5);
                    similarityScore(a, b, row_offset, prev_row_offset, j+7, m, H, P, &max, i_minus_1, j+6);
                }
                
                // Handle remaining elements
                for (; j < j_end; ++j) {
                    similarityScore(a, b, row_offset, prev_row_offset, j, m, H, P, &max, i_minus_1, j-1);
                }
            }
        }
    }
}
