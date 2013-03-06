#include "market.hpp"

using namespace opec;

double Market::price(int round, double quantity) const {
    auto& demand = world.demand(round);
    return demand.evaluate(quantity);
}

double Market::dPrice(int round, double quantity) const {
    auto& demand = world.demand(round);
    return demand.derivative(quantity);
}

Vector Market::inflate(const Vector& prices) {
    Vector inflated = prices;
    double inflation = 1.;
    for (int r = NumRounds-1; r >= 0; r--) {
        inflated(r) *= inflation;
        inflation *= InterestRate;
    }
    return inflated;
}
