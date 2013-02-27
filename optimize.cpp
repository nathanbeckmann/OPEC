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

void verify(const Solution& solution, const Market& market) {
    // check that no country can make a beneficial trade, within a
    // small margin of error
    for (int a = 0; a < market.size(); a++) {
        auto& actor = *market.actors[a];
        auto q = solution.quantities.row(a);
        auto margin = Market::inflate(solution.prices - actor.costs(q));
    
        for (int r = 0; r < NumRounds; r++) {
            // if we aren't producing at quantity in this round, then
            // it must be the case that no other round in which we
            // aren't producing at quantity would give a better payoff
            if (q(r) < actor.capacity) {
                for (int s = 0; s < NumRounds; s++) {
                    assert(q(s) == actor.capacity ||
                           margin(s) <= 1.01 * margin(r));
                }
            }
        }
    }
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
                  << "\t(" << (100. * solution.values.sum() / initialValues.sum() - 100.) << "% improved)"
                  << "\tChange: " << minDiff << "-" << maxDiff << std::endl;

        maxDiff = std::numeric_limits<double>::min();
        minDiff = std::numeric_limits<double>::max();
        
        // set quantity based on current prices
        for (int a = 0; a < market.size(); a++) {
            auto& actor = *market.actors[a];
            auto q = solution.quantities.row(a);

            double prevValue = solution.values(a);

            // this vector points in the direction of greatest profit;
            // this is where we want to move. we need to scale the
            // prices to reflect inflation.
            RoundVector margin = Market::inflate(solution.prices - actor.costs(q));
            
            RoundVector step = project(margin).normalized() * Delta;
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

            actor.isConstrained(q);

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
    } while (iter < 25000); //(maxDiff > TerminationCondition);

    verify(solution, market);

    return solution;
}

std::ostream& opec::operator<< (std::ostream& os, const opec::Solution& solution) {
    os << "Prices:     " << solution.prices.transpose() << std::endl
       << "Inflated:   " << Market::inflate(solution.prices).transpose() << std::endl            
       << "Values:     " << solution.values.transpose() << std::endl
       << "Quantities: " << std::endl << solution.quantities << std::endl;
    return os;
}
