#include <iostream>
#include "actor.hpp"
#include "misc.hpp"

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
