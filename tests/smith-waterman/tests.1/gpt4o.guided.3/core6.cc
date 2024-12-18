
#include <cstdint>  // Include for fixed-width integer types

// Assuming these constants are declared elsewhere or defined similarly
constexpr int matchScore = 1;
constexpr int missmatchScore = -1;
constexpr int gapScore = -2;

constexpr int NONE = -1; 
constexpr int DIAGONAL = 0;
constexpr int UP = 1;
constexpr int LEFT = 2;

namespace {

// Ensure inline function signature matches correct parameter types
inline int matchMissmatchScore(const char* a, const char* b, std::int64_t i, std::int64_t j) {
    return a[j-1] == b[i-1] ? matchScore : missmatchScore;
}

// Ensure function signature matches the declaration expected by the linker
void similarityScore(const char* a, const char* b, 
                     std::int64_t i, std::int64_t j, std::int64_t m,
                     int* H, int* P, 
                     std::int64_t &maxPos) 
{
    std::int64_t index = m * i + j;

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

// Ensure compute function signature matches the one requiring definition
void compute(const char* a, const char* b, int* H, int* P, std::int64_t n, std::int64_t m, std::int64_t &maxPos)
{
    maxPos = 0; // Initialize maxPos
    for (std::int64_t i = 1; i < n; ++i) {
        for (std::int64_t j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, maxPos);
        }
    }
}
