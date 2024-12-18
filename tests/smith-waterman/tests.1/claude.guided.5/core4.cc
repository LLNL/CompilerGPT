
#include "constants.h"
#include <algorithm>

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    const int current_max, long long int* __restrict__ maxPos)
{
    const long long int index = m * i + j;
    
    // Calculate scores directly to reduce register pressure
    const int up = H[index - m] + gapScore;
    const int left = H[index - 1] + gapScore;
    const int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

    // Simple max calculation
    int max = NONE;
    int pred = NONE;
    
    // Unroll comparisons to help compiler optimize
    if (diag > max) {
        max = diag;
        pred = DIAGONAL;
    }
    if (up > max) {
        max = up;
        pred = UP;
    }
    if (left > max) {
        max = left;
        pred = LEFT;
    }

    H[index] = max;
    P[index] = pred;

    // Update maxPos if necessary
    if (max > current_max) {
        *maxPos = index;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            long long int n, long long int m, long long int& max)
{
    // Process rows in chunks for better cache utilization
    constexpr long long int BLOCK_SIZE = 16; // Smaller blocks to reduce register pressure
    
    for (long long int i = 1; i < n; ++i) {
        const int current_max = H[max]; // Cache current max for the row
        
        // Process elements in blocks
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int j_end = std::min(jj + BLOCK_SIZE, m);
            
            // Inner loop with minimal register usage
            #pragma clang loop vectorize(enable) interleave(enable)
            for (long long int j = jj; j < j_end; ++j) {
                similarityScore(a, b, i, j, m, H, P, current_max, &max);
            }
        }
    }
}
