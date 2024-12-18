
#include "constants.h"

namespace {

// Inline logic directly within compute for reduced overhead
void compute(const char *a, const char *b, int *H, int *P, long long n, long long m, long long &maxPos) {
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            long long index = m * i + j;

            // Cache frequently accessed data to minimize repetitive memory accesses
            int up = H[index - m] + gapScore;
            int left = H[index - 1] + gapScore;
            int diag = H[index - m - 1] + ((a[j - 1] == b[i - 1]) ? matchScore : missmatchScore);

            int maxVal = NONE;
            int pred = NONE;

            // Evaluate max score with predicted branches
            if (diag > maxVal) [[likely]] {
                maxVal = diag;
                pred = DIAGONAL;
            }
            if (up > maxVal) [[unlikely]] {
                maxVal = up;
                pred = UP;
            }
            if (left > maxVal) [[unlikely]] {
                maxVal = left;
                pred = LEFT;
            }

            // Update matrices
            H[index] = maxVal;
            P[index] = pred;

            // Update maximum position if found
            if (maxVal > H[maxPos]) {
                maxPos = index;
            }

            // Explicit prefetching (compiler-dependent)
            // Some compilers support prefetch intrinsic or specific pragmas for guiding future loads
            // Uncomment lines below if your compiler has support
            // __builtin_prefetch(&H[index + m], 0, 1);  // future m rows ahead in H
            // __builtin_prefetch(&P[index + m], 1, 1);  // future m rows ahead in P
        }
    }
}
