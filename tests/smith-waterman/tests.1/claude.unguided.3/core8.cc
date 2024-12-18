
#include "constants.h"

namespace {

// CPU cache line size optimization
constexpr int CACHE_LINE_SIZE = 64;

// Force inlining and align the function for better instruction fetch
__attribute__((always_inline, hot, aligned(CACHE_LINE_SIZE)))
static inline int matchMissmatchScore(const char a, const char b) {
    // Branchless score calculation using bitwise operations
    return matchScore - ((a ^ b) != 0) * (matchScore - missmatchScore);
}

// Pack frequently used constants into a cache-aligned struct
struct alignas(CACHE_LINE_SIZE) ScoreConstants {
    const int gapScore_{gapScore};
    const int none_{NONE};
} static const scoreConstants{};

__attribute__((always_inline, hot, aligned(CACHE_LINE_SIZE)))
void similarityScore(const char *__restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* const __restrict__ maxPos) 
{
    // Pre-calculate all indices using single multiplication
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index_up - 1;

    // Multi-level prefetching strategy
    __builtin_prefetch(&H[index + m + 1], 0, 3);
    __builtin_prefetch(&H[index + 2*m], 0, 2);
    __builtin_prefetch(&P[index + m + 1], 1, 3);
    __builtin_prefetch(&a[j + CACHE_LINE_SIZE/2], 0, 0);
    __builtin_prefetch(&b[i + CACHE_LINE_SIZE/2], 0, 0);
    
    // Load all values at once to help instruction pipelining
    const int H_up = H[index_up];
    const int H_left = H[index_left];
    const int H_diag = H[index_diag];
    const char char_a = a[j-1];
    const char char_b = b[i-1];

    // Compute all scores simultaneously
    const int up = H_up + scoreConstants.gapScore_;
    const int left = H_left + scoreConstants.gapScore_;
    const int diag = H_diag + matchMissmatchScore(char_a, char_b);

    // Branchless maximum calculation using data dependencies
    int max = scoreConstants.none_;
    int pred = scoreConstants.none_;
    
    // Optimized comparison chain for better instruction pipelining
    const int diag_gt_max = (diag > max);
    max = diag_gt_max ? diag : max;
    pred = diag_gt_max ? DIAGONAL : pred;
    
    const int up_gt_max = (up > max);
    max = up_gt_max ? up : max;
    pred = up_gt_max ? UP : pred;
    
    const int left_gt_max = (left > max);
    max = left_gt_max ? left : max;
    pred = left_gt_max ? LEFT : pred;

    // Coalesced memory writes
    H[index] = max;
    P[index] = pred;

    // Branchless maximum position update
    const long long int curr_max_pos = *maxPos;
    const int curr_max_score = H[curr_max_pos];
    *maxPos = (max > curr_max_score) ? index : curr_max_pos;
}

} // namespace

__attribute__((noinline, hot))
void compute(const char *__restrict__ a, const char* __restrict__ b, 
            int* __restrict__ H, int* __restrict__ P, 
            const long long int n, const long long int m, long long int& max)
{
    // Optimize tile size for L1 cache
    constexpr int L1_CACHE_SIZE = 32768;  // Typical L1 cache size
    constexpr int MATRIX_ELEMENT_SIZE = sizeof(int);
    constexpr int TILE_SIZE = (L1_CACHE_SIZE / (3 * MATRIX_ELEMENT_SIZE)) / 2;
    
    // Pre-calculate loop bounds
    const long long int n_bound = n - 1;
    const long long int m_bound = m - 1;
    
    // Main computation loop with enhanced tiling
    #pragma GCC ivdep
    for (long long int i_outer = 1; i_outer <= n_bound; i_outer += TILE_SIZE) {
        const long long int i_limit = (i_outer + TILE_SIZE <= n) ? i_outer + TILE_SIZE : n;
        
        #pragma GCC ivdep
        for (long long int j_outer = 1; j_outer <= m_bound; j_outer += TILE_SIZE) {
            const long long int j_limit = (j_outer + TILE_SIZE <= m) ? j_outer + TILE_SIZE : m;
            
            // Process current tile
            #pragma GCC ivdep
            #pragma clang loop vectorize(enable) interleave(enable)
            for (long long int i = i_outer; i < i_limit; ++i) {
                // Inner loop optimization
                #pragma GCC unroll 16
                #pragma clang loop vectorize(enable) interleave(enable)
                #pragma GCC ivdep
                for (long long int j = j_outer; j < j_limit; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
