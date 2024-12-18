
#include "constants.h"

namespace {

inline int matchMissmatchScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

#ifdef __GNUC__
__attribute__((always_inline))
#endif
void similarityScore(const char *a, const char* b, 
                    const long long int i, const long long int j, 
                    const long long int m,
                    int* __restrict__ H, int* __restrict__ P, 
                    const int currentMax,
                    long long int* __restrict__ maxPos) 
{
    // Pre-compute all indices
    const long long int index = m * i + j;
    const long long int diag_index = index - m - 1;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;

    // Load all matrix values first
    const int h_diag = H[diag_index];
    const int h_up = H[up_index];
    const int h_left = H[left_index];
    
    // Pre-compute scores
    const int diag = h_diag + matchMissmatchScore(a[j-1], b[i-1]);
    const int up = h_up + gapScore;
    const int left = h_left + gapScore;

    // Compute maximum using sequential comparisons
    int max = (diag > NONE) ? diag : NONE;
    int pred = (diag > NONE) ? DIAGONAL : NONE;
    
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

    // Update maximum position if needed
    if (max > currentMax) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char *a, const char *b, 
            int* __restrict__ H, int* __restrict__ P, 
            const long long int n, const long long int m, 
            long long int& maxPos)
{
    // Process one row at a time to improve cache usage
    for (long long int i = 1; i < n; ++i) {
        // Cache the current maximum to reduce memory access
        int currentMax = H[maxPos];
        
        // Process elements in the row
        #ifdef __GNUC__
        #pragma GCC ivdep
        #pragma GCC unroll 4
        #endif
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, currentMax, &maxPos);
            
            // Update cached maximum
            if (H[maxPos] > currentMax) {
                currentMax = H[maxPos];
            }
        }
    }
}
