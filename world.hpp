#pragma once

#include "actor.hpp"

namespace opec {

class World : public Actor {
    public:
        World(int _capacity,
              std::vector<double>&& worldPrices,
              std::vector<double>&& worldQuantity)
                : Actor("world")
                , reserves(std::numeric_limits<int>::max())
                , capacity(_capacity)
                , residual(std::forward(worldPrices), std::forward(worldQuantity)) {
        }

        Curve& supply() { return residual; }

        static World lowDemand;
        static World highDemand;

        static World& current(int round) {
            if (round % 2 == 0) {
                return highDemand;
            } else {
                return lowDemand;
            }
        }

    private:
        InterpolatingCurve residual;
};

}
