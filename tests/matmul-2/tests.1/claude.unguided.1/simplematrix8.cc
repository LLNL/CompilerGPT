
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Fast initialization
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            res(i, j) = 0;
        }
    }

    // Process matrix in 2x2 blocks for the main dimensions
    const int Mlim = M & ~1;  // Round down to multiple of 2
    const int Nlim = N & ~1;
    const int Klim = K & ~7;  // Round down to multiple of 8

    for (int i = 0; i < Mlim; i += 2) {
        for (int j = 0; j < Nlim; j += 2) {
            // Four accumulators for 2x2 block
            SimpleMatrix::value_type sum00 = 0, sum01 = 0;
            SimpleMatrix::value_type sum10 = 0, sum11 = 0;
            
            // Secondary accumulators to break dependencies
            SimpleMatrix::value_type temp00 = 0, temp01 = 0;
            SimpleMatrix::value_type temp10 = 0, temp11 = 0;

            // Process K dimension in chunks of 8
            for (int k = 0; k < Klim; k += 8) {
                // First chunk of 4 elements
                {
                    const SimpleMatrix::value_type lhs00 = lhs(i, k);
                    const SimpleMatrix::value_type lhs01 = lhs(i, k+1);
                    const SimpleMatrix::value_type lhs02 = lhs(i, k+2);
                    const SimpleMatrix::value_type lhs03 = lhs(i, k+3);
                    const SimpleMatrix::value_type lhs10 = lhs(i+1, k);
                    const SimpleMatrix::value_type lhs11 = lhs(i+1, k+1);
                    const SimpleMatrix::value_type lhs12 = lhs(i+1, k+2);
                    const SimpleMatrix::value_type lhs13 = lhs(i+1, k+3);

                    const SimpleMatrix::value_type rhs00 = rhs(k, j);
                    const SimpleMatrix::value_type rhs01 = rhs(k, j+1);
                    const SimpleMatrix::value_type rhs10 = rhs(k+1, j);
                    const SimpleMatrix::value_type rhs11 = rhs(k+1, j+1);
                    const SimpleMatrix::value_type rhs20 = rhs(k+2, j);
                    const SimpleMatrix::value_type rhs21 = rhs(k+2, j+1);
                    const SimpleMatrix::value_type rhs30 = rhs(k+3, j);
                    const SimpleMatrix::value_type rhs31 = rhs(k+3, j+1);

                    sum00 += lhs00 * rhs00 + lhs01 * rhs10;
                    sum01 += lhs00 * rhs01 + lhs01 * rhs11;
                    sum10 += lhs10 * rhs00 + lhs11 * rhs10;
                    sum11 += lhs10 * rhs01 + lhs11 * rhs11;

                    temp00 += lhs02 * rhs20 + lhs03 * rhs30;
                    temp01 += lhs02 * rhs21 + lhs03 * rhs31;
                    temp10 += lhs12 * rhs20 + lhs13 * rhs30;
                    temp11 += lhs12 * rhs21 + lhs13 * rhs31;
                }

                // Second chunk of 4 elements
                {
                    const SimpleMatrix::value_type lhs04 = lhs(i, k+4);
                    const SimpleMatrix::value_type lhs05 = lhs(i, k+5);
                    const SimpleMatrix::value_type lhs06 = lhs(i, k+6);
                    const SimpleMatrix::value_type lhs07 = lhs(i, k+7);
                    const SimpleMatrix::value_type lhs14 = lhs(i+1, k+4);
                    const SimpleMatrix::value_type lhs15 = lhs(i+1, k+5);
                    const SimpleMatrix::value_type lhs16 = lhs(i+1, k+6);
                    const SimpleMatrix::value_type lhs17 = lhs(i+1, k+7);

                    const SimpleMatrix::value_type rhs40 = rhs(k+4, j);
                    const SimpleMatrix::value_type rhs41 = rhs(k+4, j+1);
                    const SimpleMatrix::value_type rhs50 = rhs(k+5, j);
                    const SimpleMatrix::value_type rhs51 = rhs(k+5, j+1);
                    const SimpleMatrix::value_type rhs60 = rhs(k+6, j);
                    const SimpleMatrix::value_type rhs61 = rhs(k+6, j+1);
                    const SimpleMatrix::value_type rhs70 = rhs(k+7, j);
                    const SimpleMatrix::value_type rhs71 = rhs(k+7, j+1);

                    sum00 += lhs04 * rhs40 + lhs05 * rhs50;
                    sum01 += lhs04 * rhs41 + lhs05 * rhs51;
                    sum10 += lhs14 * rhs40 + lhs15 * rhs50;
                    sum11 += lhs14 * rhs41 + lhs15 * rhs51;

                    temp00 += lhs06 * rhs60 + lhs07 * rhs70;
                    temp01 += lhs06 * rhs61 + lhs07 * rhs71;
                    temp10 += lhs16 * rhs60 + lhs17 * rhs70;
                    temp11 += lhs16 * rhs61 + lhs17 * rhs71;
                }
            }

            // Handle remaining K elements
            for (int k = Klim; k < K; k++) {
                const SimpleMatrix::value_type lhs0 = lhs(i, k);
                const SimpleMatrix::value_type lhs1 = lhs(i+1, k);
                const SimpleMatrix::value_type rhs0 = rhs(k, j);
                const SimpleMatrix::value_type rhs1 = rhs(k, j+1);
                
                sum00 += lhs0 * rhs0;
                sum01 += lhs0 * rhs1;
                sum10 += lhs1 * rhs0;
                sum11 += lhs1 * rhs1;
            }

            // Combine accumulators and store results
            res(i, j) = sum00 + temp00;
            res(i, j+1) = sum01 + temp01;
            res(i+1, j) = sum10 + temp10;
            res(i+1, j+1) = sum11 + temp11;
        }
    }

    // Handle remaining rows and columns
    for (int i = Mlim; i < M; i++) {
        for (int j = 0; j < N; j++) {
            SimpleMatrix::value_type sum = 0;
            for (int k = 0; k < K; k++) {
                sum += lhs(i, k) * rhs(k, j);
            }
            res(i, j) = sum;
        }
    }
    
    for (int i = 0; i < Mlim; i++) {
        for (int j = Nlim; j < N; j++) {
            SimpleMatrix::value_type sum = 0;
            for (int k = 0; k < K; k++) {
                sum += lhs(i, k) * rhs(k, j);
            }
            res(i, j) = sum;
        }
    }

    return res;
}
