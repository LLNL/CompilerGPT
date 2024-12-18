
#include "constants.h"

namespace {

// Force inlining and align the function for better instruction fetch
__attribute__((always_inline, aligned(64)))
static inline int matchMissmatchScore(const char a, const char b) {
    return (a == b) ? matchScore : missmatchScore;
}

__attribute__((always_inline, aligned(64)))
void similarityScore(const char *__restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* const maxPos)
{
    // Pre-calculate the index to avoid repeated multiplication
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index_up - 1;

    // Aggressive prefetching for next iterations
    __builtin_prefetch(&H[index + m], 0, 3);  // Prefetch for next row
    __builtin_prefetch(&H[index + 1], 0, 3);  // Prefetch for next column
    __builtin_prefetch(&P[index + m], 1, 3);  // Prefetch for next row
    
    // Load all memory values at once to allow for better instruction reordering
    const int H_up = H[index_up];
    const int H_left = H[index_left];
    const int H_diag = H[index_diag];
    const char char_a = a[j-1];
    const char char_b = b[i-1];

    // Calculate scores
    const int up = H_up + gapScore;
    const int left = H_left + gapScore;
    const int diag = H_diag + matchMissmatchScore(char_a, char_b);

    // Branchless max calculation using temporary variables
    int max = NONE;
    int pred = NONE;
    
    // Using temporary variables to help compiler with register allocation
    const int tmp_diag = (diag > max);
    max = tmp_diag ? diag : max;
    pred = tmp_diag ? DIAGONAL : pred;
    
    const int tmp_up = (up > max);
    max = tmp_up ? up : max;
    pred = tmp_up ? UP : pred;
    
    const int tmp_left = (left > max);
    max = tmp_left ? left : max;
    pred = tmp_left ? LEFT : pred;

    // Store results
    H[index] = max;
    P[index] = pred;

    // Branchless max position update
    const long long int curr_max_pos = *maxPos;
    const int curr_max_score = H[curr_max_pos];
    *maxPos = (max > curr_max_score) ? index : curr_max_pos;
}

} // namespace

__attribute__((noinline))
void compute(const char *__restrict__ a, const char* __restrict__ b, 
            int* __restrict__ H, int* __restrict__ P, 
            const long long int n, const long long int m, long long int& max)
{
    // Align data for better memory access
    constexpr int CACHE_LINE = 64;
    constexpr int TILE_SIZE = CACHE_LINE / sizeof(int);  // Optimize for cache line size
    
    // Ensure both dimensions are processed in cache-friendly manner
    #pragma GCC ivdep
    for (long long int i_tile = 1; i_tile < n; i_tile += TILE_SIZE) {
        const long long int i_end = ((i_tile + TILE_SIZE) < n) ? (i_tile + TILE_SIZE) : n;
        
        #pragma GCC ivdep
        for (long long int j_tile = 1; j_tile < m; j_tile += TILE_SIZE) {
            const long long int j_end = ((j_tile + TILE_SIZE) < m) ? (j_tile + TILE_SIZE) : m;
            
            // Process tile
            #pragma GCC ivdep
            for (long long int i = i_tile; i < i_end; ++i) {
                // Manual loop unrolling for innermost loop
                #pragma GCC unroll 8
                for (long long int j = j_tile; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
