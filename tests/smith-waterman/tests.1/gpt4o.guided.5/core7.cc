
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

        const int diag = H[index - m - 1] + matchMissmatchScore(a[j - 1], b[i - 1]);
        const int up = H[index - m] + gapScore;
        const int left = H[index - 1] + gapScore;

        int max_val = NONE;
        int pred = NONE;

        if (diag > max_val) {
            max_val = diag;
            pred = DIAGONAL;
        }
        if (up > max_val) {
            max_val = up;
            pred = UP;
        }
        if (left > max_val) {
            max_val = left;
            pred = LEFT;
        }

        H[index] = max_val;
        P[index] = pred;

        if (max_val > H[*maxPos]) {
            *maxPos = index;
        }
    }
}

void compute(const char *a, const char *b, int *H, int *P, long long int n, long long int m, long long int &max)
{
    long long int maxPos = 0;
    for (long long int i = 1; i < n; ++i)
    {
        for (long long int j = 1; j < m; ++j)
        {
            similarityScore(a, b, i, j, m, H, P, &maxPos);
        }
    }
    max = maxPos;
}
