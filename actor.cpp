#include "actor.hpp"

using namespace opec;

double Actor::value(const RoundVector& quantity, const RoundVector& prices) const {
    double v = 0.;
    int res = reserves;

    for (int r = 0; r < NumRounds; r++) {
        auto q = quantity(r);
        assert(q < capacity);
        assert(q < reserves);
        
        auto margin = prices(q) - supply()(q);
        v += q * margin;
        v *= InterestRate;

        res -= q;
    }

    assert(res == quantity(NumRounds));
    v += quantity(NumRounds) * SellOffPrice;

    return v;
}
