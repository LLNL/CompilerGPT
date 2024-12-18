
#include "constants.h"

// Helper macros for branch prediction
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace {

inline int matchMissmatchScore(const char* __restrict__ a, const char* __restrict__ b, long long i, long long j) {
    // Inline condition has potential for better branch prediction
    return likely(a[j - 1] == b[i - 1]) ? matchScore : missmatchScore;
}

void similarityScore(const char* __restrict__ a, const char* __restrict__ b, 
                     const long long i, const long long j, const long long m,
                     int* __restrict__ H, int* __restrict__ P, 
                     long long* maxPos)
{
    const long long index = m * i + j;
    const long long index_m = index - m;
    const long long index_m1 = index_m - 1;
    const long long index_1 = index - 1;

    // Cache previous values
    const int up = H[index_m] + gapScore;
    const int left = H[index_1] + gapScore;
    const int diag = H[index_m1] + matchMissmatchScore(a, b, i, j);

    // Use a single max calculation
    int max = diag;
    int pred = DIAGONAL;

    if (unlikely(up > max)) {
        max = up;
        pred = UP;
    }

    if (unlikely(left > max)) {
        max = left;
        pred = LEFT;
    }

    // Store results
    H[index] = max;
    P[index] = pred;

    if (unlikely(max > H[*maxPos])) {
        *maxPos = index;
    }
}

}

void compute(const char* __restrict__ a, const char* __restrict__ b, 
             int* __restrict__ H, int* __restrict__ P, 
             const long long n, const long long m, long long& max)
{
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &max);
        }
    }
}
