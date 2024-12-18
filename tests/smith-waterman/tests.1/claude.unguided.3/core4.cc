
#include "constants.h"

namespace {

// Force inlining and align the function for better instruction fetch
__attribute__((always_inline, hot, aligned(64)))
static inline int matchMissmatchScore(const char a, const char b) {
    // Use XOR to detect equality - potentially faster than comparison
    return (((a ^ b) == 0) ? matchScore : missmatchScore);
}

// Pack frequently used constants into a struct for better cache locality
struct alignas(64) ScoreConstants {
    const int gapScore_{gapScore};
    const int matchScore_{matchScore};
    const int missmatchScore_{missmatchScore};
    const int none_{NONE};
} static const scoreConstants;

__attribute__((always_inline, hot, aligned(64)))
void similarityScore(const char *__restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* const maxPos)
{
    // Pre-calculate all indices at once
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;

    // Aggressive prefetching with specific cache hints
    __builtin_prefetch(&H[index + m + 1], 0, 3);  // Prefetch diagonal for next iteration
    __builtin_prefetch(&H[index + 2*m], 0, 2);    // Prefetch two rows ahead
    __builtin_prefetch(&P[index + m + 1], 1, 3);  // Prefetch P matrix ahead
    
    // Batch load all memory values
    register const int H_up = H[index_up];
    register const int H_left = H[index_left];
    register const int H_diag = H[index_diag];
    register const char char_a = a[j-1];
    register const char char_b = b[i-1];

    // Calculate all scores in parallel if possible
    register const int up = H_up + scoreConstants.gapScore_;
    register const int left = H_left + scoreConstants.gapScore_;
    register const int diag = H_diag + matchMissmatchScore(char_a, char_b);

    // Optimized max calculation using branchless operations
    register int max = scoreConstants.none_;
    register int pred = scoreConstants.none_;
    
    // Using register variables and temporary results
    register const int is_diag_larger = (diag > max);
    max = is_diag_larger ? diag : max;
    pred = is_diag_larger ? DIAGONAL : pred;
    
    register const int is_up_larger = (up > max);
    max = is_up_larger ? up : max;
    pred = is_up_larger ? UP : pred;
    
    register const int is_left_larger = (left > max);
    max = is_left_larger ? left : max;
    pred = is_left_larger ? LEFT : pred;

    // Store results - grouped for better memory access
    H[index] = max;
    P[index] = pred;

    // Branchless max position update using bit operations
    register const long long int curr_max_pos = *maxPos;
    register const int curr_max_score = H[curr_max_pos];
    register const long long int new_pos = (max > curr_max_score) ? index : curr_max_pos;
    *maxPos = new_pos;
}

} // namespace

__attribute__((noinline, hot))
void compute(const char *__restrict__ a, const char* __restrict__ b, 
            int* __restrict__ H, int* __restrict__ P, 
            const long long int n, const long long int m, long long int& max)
{
    // Cache-oriented constants
    constexpr int CACHE_LINE = 64;
    constexpr int INT_SIZE = sizeof(int);
    constexpr int TILE_SIZE = (CACHE_LINE / INT_SIZE) * 2;  // Doubled for better utilization
    
    // Pre-calculate loop bounds
    const long long int n_minus_1 = n - 1;
    const long long int m_minus_1 = m - 1;
    
    // Main computation loop with tiling
    #pragma GCC ivdep
    for (long long int i = 1; i <= n_minus_1; i += TILE_SIZE) {
        const long long int i_end = (i + TILE_SIZE <= n) ? i + TILE_SIZE : n;
        
        #pragma GCC ivdep
        for (long long int j = 1; j <= m_minus_1; j += TILE_SIZE) {
            const long long int j_end = (j + TILE_SIZE <= m) ? j + TILE_SIZE : m;
            
            // Process tile
            #pragma GCC ivdep
            #pragma clang loop vectorize(enable) interleave(enable)
            for (long long int ii = i; ii < i_end; ++ii) {
                // Explicit vectorization hint
                #pragma GCC unroll 16
                #pragma clang loop vectorize(enable) interleave(enable)
                for (long long int jj = j; jj < j_end; ++jj) {
                    similarityScore(a, b, ii, jj, m, H, P, &max);
                }
            }
        }
    }
}
