#include "market.hpp"

using namespace opec;

double Market::price(int round, Vector quantity) const {
    auto& demand = world.demand(round);
    auto production = quantity.sum();
    return demand.evaluate(production);
}

Vector Market::inflate(const Vector& prices) {
    Vector inflated = prices;
    double inflation = 1.;
    for (int r = NumRounds; r >= 0; r--) {
        inflated(r) *= inflation;
        inflation *= InterestRate;
    }
    return inflated;
}
