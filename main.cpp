#include <iomanip>
#include "optimize.hpp"
#include "opec.hpp"
#include "debug.hpp"

using namespace opec;

int main() {
    std::vector<const Actor*> actors;
    for (int c = 0; c < Opec::NumCountries; c++) {
        actors.push_back(&opec::opec[c]);
    }
    
    Market market(std::move(actors));
    
    auto solution = solve(market);

    std::cout << "\n\nDone!\n\n";
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

    return 0;
}
