
#include "constants.h"

namespace {

void compute(const char *a, const char *b, int *H, int *P, long long n, long long m, long long &maxPos) {
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            long long index = m * i + j;

            // Cache frequently accessed data
            int up = H[index - m] + gapScore;
            int left = H[index - 1] + gapScore;
            int diag = H[index - m - 1] + ((a[j - 1] == b[i - 1]) ? matchScore : missmatchScore);

            int maxVal = NONE;
            int pred = NONE;

            // Evaluate max score and direction
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

            // Update maximum position if necessary
            if (maxVal > H[maxPos]) {
                maxPos = index;
            }
        }
    }
}

} // end of anonymous namespace
