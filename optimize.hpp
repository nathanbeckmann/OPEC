#pragma once

#include <iostream>
#include <vector>
#include <Eigen/Core>
#include "constants.hpp"

namespace opec {

typedef Eigen::Matrix<double, NumRounds+1, 1> RoundVector;
typedef Eigen::Matrix<double, Eigen::Dynamic, NumRounds+1> RoundMatrix;
typedef Eigen::Block<RoundMatrix, 1, NumRounds+1> RowRoundVectorRef;
typedef Eigen::VectorXd Vector;
typedef Eigen::MatrixXd Matrix;

class Market;

struct Solution {
        Solution(int nactors)
                : quantities(nactors, NumRounds+1)
                , values(nactors) {
            quantities.setZero();
            prices.setZero();
            values.setZero();
        }
        
        RoundMatrix quantities;
        RoundVector production;         // aka, column-wise sum of quantities
        RoundVector prices;
        Vector values;
};

std::ostream& operator<< (std::ostream& os, const Solution& sol);

Solution solve(const Market& market);

}
