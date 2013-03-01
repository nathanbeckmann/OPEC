#include <algorithm>
#include <iomanip>

#include "cartel.hpp"
#include "misc.hpp"

using namespace opec;

Cartel::Cartel(string name, const std::vector<Actor*>& _actors, Solution& _competitiveSolution)
        : Actor(name)
        , actors(_actors)
        , piecewise(NumRounds+1)
        , quantities(size(), NumRounds+1)
        , competitiveSolution(_competitiveSolution)
        , actorReserves(size())
        , actorCapacity(size()) {
    // assume constant supply curve from each actor for now
    // std::sort(actors.begin(), actors.end(),
    //           [] (const Actor* a, const Actor* b) { return a->supply(0).evaluate(0) > b->supply(0).evaluate(0); });

    for (int a = 0; a < size(); a++) {
        actorReserves(a) = actors[a]->reserves;
        actorCapacity(a) = actors[a]->capacity;
    }
    reserves = actorReserves.sum();
    capacity = actorCapacity.sum();

    for (int r = 0; r <= NumRounds; r++) {
        for (int a = 0; a < size(); a++) {
            auto& actor = *actors[a];
            auto mc = actor.supply(r).evaluate(0);
            assert(actor.supply(r).evaluate(0) == actor.supply(r).evaluate(1000));
            auto prod = actor.reserves / NumRounds;
            piecewise[r].add(mc, prod);
        }
    }
}

// Assign quantities to each member country such that all country's
// gain from collusion over competition is the same. Let:
//
// PM[i] = Monopoly profits for country i
// PC[i] = Competitive profits for country i
//
// alpha[i] = PM[i] / PC[i]
//
// We want:
//
// alpha[i] = alpha[j] for all i, j
//
// PM = total monopoly profits
// PC = total competitive profits
//
// beta[i] = PC[i] / PC
//
// Assign:
//
// PM[i] = beta[i] * PM = PM * PC[i] / PC
//
// This means that we are done because for all i:
//
// alpha[i] = PM[i] / PC[i] = PM * PC[i] / PC * PC[i] = PM / PC
//
// So how can we achieve this target PM[i]? Set:
//
// AR[i] = avg. revenue per quantity for country i under monopoly
// res[i] = country i's reserves
//
// Then:
// 
// PM[i] = AR[i] * res[i]       <=>       AR[i] = PM[i] / res[i]
//
// This function iteratively assign reserves to each country such that
// they end up with AR[i] = PM[i] / res[i].
//
// This procedure faces these constraints:
//   - Production per round.
//   - Capacity per country.
//   - Reserves per country.
//   - Equal average revenue per quantity.
// 
void Cartel::update(Solution& solution, RowRoundVectorRef production) {
    Actor::update(solution, production);
    
    simpleQuantities(production);

#if 0
    auto PM = value(production, solution.prices).sum();
    auto PC = competitiveSolution.values.sum();

    RoundMatrix ar(size(), NumRounds+1);
    Vector tgtAr(size());

    // Iteratively compute actual AR and target AR, then re-assign
    // quantity within a round (keeping production constant). This is
    // done by moving in the direction of equal ar by a fixed delta.
    for (int i = 0; i < 100; i++) {
        for (int a = 0; a < size(); a++) {
            auto& actor = *actors[a];
            Vector q = quantities.row(a);
            assert(feq(q.sum(), actor.reserves));

            auto pm = actor.value(q, solution.prices);
            ar.row(a) = pm.array() / q.array();

            auto pc = competitiveSolution.values(a);
            auto beta = pc / PC;

            auto tgtPm = beta * PM;
            tgtAr(a) = tgtPm / actor.reserves;
        }

        for (int a = 0; a < size(); a++) {
            Vector dir = ar.row(a).array() - tgtAr(a);
            dir /= dir.sum();
            quantities.row(a) += 10. * dir;
        }
        
        preserveConstraints(production);
    }
#endif

    // validate constraints
    // refresh();
    // for (int a = 0; a < size(); a++) {
    //     assert(approx(ar.row(a).mean(), tgtAr(a), 100));
    // }
    verifyQuantities(production);

    // change supply curve shape
    for (int r = 0; r <= NumRounds; r++) {
        auto& pw = piecewise[r];
        for (int a = 0; a < size(); a++) {
            pw.pieces[a].length = quantities(a, r);
        }
    }
}

void Cartel::preserveConstraints(const RowRoundVectorRef production) {
    for (int r = 0; r <= NumRounds; r++) {
        for (int a = 0; a < size(); a++) {
            auto& q = quantities(a, r);
            q = std::max(0., q);
            q = std::min(q, actorCapacity(a));
        }
    }

    Vector prodSpill = quantities.colwise().sum() - production;
    Vector resSpill = quantities.rowwise().sum() - actorReserves;

    for (int r = 0; r <= NumRounds; r++) {
        Vector dir = (.5 * prodSpill(r) / resSpill.sum()) * resSpill;
        quantities.col(r) -= dir;
    }

    for (int a = 0; a < size(); a++) {
        Vector dir = (.5 * resSpill(a) / prodSpill.sum()) * prodSpill;
        quantities.row(a) -= dir;
    }

    prodSpill = quantities.colwise().sum() - production;
    resSpill = quantities.rowwise().sum() - actorReserves;

    assert(feq(prodSpill.sum(), 0.));
    assert(feq(resSpill.sum(), 0.));

    // std::cout << quantities << std::endl << std::endl;
    // std::cout << prodSpill.norm() << " " << resSpill.norm() << std::endl << std::endl;
}

// Initial simple allocation. Satisfy production, capacity, and
// reserve constraints.
//
// Put as much of the low-marginal-cost (ie high-value) production in
// early rounds as possible to maximize profit (because of inflation).
void Cartel::simpleQuantities(const RowRoundVectorRef production) {
    quantities.setZero();
    for (int r = 0; r <= NumRounds; r++) {
        for (int a = 0; a < size(); a++) {
            quantities(a, r) = actorCapacity(a);
        }
    }
    
    preserveConstraints(production);
}

void Cartel::verifyQuantities(const RowRoundVectorRef production) const {
    for (int a = 0; a < size(); a++) {
        assert(feq(quantities.row(a).sum(), actorReserves(a)));
        for (int r = 0; r <= NumRounds; r++) {
            auto q = quantities(a, r);
            assert(0. <= q && q <= 1.01 * actorCapacity(a));
        }
    }
    for (int r = 0; r <= NumRounds; r++) {
        assert(feq(quantities.col(r).sum(), production(r)));
    }
}

void Cartel::dump(const Solution& solution) const {
    std::cout << std::endl << std::setw(20) << "Full quantities: " << std::endl
              << fullQuantities() << std::endl;

    Vector values(size());
    for (int a = 0; a < size(); a++) {
        values(a) = actors[a]->value(quantities.row(a), solution.prices).sum();
    }

    std::cout << std::setw(20) << "Values: " << values.transpose() << std::endl;
}
