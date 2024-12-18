
namespace {

inline int matchMissmatchScore(char a, char b) {
    return (a == b) ? matchScore : missmatchScore;
}

void similarityScore(const char *a, const char* b, 
                    long long int i, long long int j, long long int m,
                    int* H, int* P, 
                    int& maxScore, long long int& maxPos) 
{
    const long long int index = m * i + j;
    const long long int diag_index = index - m - 1;
    const long long int up_index = index - m;
    const long long int left_index = index - 1;
    
    const int up = H[up_index] + gapScore;
    const int left = H[left_index] + gapScore;
    const int diag = H[diag_index] + matchMissmatchScore(b[i-1], a[j-1]);

    int max = NONE;
    int pred = NONE;
    
    // Branchless version using conditional moves
    max = diag;
    pred = DIAGONAL;
    
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

    if (max > maxScore) {
        maxScore = max;
        maxPos = index;
    }
}

} // namespace

void compute(const char *a, const char* b, int* H, int* P, 
            long long int n, long long int m, long long int& maxPos)
{
    int maxScore = H[maxPos]; // Cache the value
    
    // Loop tiling for better cache utilization
    const int TILE_SIZE = 64;
    
    for (long long int ii = 1; ii < n; ii += TILE_SIZE) {
        const long long int iEnd = std::min(ii + TILE_SIZE, n);
        for (long long int jj = 1; jj < m; jj += TILE_SIZE) {
            const long long int jEnd = std::min(jj + TILE_SIZE, m);
            
            for (long long int i = ii; i < iEnd; ++i) {
                for (long long int j = jj; j < jEnd; ++j) {
                    similarityScore(a, b, i, j, m, H, P, maxScore, maxPos);
                }
            }
        }
    }
}
