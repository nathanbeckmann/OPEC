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

RoundVector Actor::costs(const RoundVector& quantity) const {
    RoundVector costs;
    for (int r = 0; r <= NumRounds; r++) {
        costs(r) = supply().evaluate(quantity(r));
    }
    return costs;
}
