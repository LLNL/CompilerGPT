
#include "constants.h"
#include <algorithm>

namespace {

inline int getScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

inline void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* __restrict__ maxPos,
                    int& current_max)
{
    const long long int index = m * i + j;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;
    const long long int diag_index = up_index - 1;
    
    // Calculate scores
    const int diag_score = H[diag_index] + getScore(b[i-1], a[j-1]);
    const int up_score = H[up_index] + gapScore;
    const int left_score = H[left_index] + gapScore;
    
    // Find maximum
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
    
    // Update maximum score if necessary
    if (max > current_max) {
        current_max = max;
        *maxPos = index;
    }
}

} // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    const long long int BLOCK_I = 32;
    const long long int BLOCK_J = 32;
    
    // Keep track of current maximum to avoid dereferencing maxPos repeatedly
    int current_max = H[max];
    
    // Blocked loops for better cache utilization
    for (long long int ii = 1; ii < n; ii += BLOCK_I) {
        const long long int i_end = std::min(ii + BLOCK_I, n);
        
        for (long long int jj = 1; jj < m; jj += BLOCK_J) {
            const long long int j_end = std::min(jj + BLOCK_J, m);
            
            // Process each block
            for (long long int i = ii; i < i_end; ++i) {
                // Unrolled inner loop
                long long int j = jj;
                for (; j + 4 <= j_end; j += 4) {
                    similarityScore(a, b, i, j, m, H, P, &max, current_max);
                    similarityScore(a, b, i, j+1, m, H, P, &max, current_max);
                    similarityScore(a, b, i, j+2, m, H, P, &max, current_max);
                    similarityScore(a, b, i, j+3, m, H, P, &max, current_max);
                }
                // Handle remaining elements
                for (; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max, current_max);
                }
            }
        }
    }
}
