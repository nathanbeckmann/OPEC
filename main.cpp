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
    
    auto sol = solveCournot(market);

    std::cout << "\n\nDone!\n\n";
    std::cout << "Countries: ";
    for (auto actor : market.actors) {
        std::cout << actor->name << ", ";
    }
    std::cout << std::endl;
    std::cout << sol;
    
    return 0;
}
