
#include "constants.h"

namespace {

// Look-up table for match/mismatch scores
static int score_table[256][256];

// Initialize score table once
struct ScoreTableInitializer {
    ScoreTableInitializer() {
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                score_table[i][j] = (i == j) ? matchScore : missmatchScore;
            }
        }
    }
} score_table_initializer;

#define PREFETCH_DISTANCE 8

void similarityScore(const char* a, const char* b,
                    const long long int i, const long long int j, 
                    const long long int m,
                    int* __restrict__ H, 
                    int* __restrict__ P,
                    long long int* const maxPos)
{
    // Pre-calculate indices
    const long long int index = m * i + j;
    const long long int index_up = index - m;
    const long long int index_diag = index_up - 1;
    
    // Prefetch next iterations
    __builtin_prefetch(&H[index + PREFETCH_DISTANCE]);
    __builtin_prefetch(&P[index + PREFETCH_DISTANCE]);
    
    // Load all values at once to avoid memory stalls
    const int H_up = H[index_up];
    const int H_left = H[index - 1];
    const int H_diag = H[index_diag];
    const int score = score_table[(unsigned char)a[j-1]][(unsigned char)b[i-1]];
    
    // Calculate scores - compiler can parallelize these
    const int up = H_up + gapScore;
    const int left = H_left + gapScore;
    const int diag = H_diag + score;

    // Branchless max calculation using ternary operators
    int max = diag;
    int pred = DIAGONAL;
    
    if (up > max) {
        max = up;
        pred = UP;
    }
    
    if (left > max) {
        max = left;
        pred = LEFT;
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    // Update maximum position if necessary
    if (max > H[*maxPos]) {
        *maxPos = index;
    }
}

} // anonymous namespace

void compute(const char* const a, const char* const b,
            int* const H, int* const P,
            const long long int n, const long long int m, long long int& max)
{
    // Calculate loop bounds once
    const long long int imax = n - 1;
    const long long int jmax = m - 1;
    const long long int jblock = 4; // Process 4 elements at a time
    
    // Main computation loops with aggressive unrolling
    for (long long int i = 1; i <= imax; ++i) {
        // Handle blocks of 4
        long long int j;
        for (j = 1; j <= jmax - (jblock-1); j += jblock) {
            similarityScore(a, b, i, j, m, H, P, &max);
            similarityScore(a, b, i, j+1, m, H, P, &max);
            similarityScore(a, b, i, j+2, m, H, P, &max);
            similarityScore(a, b, i, j+3, m, H, P, &max);
        }
        // Handle remaining elements
        for (; j <= jmax; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
