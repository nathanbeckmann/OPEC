#include <iomanip>
#include "optimize.hpp"
#include "opec.hpp"
#include "cartel.hpp"
#include "debug.hpp"

using namespace opec;

void dump(const Market& market, const Solution& solution) {
    std::cout << std::setw(20) << "Countries: ";
    for (auto actor : market.actors) {
        std::cout << actor->name << ", ";
    }
    std::cout << std::endl;
    std::cout << solution << std::endl;

    std::cout << "Marginal revenue (inflated):" << std::endl;
    for (int a = 0; a < market.size(); a++) {
        auto& actor = *market.actors[a];
        auto q = solution.quantities.row(a);
        auto margin = actor.marginalRevenue(solution.prices, q, market, solution.production);
        margin -= actor.marginalCost(q);
        margin = Market::inflate(margin);

        std::cout << std::setw(18) << actor.name << ": " << margin.transpose() << std::endl;
    }
}

int main() {
    std::vector<Actor*> countries;
    for (int c = 0; c < Opec::NumCountries; c++) {
        countries.push_back(&opec::opec[c]);
    }

    std::cout << std::setw(40) << "" << "COMPETITION:" << std::endl;
    Market competitive(countries);
    auto competitiveSolution = solve(competitive);
    dump(competitive, competitiveSolution);

    std::cout << std::endl << std::endl
              << std::setw(40) << "" << "MONOPOLY:" << std::endl;
    Cartel opec("Opec", countries, competitiveSolution);
    std::vector<Actor*> opecv{&opec};
    Market monopoly(opecv);
    auto monopolySolution = solve(monopoly);
    dump(monopoly, monopolySolution);
    opec.dump(monopolySolution);

    return 0;
}
