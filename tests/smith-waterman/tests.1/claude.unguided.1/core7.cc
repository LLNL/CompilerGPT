
#include "constants.h"
#include <algorithm>

namespace {

// Branchless score calculation, forced inline
#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
inline int getScore(char a, char b) {
    return matchScore - ((a != b) * (matchScore - missmatchScore));
}

// Main computation kernel, forced inline
#if defined(__GNUC__) || defined(__clang__)
__attribute__((always_inline))
#endif
inline void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos,
                    const long long int diag_index,
                    const long long int up_index,
                    const long long int left_index,
                    const long long int index)
{
    // Load all values first - helps CPU pipeline loads
    const int H_up = H[up_index];
    const int H_left = H[left_index];
    const int H_diag = H[diag_index];
    
    // Calculate all scores
    const int diag_score = H_diag + getScore(b[i-1], a[j-1]);
    const int up_score = H_up + gapScore;
    const int left_score = H_left + gapScore;
    
    // Find maximum using sequential comparisons
    int max, pred;
    if (diag_score >= up_score && diag_score >= left_score) {
        max = diag_score;
        pred = DIAGONAL;
    } else if (up_score >= left_score) {
        max = up_score;
        pred = UP;
    } else {
        max = left_score;
        pred = LEFT;
    }
    
    // Store results
    H[index] = max;
    P[index] = pred;
    
    // Update maximum if necessary
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
    constexpr long long int BLOCK_I = 64;
    constexpr long long int BLOCK_J = 64;
    
    // Main computation with blocked processing
    for (long long int ii = 1; ii < n; ii += BLOCK_I) {
        const long long int i_end = std::min(ii + BLOCK_I, n);
        
        for (long long int jj = 1; jj < m; jj += BLOCK_J) {
            const long long int j_end = std::min(jj + BLOCK_J, m);
            
            // Process each block
            for (long long int i = ii; i < i_end; ++i) {
                // Pre-calculate row-specific indices
                const long long int row_offset = i * m;
                const long long int prev_row_offset = (i-1) * m;
                
                // Manual loop unrolling with 6 iterations
                long long int j = jj;
                for (; j + 5 < j_end; j += 6) {
                    // Pre-calculate all indices for the unrolled loop
                    const long long int index1 = row_offset + j;
                    const long long int index2 = index1 + 1;
                    const long long int index3 = index1 + 2;
                    const long long int index4 = index1 + 3;
                    const long long int index5 = index1 + 4;
                    const long long int index6 = index1 + 5;
                    
                    similarityScore(a, b, i, j, m, H, P, &max,
                        prev_row_offset + j - 1,
                        prev_row_offset + j,
                        index1 - 1,
                        index1);
                    
                    similarityScore(a, b, i, j+1, m, H, P, &max,
                        prev_row_offset + j,
                        prev_row_offset + j + 1,
                        index2 - 1,
                        index2);
                    
                    similarityScore(a, b, i, j+2, m, H, P, &max,
                        prev_row_offset + j + 1,
                        prev_row_offset + j + 2,
                        index3 - 1,
                        index3);
                    
                    similarityScore(a, b, i, j+3, m, H, P, &max,
                        prev_row_offset + j + 2,
                        prev_row_offset + j + 3,
                        index4 - 1,
                        index4);
                    
                    similarityScore(a, b, i, j+4, m, H, P, &max,
                        prev_row_offset + j + 3,
                        prev_row_offset + j + 4,
                        index5 - 1,
                        index5);
                    
                    similarityScore(a, b, i, j+5, m, H, P, &max,
                        prev_row_offset + j + 4,
                        prev_row_offset + j + 5,
                        index6 - 1,
                        index6);
                }
                
                // Handle remaining elements
                for (; j < j_end; ++j) {
                    const long long int index = row_offset + j;
                    similarityScore(a, b, i, j, m, H, P, &max,
                        prev_row_offset + j - 1,
                        prev_row_offset + j,
                        index - 1,
                        index);
                }
            }
        }
    }
}
