
#include <cstdint>  // Include for fixed-width integer types if needed

// Define constants for the algorithm. Replace with actual values as needed.
constexpr int matchScore = 1;
constexpr int missmatchScore = -1;
constexpr int gapScore = -2;

constexpr int NONE = -1;
constexpr int DIAGONAL = 0;
constexpr int UP = 1;
constexpr int LEFT = 2;

namespace {

inline int matchMissmatchScore(const char* a, const char* b, long long i, long long j) {
    return a[j-1] == b[i-1] ? matchScore : missmatchScore;
}

void similarityScore(const char* a, const char* b,
                     long long i, long long j, long long m,
                     int* H, int* P,
                     long long &maxPos)
{
    long long index = m * i + j;

    int up = H[index - m] + gapScore;
    int left = H[index - 1] + gapScore;
    int diag = H[index - m - 1] + matchMissmatchScore(a, b, i, j);

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

    H[index] = max;
    P[index] = pred;

    if (max > H[maxPos]) {
        maxPos = index;
    }
}

} // End of unnamed namespace

// Definition of compute with 'long long' to match typical C++ usage and expected signature.
void compute(const char* a, const char* b, int* H, int* P, long long n, long long m, long long &maxPos)
{
    maxPos = 0;  // Initialize maxPos
    for (long long i = 1; i < n; ++i) {
        for (long long j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, maxPos);
        }
    }
}
