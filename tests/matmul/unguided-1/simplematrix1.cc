
SimpleMatrix operator*(const SimpleMatrix& lhs, the SimpleMatrix& rhs)
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    int lhsRows = lhs.rows();
    int rhsCols = rhs.columns();
    int innerDim = lhs.columns();

    SimpleMatrix res{lhsRows, rhsCols};

    // Calculate products
    for (int i = 0; i < lhsRows; ++i)
    {
        for (int k = 0; k < innerDim; ++k)
        {
            auto temp = lhs(i, k);
            for (int j = 0; j < rhsCols; ++j)
            {
                res(i, j) += temp * rhs(k, j);
            }
        }
    }

    return res;
}
