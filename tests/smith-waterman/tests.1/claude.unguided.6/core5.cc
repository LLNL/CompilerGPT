
#include "constants.h"

namespace {

// Pre-compute gap scores since they're constant
static const int GAP_SCORE = gapScore;
static const int MATCH = matchScore;
static const int MISMATCH = missmatchScore;

__attribute__((always_inline))
inline int matchMissmatchScore(const char a_char, const char b_char) {
    return (a_char == b_char) ? MATCH : MISMATCH;
}

__attribute__((always_inline))
inline void similarityScore(const char* __restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P, 
                    long long int* __restrict__ maxPos)
{
    // Pre-calculate indices
    const long long int index = m * i + j;
    const long long int idx_up = index - m;
    const long long int idx_left = index - 1;
    const long long int idx_diag = idx_up - 1;

    // Load all values at once to help prefetcher
    const int h_up = H[idx_up];
    const int h_left = H[idx_left];
    const int h_diag = H[idx_diag];
    
    // Calculate scores
    const int up = h_up + GAP_SCORE;
    const int left = h_left + GAP_SCORE;
    const int diag = h_diag + matchMissmatchScore(a[j-1], b[i-1]);

    // Initialize with diagonal case
    int max = diag;
    int pred = DIAGONAL;
    
    // Compare with up
    if (up > max) {
        max = up;
        pred = UP;
    }
    
    // Compare with left
    if (left > max) {
        max = left;
        pred = LEFT;
    }

    // Ensure max is at least NONE
    if (max < NONE) {
        max = NONE;
        pred = NONE;
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum position if needed
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // namespace

void compute(const char* __restrict__ a, const char* __restrict__ b,
            int* __restrict__ H, int* __restrict__ P,
            const long long int n, const long long int m, long long int& max)
{
    // Optimize block size for L1 cache
    constexpr long long int BLOCK_SIZE = 32;
    
    // Main computation loop with blocking
    #define MIN(a,b) ((a) < (b) ? (a) : (b))
    
    // Process the matrix in blocks
    for (long long int ii = 1; ii < n; ii += BLOCK_SIZE) {
        const long long int i_end = MIN(ii + BLOCK_SIZE, n);
        
        for (long long int jj = 1; jj < m; jj += BLOCK_SIZE) {
            const long long int j_end = MIN(jj + BLOCK_SIZE, m);
            
            // Process current block with loop unrolling
            for (long long int i = ii; i < i_end; ++i) {
                long long int j = jj;
                
                // Manual loop unrolling for better instruction pipelining
                for (; j + 3 < j_end; j += 4) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                    similarityScore(a, b, i, j+1, m, H, P, &max);
                    similarityScore(a, b, i, j+2, m, H, P, &max);
                    similarityScore(a, b, i, j+3, m, H, P, &max);
                }
                
                // Handle remaining elements
                for (; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
    #undef MIN
}
