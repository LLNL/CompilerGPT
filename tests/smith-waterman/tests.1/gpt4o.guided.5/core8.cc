
#include "constants.h"
#include <algorithm>

namespace
{
    inline int matchMissmatchScore(char a, char b) {
        return (a == b) ? matchScore : missmatchScore;
    }

    void similarityScore(const char *a, const char *b, long long int i, long long int j, long long int m,
                         int *H, int *P, long long int *maxPos) 
    {
        const long long int index = m * i + j;

        int scoreDiag = matchMissmatchScore(a[j - 1], b[i - 1]);
        const int diag = H[index - m - 1] + scoreDiag;
        const int up = H[index - m] + gapScore;
        const int left = H[index - 1] + gapScore;

        // Using a max initializer for scores
        int max_val = std::max({diag, up, left});
        int pred = (max_val == diag) ? DIAGONAL : (max_val == up) ? UP : LEFT;

        H[index] = max_val;
        P[index] = pred;

        // Cache `H[*maxPos]` to minimize repeated load
        int currentMax = H[*maxPos];
        if (max_val > currentMax) {
            *maxPos = index;
        }
    }
}

void compute(const char *a, const char *b, int *H, int *P, long long int n, long long int m, long long int &max)
{
    long long int maxPos = 0;
    for (long long int i = 1; i < n; ++i) {
        for (long long int j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
    max = maxPos; // Return the highest scoring position's index
}
