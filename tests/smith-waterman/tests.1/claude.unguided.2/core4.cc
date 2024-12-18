
#include "constants.h"

namespace {

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
    
    // Calculate all scores at once to enable instruction pipelining
    const int up = H[row_offset] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[row_offset - 1] + matchMissmatchScore(a, b, i, j);

    // Initialize max to diagonal score and update if needed
    int max = diag;
    int pred = DIAGONAL;
    
    // Branchless max calculation using conditional operators
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
    // Pre-calculate loop bounds
    const long long int m_minus_4 = m - 4;
    const long long int n_minus_1 = n - 1;

    // Main computation loop
    for (long long int i = 1; i <= n_minus_1; ++i) {
        // Vector-friendly loop with manual unrolling
        long long int j = 1;
        
        // Main chunk processing with unrolled loops
        for (; j <= m_minus_4; j += 4) {
            // Process blocks of 4 elements to improve instruction pipelining
            similarityScore(a, b, i, j, m, H, P, &max);
            similarityScore(a, b, i, j+1, m, H, P, &max);
            similarityScore(a, b, i, j+2, m, H, P, &max);
            similarityScore(a, b, i, j+3, m, H, P, &max);
        }
        
        // Handle remaining elements
        const long long int m_minus_1 = m - 1;
        for (; j <= m_minus_1; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
