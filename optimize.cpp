#include <iostream>
#include "optimize.hpp"
#include "market.hpp"
#include "misc.hpp"

using namespace opec;

// The constraint for optimization is that the quantity in all rounds
// must add up to the reserve:
//
// q1 + q2 + ... + qN = reserves
//
// Which defines a plane in N-dimensional space (N = NumRounds). In
// order to iterate along this plane towards the optimal solution, we
// need to project the price vector onto the plane.

namespace {

RoundVector roundOrthogonal = RoundVector::Constant(1.).normalized();

Vector project(const Vector& v, const Vector& onto = roundOrthogonal) {
    auto rejection = v.dot(onto) * onto;
    return v - rejection;
}

}

Solution opec::solveCournot(const Market& market) {
    Solution solution(market.size());
    double minDiff, maxDiff;
    const double Delta = 10.;
    // const double TerminationCondition = 0.1;

    // 1. Initial values
    for (int a = 0; a < market.size(); a++) {
        auto& actor = *market.actors[a];
        for (int r = 0; r < NumRounds; r++) {
            solution.quantities(a, r) = actor.reserves / NumRounds;
        }
        solution.quantities(a, NumRounds) = 0.;
    }

    for (int r = 0; r < NumRounds; r++) {
        solution.prices(r) = market.price(r, solution.quantities.col(r));
    }
    solution.prices(NumRounds) = SellOffPrice;

    for (int a = 0; a < market.size(); a++) {
        auto& actor = *market.actors[a];
        solution.values(a) += actor.value(solution.quantities.row(a), solution.prices);
    }

    auto initialValues = solution.values;

    // 2. iterate to fixed point on quantity
    int iter = 0;
    do {
        std::cout << "Iteration " << ++iter << ": " << solution.values.sum()
                  << " (" << (100. * solution.values.sum() / initialValues.sum() - 100.) << "% improved); "
                  << "\tChange: " << minDiff << "-" << maxDiff << std::endl;

        maxDiff = std::numeric_limits<double>::min();
        minDiff = std::numeric_limits<double>::max();
        
        // set quantity based on current prices
        for (int a = 0; a < market.size(); a++) {
            auto& actor = *market.actors[a];
            auto q = solution.quantities.row(a);

            double prevValue = solution.values(a);

            RoundVector costs;
            for (int r = 0; r <= NumRounds; r++) {
                costs(r) = actor.supply().evaluate(q(r));
            }

            // this vector points in the direction of greatest profit;
            // this is where we want to move. we need to scale the
            // prices to reflect inflation.
            RoundVector profit = solution.prices - costs;
            double inflation = 1.;
            for (int r = NumRounds; r >= 0; r--) {
                profit(r) *= inflation;
                inflation *= InterestRate;
            }
            
            RoundVector step = project(profit).normalized() * Delta;
            assert(feq(step.sum(), 0.)); // we're moving along a constant constraint; the net change must be zero

            // enforce capacity constraints...can't produce negative
            // or exceed capacity. if we have violated a constraint,
            // move to the corner case and zero out the step on that
            // dimension
            RoundVector constraints = roundOrthogonal;
            bool satisfied = false;

            while (!satisfied) {
                satisfied = true;
                
                for (int r = 0; r <= NumRounds; r++) {
                    auto nq = q(r) + step(r);
                    if (nq > actor.capacity || nq < 0.) {
                        if (nq > actor.capacity) q(r) = actor.capacity;
                        else q(r) = 0.;
                        
                        constraints(r) = 0.;
                        step(r) = 0.;
                        satisfied = false;
                    }
                }

                // renormalize step to keep reserve constraint ... this
                // will naturally reduce the size of the step if we are in
                // a corner solution, so we don't re-normalize.
                constraints.normalize();
                step = project(step, constraints);
            }

            // make the move!
            q += step;

            // verify constraints
            for (int r = 0; r <= NumRounds; r++) {
                bool satisfied = 0. <= q(r) && q(r) <= actor.capacity;
                if (!satisfied) { std::cerr << 0. << " < " << q(r) << " < " << actor.capacity << std::endl << "round: " << r << ", step: " << step(r) << ", constraint: " << constraints(r) << std::endl; assert(false); }
            }
            if (!approx<double>(q.sum(), actor.reserves, 1000)) { std::cerr << q.sum() << " != " << actor.reserves << std::endl; assert(false); }

            solution.values(a) = actor.value(q, solution.prices);
            auto diff = solution.values(a) - prevValue;
            // assert(diff > 0.);
            maxDiff = std::max(maxDiff, diff);
            minDiff = std::min(minDiff, diff);
        }

        // update prices
        for (int r = 0; r < NumRounds; r++) {
            solution.prices(r) = market.price(r, solution.quantities.col(r));
        }
    } while (iter < 100000); //(maxDiff > TerminationCondition);

    return solution;
}

std::ostream& opec::operator<< (std::ostream& os, const opec::Solution& solution) {
    os << "Prices: " << solution.prices.transpose() << std::endl
       << "Values: " << solution.values.transpose() << std::endl
       << "Quantities: " << std::endl << solution.quantities << std::endl;
    return os;
}
