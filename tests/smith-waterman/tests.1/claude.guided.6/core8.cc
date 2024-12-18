
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                    const long long int i, const long long int j, 
                    const long long int m,
                    int* __restrict__ H, int* __restrict__ P, 
                    const int* __restrict__ maxScore,
                    long long int* __restrict__ maxPos) 
{
    // All index calculations at once
    const long long int index = m * i + j;
    const long long int diag_index = index - m - 1;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;

    // Pre-load all matrix values
    const int h_diag = H[diag_index];
    const int h_up = H[up_index];
    const int h_left = H[left_index];

    // Pre-load characters once
    const char b_char = b[i-1];
    const char a_char = a[j-1];
    
    // Calculate scores
    const int diag = h_diag + matchMissmatchScore(a_char, b_char);
    const int up = h_up + gapScore;
    const int left = h_left + gapScore;

    // Branchless max computation
    const int max1 = (diag > NONE) ? diag : NONE;
    const int pred1 = (diag > NONE) ? DIAGONAL : NONE;
    
    const int max2 = (up > max1) ? up : max1;
    const int pred2 = (up > max1) ? UP : pred1;
    
    const int max = (left > max2) ? left : max2;
    const int pred = (left > max2) ? LEFT : pred2;

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum if needed
    if (max > *maxScore) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char *a, const char *b, 
            int* __restrict__ H, int* __restrict__ P, 
            const long long int n, const long long int m, 
            long long int& maxPos)
{
    // Process in small blocks for better cache utilization
    constexpr long long int BLOCK_SIZE = 16;
    int currentMax = H[maxPos];  // Cache current maximum

    for (long long int i = 1; i < n; ++i) {
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int jEnd = (jj + BLOCK_SIZE < m) ? jj + BLOCK_SIZE : m;
            
            #ifdef __GNUC__
            #pragma GCC ivdep
            #pragma GCC unroll 4
            #endif
            for (long long int j = jj; j < jEnd; ++j) {
                similarityScore(a, b, i, j, m, H, P, &currentMax, &maxPos);
                // Update cached maximum
                const int newMax = H[maxPos];
                currentMax = (newMax > currentMax) ? newMax : currentMax;
            }
        }
    }
}
