
#include "constants.h"
#include <algorithm>

namespace {

// Make score calculation branchless
inline int getScore(char a, char b) {
    return matchScore - (missmatchScore - matchScore) * (a != b);
}

inline void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos,
                    int& current_max)
{
    // Pre-calculate all indices
    const long long int index = m * i + j;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;
    const long long int diag_index = up_index - 1;
    
    // Load all values at once to allow CPU to handle memory operations in parallel
    const int H_up = H[up_index];
    const int H_left = H[left_index];
    const int H_diag = H[diag_index];
    
    // Calculate all scores
    const int diag_score = H_diag + getScore(b[i-1], a[j-1]);
    const int up_score = H_up + gapScore;
    const int left_score = H_left + gapScore;
    
    // Find maximum more efficiently
    int max = diag_score;
    int pred = DIAGONAL;
    
    // Use conditional assignments instead of if-statements where possible
    if (up_score > max) {
        max = up_score;
        pred = UP;
    }
    if (left_score > max) {
        max = left_score;
        pred = LEFT;
    }
    
    // Write results
    H[index] = max;
    P[index] = pred;
    
    // Update maximum using a branchless approach when possible
    const bool is_new_max = max > current_max;
    current_max = is_new_max ? max : current_max;
    *maxPos = is_new_max ? index : *maxPos;
}

} // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Optimize block sizes for L1 cache
    const long long int BLOCK_I = 64;  // Increased block size
    const long long int BLOCK_J = 64;
    
    int current_max = H[max];
    
    // Main computation with blocked processing
    for (long long int ii = 1; ii < n; ii += BLOCK_I) {
        const long long int i_end = std::min(ii + BLOCK_I, n);
        
        for (long long int jj = 1; jj < m; jj += BLOCK_J) {
            const long long int j_end = std::min(jj + BLOCK_J, m);
            
            // Process block
            for (long long int i = ii; i < i_end; ++i) {
                // Explicitly tell compiler to align memory
                #ifdef __GNUC__
                #pragma GCC ivdep
                #endif
                
                // Manual loop unrolling with step of 8
                long long int j = jj;
                for (; j + 7 < j_end; j += 8) {
                    similarityScore(a, b, i, j, m, H, P, &max, current_max);
                    similarityScore(a, b, i, j+1, m, H, P, &max, current_max);
                    similarityScore(a, b, i, j+2, m, H, P, &max, current_max);
                    similarityScore(a, b, i, j+3, m, H, P, &max, current_max);
                    similarityScore(a, b, i, j+4, m, H, P, &max, current_max);
                    similarityScore(a, b, i, j+5, m, H, P, &max, current_max);
                    similarityScore(a, b, i, j+6, m, H, P, &max, current_max);
                    similarityScore(a, b, i, j+7, m, H, P, &max, current_max);
                }
                
                // Handle remaining elements
                for (; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max, current_max);
                }
            }
        }
    }
}
