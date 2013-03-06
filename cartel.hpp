#pragma once

#include "actor.hpp"

namespace opec {

class Cartel : public Actor {
    public:
        Cartel(string name, const std::vector<Actor*>& _actors, Solution& _competitiveSolution);

        std::vector<Actor*> actors;

        void update(Solution& solution, RowRoundVectorRef production);
        void dump(const Solution& solution) const;

        RoundVector marginalCost(const RoundVector& quantity) const;

        const Curve& supply(int round) const { return piecewise[round]; }
        const RoundMatrix& fullQuantities() const { return quantities; }

        int size() const { return (int)actors.size(); }

    private:
        void simpleQuantities(const RowRoundVectorRef production);
        void preserveConstraints(const RowRoundVectorRef production);
        void verifyQuantities(const RowRoundVectorRef production) const;
        
        std::vector<PiecewiseCurve> piecewise;
        RoundMatrix quantities;
        Solution& competitiveSolution;
        Vector actorReserves;
        Vector actorCapacity;

};

}
