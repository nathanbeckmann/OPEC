#include <iostream>
#include "actor.hpp"
#include "misc.hpp"
#include "market.hpp"

using namespace opec;

double Actor::value(const RoundVector& quantity, const RoundVector& prices) const {
    double v = 0.;
    double res = reserves;

    for (int r = 0; r < NumRounds; r++) {
        auto q = quantity(r);
        assert(q <= 1.01 * capacity);
        assert(q <= 1.01 * res);
        
        v += prices(r) * q - supply().integrate(q);
        v *= InterestRate;

        res -= q;
    }

    v += quantity(NumRounds) * SellOffPrice;

    return v;
}

void Actor::update(Solution& solution, RowRoundVectorRef quantities) const {
    // renormalize quantities to reserves
    quantities *= reserves / quantities.sum();
}

void Actor::isConstrained(const RoundVector& quantity) const {
    for (int r = 0; r <= NumRounds; r++) {
        bool satisfied = 0. <= quantity(r) && quantity(r) <= capacity;
        if (!satisfied) {
            std::cerr << 0. << " < " << quantity(r) << " < " << capacity << std::endl;
                      // << "round: " << r << ", step: " << step(r) << ", constraint: " << constraints(r) << std::endl;
            assert(false);
        }
    }
    if (!approx<double>(quantity.sum(), reserves, 1000)) {
        std::cerr << quantity.sum() << " != " << reserves << std::endl;
        assert(false);
    }
}

// Marginal cost is trivial read off the supply function.
// 
RoundVector Actor::marginalCost(const RoundVector& quantity) const {
    RoundVector mc;
    for (int r = 0; r <= NumRounds; r++) {
        mc(r) = supply().evaluate(quantity(r));
    }
    return mc;
}

// Compute the marginal revenue over all rounds for this actor, given
// some prices, the actor's quantity, the market demand, and total
// (OPEC) production in each round.
//
// Revenue for country 1 in any given round is:
//
// R1 = p q1
//
// Where p(q) = p(q1 + q1 + ... qN) is a function of total production.
//
// Therefore country 1's marginal revenue is given by:
//
// dR1   dp         dq1   dp
// --- = --- q1 + p --- = --- q1 + p
// dq1   dq1        dq1   dq1
//
// And by the chain rule
//
// dp    dp   dq    dp   d(q1+q2+...+qN)   dp
// --- = -- x --- = -- x --------------- = --
// dq1   dq   dq1   dq        dq1          dq
//
// Finally
//
// dR1   dp
// --- = -- q1 + p
// dq1   dq
//
bool debug;

RoundVector Actor::marginalRevenue(const RoundVector& prices,
                                   const RoundVector& quantity,
                                   const Market& market,
                                   const RoundVector& production) const {
    RoundVector mr;
    for (int r = 0; r <= NumRounds; r++) {
        mr(r) = prices(r) + quantity(r) * market.dPrice(r, production(r));
    }
    return mr;
}
