#pragma once

#include <vector>
#include <Eigen/Matrix>
#include "opec.hpp"
#include "constants.hpp"

namespace opec {
namespace optimize {

typedef Eigen::Vector<double, NumRounds+1, 1> RoundVector;
typedef Eigen::VectorXd Vector;
typedef Eigen::MatrixXXd Matrix;

class Market;

struct Solution {
        Solution(int nactors) : quantities(nactors), values(nactor) {
            quantities.setZero();
            prices.setZero();
            values.setZero();
        }
        
        Eigen::Matrix<double, Dynamic, NumRounds+1> quantities;
        RoundVector prices;
        Vector values;
};

Solution solveCournot(const Market& market);

}
}
