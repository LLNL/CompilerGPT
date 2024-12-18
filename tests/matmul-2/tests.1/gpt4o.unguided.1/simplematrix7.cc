
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

#ifdef __AVX__
#include <immintrin.h>
#endif

constexpr int BLOCK_SIZE = 32;
using ValueType = SimpleMatrix::value_type;

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
    int lhsRows = lhs.rows();
    int lhsCols = lhs.columns();
    int rhsCols = rhs.columns();

    if (lhsCols != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    SimpleMatrix res{lhsRows, rhsCols};

#ifdef __AVX__
    // Ensure alignment if using AVX for aligning the matrix data
    if ((reinterpret_cast<uintptr_t>(lhs.data()) % 32 != 0) || (reinterpret_cast<uintptr_t>(rhs.data()) % 32 != 0) || (reinterpret_cast<uintptr_t>(res.data()) % 32 != 0)) {
        throw std::runtime_error{"Data must be 32-byte aligned for AVX"};
    }
#endif

    for (int i = 0; i < lhsRows; i += BLOCK_SIZE)
    {
        for (int j = 0; j < rhsCols; j += BLOCK_SIZE)
        {
            for (int k = 0; k < lhsCols; k += BLOCK_SIZE)
            {
                int iMax = std::min(i + BLOCK_SIZE, lhsRows);
                int jMax = std::min(j + BLOCK_SIZE, rhsCols);
                int kMax = std::min(k + BLOCK_SIZE, lhsCols);

                for (int ib = i; ib < iMax; ++ib)
                {
                    for (int jb = j; jb < jMax; ++jb)
                    {
                        ValueType sum = res(ib, jb);

                        int kb;
#ifdef __AVX__
                        // Preparing to use AVX intrinsics for SIMD vectorization
                        __m256d vecSum = _mm256_setzero_pd();
                        
                        for (kb = k; kb <= kMax - 4; kb += 4)
                        {
                            // Load elements with AVX
                            __m256d lhsVec = _mm256_load_pd(&lhs(ib, kb));
                            __m256d rhsVec = _mm256_load_pd(&rhs(kb, jb));

                            // Perform Fused Multiply-Add
                            vecSum = _mm256_add_pd(vecSum, _mm256_mul_pd(lhsVec, rhsVec));
                        }
                        
                        // Horizontal sum of 4 packed double results
                        __m128d low = _mm256_castpd256_pd128(vecSum);  // extract lower double
                        __m128d high = _mm256_extractf128_pd(vecSum, 1);  // extract higher double
                        __m128d sum2 = _mm_add_pd(low, high);
                        sum = _mm_cvtsd_f64(_mm_hadd_pd(sum2, sum2));

#else
                        for (kb = k; kb <= kMax - 8; kb += 8)
                        {
                            sum += lhs(ib, kb) * rhs(kb, jb);
                            sum += lhs(ib, kb + 1) * rhs(kb + 1, jb);
                            sum += lhs(ib, kb + 2) * rhs(kb + 2, jb);
                            sum += lhs(ib, kb + 3) * rhs(kb + 3, jb);
                            sum += lhs(ib, kb + 4) * rhs(kb + 4, jb);
                            sum += lhs(ib, kb + 5) * rhs(kb + 5, jb);
                            sum += lhs(ib, kb + 6) * rhs(kb + 6, jb);
                            sum += lhs(ib, kb + 7) * rhs(kb + 7, jb);
                        }
#endif
                        for (; kb < kMax; ++kb)
                        {
                            sum += lhs(ib, kb) * rhs(kb, jb);
                        }

                        res(ib, jb) = sum;
                    }
                }
            }
        }
    }

    return res;
}
