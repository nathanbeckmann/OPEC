#include "optimize.hpp"
#include "market.hpp"

using namespace opec;

// The constraint for optimization is that the quantity in all rounds
// must add up to the reserve:
//
// q1 + q2 + ... + qN = reserves
//
// Which defines a plane in N-dimensional space (N = NumRounds). In
// order to iterate along this plane towards the optimal solution, we
// need to project the price vector onto the plane.

typedef Eigen::Matrix<double, NumRounds, NumRounds> RoundMatrix;

const Vector roundOrthogonal = RoundMatrix::Identity()::unitOrthogonal();

Vector project(const Vector& v) {
    auto rejection = v.dot(roundOrthogonal) * roundOrthogonal;
    return v - rejection;
}

Solution solveCournot(const Market& market) {
    Solution solution(market.size());
    double maxDiff = 0;
    const Delta = 10.;
    const TerminationCondition = 0.1;

    // 1. Initial values
    for (int a = 0; a < market.size(); a++) {
        auto& actor = market.actors[a];
        for (int r = 0; r < NumRounds; r++) {
            solution.quantities(a, r) = actor.reserves / NumRounds;
        }
        solution.quantities(a, NumRounds) = 0.;
    }

    for (int r = 0; r < NumRounds; r++) {
        solution.prices(r) = market.price(r, quantities.col(r));
    }
    solution.prices(NumRounds) = SellOffPrice;

    for (int a = 0; a < market.size(); a++) {
        auto& actor = market.actors[a];
        for (int r = 0; r < NumRounds; r++) {
            solution.values(a) += actor.value(quantities.row(a), solutions.prices);
        }
    }
    
    // 2. iterate to fixed point on quantity
    do {
        // set quantity based on current prices
        for (int a = 0; a < market.size(); a++) {
            auto& actor = *market.actors[a];
            auto& q = solution.quantities.row(a);

            double prevValue = solution.values(a);

            RoundVector costs;
            for (int r = 0; r < NumRounds+1; r++) {
                costs(r) = actor.supply()(q(r));
            }

            // this vector points in the direction of greatest profit;
            // this is where we want to move
            auto profit = solution.prices - costs;
            auto direction = project(profit).normalize();
            assert(approx(direction.sum(), 0., 1));

            quantities.row(a) += direction * Delta;

            // constrain quantities within capacity...
            // place excess production into reserves
            for (int r = 0; r < NumRounds; r++) {
                if (q(r) > actor.capacity) {
                    q(NumRounds) += q(r) - actor.capacity;
                    q(r) = actor.capacity;
                }
            }
            assert(approx(q.sum(), actor.reserves, 1));

            values(a) = actor.value(q, solution.prices);
            auto diff = values(a) - prevValue;
            assert(diff > 0.);
            maxDiff = std::max(maxDiff, diff);
        }

        // update prices
        for (int r = 0; r < NumRounds; r++) {
            solution.prices(r) = market.price(r, quantities.col(r));
        }
    } while (maxDiff > TerminationCondition);

    return solution;
}
