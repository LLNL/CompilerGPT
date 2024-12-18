
#include "constants.h"

namespace
{
  
inline int matchMissmatchScore(const char *a, const char* b, long long int i, long long int j) {
    // Help compiler with pointer alignment and restrict
    const char* __restrict__ a_ptr = a + (j-1);
    const char* __restrict__ b_ptr = b + (i-1);
    return (*a_ptr == *b_ptr) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                    long long int i, long long int j, long long int m,
                    int* __restrict__ H, int* __restrict__ P, 
                    long long int* __restrict__ maxPos) 
{
    // Pre-calculate all indices to help register allocation
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_left = index - 1;
    const long long int index_diag = index - m - 1;

    // Load values into registers once
    const int max_pos_idx = *maxPos;
    const int curr_max = H[max_pos_idx];
    
    // Calculate all scores
    const int up = H[index_up] + gapScore;
    const int left = H[index_left] + gapScore;
    const int diag = H[index_diag] + matchMissmatchScore(a, b, i, j);

    // Use registers for intermediate results
    int max = diag;
    int pred = DIAGONAL;
    
    // Branchless max calculation might help vectorization
    const bool up_greater = up > max;
    max = up_greater ? up : max;
    pred = up_greater ? UP : pred;
    
    const bool left_greater = left > max;
    max = left_greater ? left : max;
    pred = left_greater ? LEFT : pred;

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maxPos if needed
    if (max > curr_max) {
        *maxPos = index;
    }
}

}  // end anonymous namespace

void compute(const char *a, const char* b, int* __restrict__ H, int* __restrict__ P, 
            long long int n, long long int m, long long int& max)
{
    // Pre-calculate loop bounds
    const long long int row_bound = n;
    const long long int col_bound = m;
    
    #ifdef __clang__
    #pragma clang loop vectorize(enable) interleave(enable)
    #endif
    for (long long int i = 1; i < row_bound; ++i) {
        #ifdef __clang__
        #pragma clang loop vectorize(enable) interleave(enable)
        #endif
        for (long long int j = 1; j < col_bound; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
