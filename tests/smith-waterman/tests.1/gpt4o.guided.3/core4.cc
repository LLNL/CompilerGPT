
#include <iostream> // Needed for basic I/O operations (if needed for debugging)
#include <cstdint>  // Include for fixed-width integer types

// Assuming these constants are needed based on your problem context. Define reasonable values if not present.
constexpr int matchScore = 1;
constexpr int missmatchScore = -1;
constexpr int gapScore = -2;

constexpr int NONE = -1;  // Change this to whatever makes sense contextually
constexpr int DIAGONAL = 0;
constexpr int UP = 1;
constexpr int LEFT = 2;

namespace {

// Inline function to calculate match/mismatch score
inline int matchMissmatchScore(const char *a, const char* b, std::int64_t i, std::int64_t j) {
    return a[j-1] == b[i-1] ? matchScore : missmatchScore;
}

// Function to calculate similarity score
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

} // unnamed namespace

// Compute function to iterate over the matrix and fill scores
void compute(const char* a, const char* b, int* H, int* P, std::int64_t n, std::int64_t m, std::int64_t &maxPos)
{
    maxPos = 0; // Initialize maxPos to ensure it's set correctly
    for (std::int64_t i = 1; i < n; ++i) {
        for (std::int64_t j = 1; j < m; ++j) {
            similarityScore(a, b, i, j, m, H, P, maxPos);
        }
    }
}

int main() {
    // Small test case setup
    const char* a = "AGCT";
    const char* b = "AGTT";

    const std::int64_t n = 5;
    const std::int64_t m = 5;

    int H[n * m] = {0};
    int P[n * m] = {0};
    std::int64_t maxPos = 0;

    compute(a, b, H, P, n, m, maxPos);

    // Debug output for verification
    std::cout << "Max Position: " << maxPos << std::endl;
    return 0;
}

