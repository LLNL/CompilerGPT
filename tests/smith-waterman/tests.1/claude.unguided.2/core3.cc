
#include "constants.h"

namespace {

// Small, frequently used function stays inline
inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b,
                    const long long int i, const long long int j, 
                    const long long int m,
                    int* const __restrict__ H, int* const __restrict__ P,
                    long long int* const __restrict__ maxPos)
{
    // Pre-calculate all indices once
    const long long int index = m * i + j;
    const long long int row_offset = index - m;
    const int up = H[row_offset] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[row_offset - 1] + matchMissmatchScore(a, b, i, j);

    // Optimize max calculation using local register variables
    register int max = diag;
    register int pred = DIAGONAL;
    
    // Simplified branching with register variables
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

    // Compare against dereferenced maxPos once
    const long long int currentMax = *maxPos;
    if (max > H[currentMax]) {
        *maxPos = index;
    }
}

} // anonymous namespace

void compute(const char* const __restrict__ a, 
            const char* const __restrict__ b,
            int* const __restrict__ H, 
            int* const __restrict__ P,
            const long long int n, 
            const long long int m, 
            long long int& max)
{
    // Hoist constant calculations out of loops
    const long long int m_minus_4 = m - 4;

    // Main computation loop with better cache locality
    for (long long int i = 1; i < n; ++i) {
        // Process main chunk with loop unrolling
        long long int j = 1;
        for (; j <= m_minus_4; j += 4) {
            // Process 4 elements in sequence to allow for vectorization
            similarityScore(a, b, i, j, m, H, P, &max);
            similarityScore(a, b, i, j+1, m, H, P, &max);
            similarityScore(a, b, i, j+2, m, H, P, &max);
            similarityScore(a, b, i, j+3, m, H, P, &max);
        }
        
        // Handle remaining elements (if any)
        for (; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
