
#include "constants.h"

namespace {

// Force inline to ensure the function is always inlined
#ifdef _MSC_VER
    #define FORCE_INLINE __forceinline
#else
    #define FORCE_INLINE __attribute__((always_inline)) inline
#endif

// Use ternary operator instead of branch for match/mismatch
FORCE_INLINE
int matchMissmatchScore(char a_char, char b_char) {
    return (a_char == b_char) ? matchScore : missmatchScore;
}

FORCE_INLINE
void similarityScore(const char* a, const char* b,
                    long long int i, long long int j, 
                    long long int m,
                    int* H, 
                    int* P,
                    long long int* maxPos)
{
    // Pre-calculate all indices at once
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;
    
    // Load all memory values at once to help cache coherency
    const int h_up = H[index_up];
    const int h_left = H[index_left];
    const int h_diag = H[index_diag];
    const int current_max = H[*maxPos];
    
    // Calculate all scores at once
    const int up = h_up + gapScore;
    const int left = h_left + gapScore;
    const int diag = h_diag + matchMissmatchScore(a[j-1], b[i-1]);

    // Find max and predecessor using conditional logic
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

    // Only compare with NONE if necessary
    if (max < NONE) {
        max = NONE;
        pred = NONE;
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum position - combine comparison into single branch
    if (max > current_max) {
        *maxPos = index;
    }
}

} // anonymous namespace

void compute(const char* a, const char* b,
            int* H, int* P,
            long long int n, long long int m, long long int& max)
{
    // Process four elements at a time where possible
    const long long int block_size = 4;
    
    for (long long int i = 1; i < n; ++i) {
        long long int j;
        
        // Main loop with 4x unrolling
        for (j = 1; j <= m-block_size; j += block_size) {
            similarityScore(a, b, i, j, m, H, P, &max);
            similarityScore(a, b, i, j+1, m, H, P, &max);
            similarityScore(a, b, i, j+2, m, H, P, &max);
            similarityScore(a, b, i, j+3, m, H, P, &max);
        }
        
        // Handle remaining elements
        for (; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
