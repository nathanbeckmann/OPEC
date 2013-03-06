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

// Compute marginal cost as the weighted average marginal cost of
// constitute countries, since dq for a monopoly (approximately) is
// made of up a weighted sum of dq1...dqN.
RoundVector Cartel::marginalCost(const RoundVector& quantity) const {
    RoundVector cartelMarginalCost;
    Vector countryMarginalCost(size());
    for (int a = 0; a < size(); a++) {
        auto& actor = *actors[a];
        auto mc = actor.supply(0).evaluate(0);
        countryMarginalCost(a) = mc;
    }

    for (int r = 0; r <= NumRounds; r++) {
        if (quantities.col(r).sum() > 0.) {
            cartelMarginalCost(r) = (quantities.col(r).array() * countryMarginalCost.array()).sum() / quantities.col(r).sum();
        } else {
            cartelMarginalCost(r) = supply(r).evaluate(0);
        }
    }

    return cartelMarginalCost;
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
// AP[i] = avg. profit per quantity for country i under monopoly
// res[i] = country i's reserves
//
// Then:
// 
// PM[i] = AP[i] * res[i]       <=>       AP[i] = PM[i] / res[i]
//
// This function iteratively assign reserves to each country such that
// they end up with AP[i] = PM[i] / res[i].
//
// This procedure faces these constraints in order of importance:
//   - Capacity per country.
//   - Reserves per country.
//   - Equal average profit per quantity.
//   - Production per round (as assigned by monopoly solver).
//
// First assign reserves to achieve desired average revenue. Given a
// fixed set of prices (as determined by solver), let MP be the
// marginal profit matrix across rounds and countries. Then let gamma
// be the vector:
//
// gamma[i] = [ AP[i] / MP[i,r] | 0 <= r <= NumRounds ]
//
// Further 1-normalize all gamma[i] such that Sum(gamma[i]) = 1. Then
// quantity can be allocated:
//
// production[i] = gamma[i] * res[i]
//
// Because
//
// AP[i] = (Sum_r MP[i,r] * production[i,r]) / res[i]
//       = MP[i,r] * AP[i] / MP[i,r]
//       = AP[i]
//
// This may not satisfy capacity, reserve, or production constraints,
// however. So we now begin a phase of re-assigning quantity to
// satisfy these constraints.
//
// First capacity constraints are satisfied by iteratively
// constraining dimensions. If any round is over capacity, then that
// dimension is constrained. Finally, the excess is re-distributed
// over remaining rounds, keeping AP[i] constant.
//
// We maintain AP[i] constant by moving along this plane:
//
// Sum_r MP[i,r] * q[i,r] = PM[i]
//
// Or equivalently,
// 
// Sum_r (MP[i,r] - AP[i]) * q[i,r] = 0
//
// So any movement away from AP in a round must be compensated in
// other rounds.
//
// We also must keep total quantity constant:
//
// Sum_r q[i,r] = res[i]
//
// To satisfy both constraints, we must move along the intersection of
// the unconstrained dimensions in each plane. This is done by
// projecting onto each plane sequentially.
//
// So to drain excess, we iteratively project the capacity excess
// times gamma[i] until it is exhausted.
//
// After we have the initial allocation of quantities for all actors,
// we want to target the monopoly quantities as much as possible. This
// is done by inserting the difference in current OPEC production from
// monopoly OPEC production using the same projection process. Because
// this problem may be overconstrained, this proceeds until current
// OPEC production is within some error bound or a fixed number of
// iterations.
// 
// If at any point the projection of a non-zero vector yields the zero
// vector, we are at an overconstrained error condition.
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
            if (r < NumRounds) q = std::min(q, actorCapacity(a));
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
        for (int r = 0; r < NumRounds; r++) {
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
