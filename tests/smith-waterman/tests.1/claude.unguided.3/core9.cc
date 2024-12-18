
#include "constants.h"

namespace {

// Architecture-specific optimizations
constexpr int CACHE_LINE_SIZE = 64;
constexpr int PREFETCH_DISTANCE = 8;
constexpr int VECTOR_SIZE = 16;

// Vectorization-friendly constant struct
struct alignas(CACHE_LINE_SIZE) ScoreConstants {
    const int gapScore_{gapScore};
    const int none_{NONE};
    const int matchScore_{matchScore};
    const int mismatchScore_{missmatchScore};
} static const scoreConstants{};

// Force aggressive inlining and alignment
__attribute__((always_inline, hot, aligned(CACHE_LINE_SIZE)))
static inline int matchMissmatchScore(const char a, const char b) {
    // Optimized branchless calculation using bitwise operations
    const int equal = !(a ^ b);
    return scoreConstants.mismatchScore_ + 
           ((scoreConstants.matchScore_ - scoreConstants.mismatchScore_) & -equal);
}

__attribute__((always_inline, hot, aligned(CACHE_LINE_SIZE)))
static inline void prefetchNextIterations(const char* a, const char* b,
                                        const int* H, const int* P,
                                        const long long int idx,
                                        const long long int m) {
    __builtin_prefetch(&H[idx + m], 0, 3);            // Next row
    __builtin_prefetch(&H[idx + 1], 0, 3);            // Next column
    __builtin_prefetch(&H[idx + m + 1], 0, 3);        // Next diagonal
    __builtin_prefetch(&P[idx + m], 1, 3);            // Next row result
    __builtin_prefetch(&a[idx/m + PREFETCH_DISTANCE], 0, 0);
    __builtin_prefetch(&b[idx%m + PREFETCH_DISTANCE], 0, 0);
}

__attribute__((always_inline, hot, aligned(CACHE_LINE_SIZE)))
void similarityScore(const char *__restrict__ a, const char* __restrict__ b,
                    const long long int i, const long long int j, const long long int m,
                    int* __restrict__ H, int* __restrict__ P,
                    long long int* const __restrict__ maxPos) 
{
    // Pre-calculate indices
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index_up - 1;

    // Prefetch for future iterations
    prefetchNextIterations(a, b, H, P, index, m);
    
    // Load all memory values at once
    const int H_up = H[index_up];
    const int H_left = H[index_left];
    const int H_diag = H[index_diag];
    const char char_a = a[j-1];
    const char char_b = b[i-1];

    // Calculate scores using SIMD-friendly operations
    const int up = H_up + scoreConstants.gapScore_;
    const int left = H_left + scoreConstants.gapScore_;
    const int diag = H_diag + matchMissmatchScore(char_a, char_b);

    // Optimized branchless max calculation
    const int max1 = diag > scoreConstants.none_ ? diag : scoreConstants.none_;
    const int pred1 = diag > scoreConstants.none_ ? DIAGONAL : scoreConstants.none_;
    
    const int max2 = up > max1 ? up : max1;
    const int pred2 = up > max1 ? UP : pred1;
    
    const int max3 = left > max2 ? left : max2;
    const int pred3 = left > max2 ? LEFT : pred2;

    // Coalesced memory writes
    H[index] = max3;
    P[index] = pred3;

    // Optimized max position update
    const long long int curr_max_pos = *maxPos;
    const int curr_max_score = H[curr_max_pos];
    *maxPos = (max3 > curr_max_score) ? index : curr_max_pos;
}

} // namespace

__attribute__((noinline, hot))
void compute(const char *__restrict__ a, const char* __restrict__ b, 
            int* __restrict__ H, int* __restrict__ P, 
            const long long int n, const long long int m, long long int& max)
{
    // Cache-optimized tile sizes
    constexpr int L1_CACHE_SIZE = 32768;
    constexpr int ELEMENTS_PER_TILE = (L1_CACHE_SIZE / sizeof(int)) / 4;
    constexpr int TILE_SIZE = ELEMENTS_PER_TILE - (ELEMENTS_PER_TILE % VECTOR_SIZE);
    
    // Pre-calculate bounds
    const long long int n_bound = n - 1;
    const long long int m_bound = m - 1;
    
    // Main computation with tiling
    #pragma GCC ivdep
    for (long long int i_tile = 1; i_tile <= n_bound; i_tile += TILE_SIZE) {
        const long long int i_end = ((i_tile + TILE_SIZE) <= n) ? (i_tile + TILE_SIZE) : n;
        
        #pragma GCC ivdep
        for (long long int j_tile = 1; j_tile <= m_bound; j_tile += TILE_SIZE) {
            const long long int j_end = ((j_tile + TILE_SIZE) <= m) ? (j_tile + TILE_SIZE) : m;
            
            // Process current tile
            #pragma GCC ivdep
            #pragma clang loop vectorize(enable) interleave(enable)
            for (long long int i = i_tile; i < i_end; ++i) {
                #pragma GCC unroll 16
                #pragma clang loop vectorize(enable) interleave(enable)
                #pragma GCC ivdep
                for (long long int j = j_tile; j < j_end; ++j) {
                    similarityScore(a, b, i, j, m, H, P, &max);
                }
            }
        }
    }
}
