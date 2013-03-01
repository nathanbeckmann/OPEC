#include <iostream>
#include <iomanip>
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
//
// We also have the capacity constraint:
//
// 0 <= qi <= capacity      for all i
//
// This is enforced by bounding the qi at each step to the constraint,
// and then eliminating constrained dimensions from the plane. The
// step is re-projected onto the plane in the unconstrained dimensions
// to keep the reserve constraint.

RoundVector opec::roundOrthogonal = RoundVector::Constant(1.).normalized();

Vector opec::project(const Vector& v, const Vector& onto) {
    auto rejection = v.dot(onto) * onto;
    return v - rejection;
}

namespace {

void verify(const Solution& solution, const Market& market) {
    // check that no country can make a beneficial trade, within a
    // small margin of error
    for (int a = 0; a < market.size(); a++) {
        auto& actor = *market.actors[a];
        auto q = solution.quantities.row(a);
        auto margin = actor.marginalRevenue(solution.prices, q, market, solution.production);
        margin -= actor.marginalCost(q);
        margin = Market::inflate(margin);
    
        for (int r = 0; r < NumRounds; r++) {
            // if we aren't producing at quantity in this round, then
            // it must be the case that no other round in which we
            // aren't producing at quantity would give a better payoff
            if (q(r) < actor.capacity) {
                for (int s = 0; s < NumRounds; s++) {
                    assert(q(s) == actor.capacity ||
                           std::abs(margin(s)) <= std::abs(1.05 * margin(r)));
                }
            }
        }
    }
}

}

Solution opec::solve(const Market& market) {
    const double Delta = 25.;
    const double TerminationCondition = .01;
    const int ReportInterval = 1000;
    const int TerminationQuantum = 1000;
    const int TerminationIterations = 100000;
    
    Solution solution(market.size());
    Vector diff(market.size());
    double maxDiff = 0.;
    int iter = 0, lastReport;

    // 1. Initial values
    for (int a = 0; a < market.size(); a++) {
        auto& actor = *market.actors[a];
        for (int r = 0; r < NumRounds; r++) {
            solution.quantities(a, r) = actor.reserves / NumRounds;
        }
        solution.quantities(a, NumRounds) = 0.;
    }

    auto update = [&] () {
        for (int r = 0; r <= NumRounds; r++) {
            solution.production(r) = solution.quantities.col(r).sum();
        }

        for (int r = 0; r <= NumRounds; r++) {
            solution.prices(r) = market.price(r, solution.production(r));
        }

        for (int a = 0; a < market.size(); a++) {
            auto& actor = *market.actors[a];
            auto q = solution.quantities.row(a);
            double prevValue = solution.values(a);
            solution.values(a) = actor.value(q, solution.prices).sum();
            diff(a) = solution.values(a) - prevValue;
        }

        maxDiff = std::max(maxDiff, diff.norm());
    };

    update();
    auto initialValues = solution.values;

    auto report = [&] () {
        std::cout << "Iteration " << iter << ": " << solution.values.sum()
        << "\t(" << (100. * solution.values.sum() / initialValues.sum() - 100.) << "% improved)"
        << "\tChange: " << maxDiff << std::endl;
        maxDiff = 0.;
        lastReport = iter;
    };

    // 2. iterate to fixed point on quantity
    do {
        update();

        for (int a = 0; a < market.size(); a++) {
            auto& actor = *market.actors[a];
            auto q = solution.quantities.row(a);
            actor.update(solution, q);
        }

        if (iter % ReportInterval == 0) report();

        // set quantity based on current prices
        for (int a = 0; a < market.size(); a++) {
            auto& actor = *market.actors[a];
            auto q = solution.quantities.row(a);

            // this vector points in the direction of greatest profit;
            // this is where we want to move. we need to scale the
            // prices to reflect inflation.
            auto margin = actor.marginalRevenue(solution.prices, q, market, solution.production);
            margin -= actor.marginalCost(q);
            margin = Market::inflate(margin);
            
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
                // a corner solution, so we don't re-scale by Delta.
                constraints.normalize();
                step = project(step, constraints);
            }

            // make the move!
            q += step;

            actor.isConstrained(q);
        }

    } while ((++iter < TerminationIterations)
             && ((iter - lastReport) < TerminationQuantum || maxDiff > TerminationCondition));

    report();
    verify(solution, market);

    return solution;
}

std::ostream& opec::operator<< (std::ostream& os, const opec::Solution& solution) {
    Vector countryProduction = solution.quantities.rowwise().sum();
    
    os << std::setw(20) << "Prices: " << solution.prices.transpose() << std::endl
       << std::setw(20) << "Inflated: " << Market::inflate(solution.prices).transpose() << std::endl            
       << std::setw(20) << "Values: " << solution.values.transpose() << std::endl
       << std::setw(20) << "Prod (round): " << solution.production.transpose() << std::endl
       << std::setw(20) << "Prod (country): " << countryProduction.transpose() << std::endl
       << "Quantities: " << std::endl << solution.quantities << std::endl;
    return os;
}
