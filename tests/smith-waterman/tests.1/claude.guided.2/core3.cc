
#include "constants.h"
#include <algorithm>
#include <cstddef>

namespace {

inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    return (a[j-1] == b[i-1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    const int current_max_val, long long int* __restrict__ maxPos)
{
    // Pre-calculate index and reuse
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;
    
    // Load matrix values first to reduce register pressure
    const int h_up = H[index_up];
    const int h_left = H[index_left];
    const int h_diag = H[index_diag];
    
    // Calculate scores using loaded values
    const int up = h_up + gapScore;
    const int left = h_left + gapScore;
    const int diag = h_diag + matchMissmatchScore(a, b, i, j);

    // Simplified max calculation to reduce register pressure
    int max, pred;
    if (diag > up) {
        if (diag > left) {
            max = diag;
            pred = DIAGONAL;
        } else {
            max = left;
            pred = LEFT;
        }
    } else {
        if (up > left) {
            max = up;
            pred = UP;
        } else {
            max = left;
            pred = LEFT;
        }
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maxPos if needed
    if (max > current_max_val) {
        *maxPos = index;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Process in smaller blocks to reduce register pressure
    constexpr long long int BLOCK_SIZE = 8;
    
    for (long long int i = 1; i < n; ++i) {
        // Load max value once per row to help LICM
        const int current_max_val = H[max];
        
        #ifdef __clang__
        #pragma clang loop vectorize(enable) interleave(enable)
        #endif
        for (long long int j = 1; j < m; j++) {
            similarityScore(a, b, i, j, m, H, P, current_max_val, &max);
        }
    }
}
