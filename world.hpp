#pragma once

#include "actor.hpp"

namespace opec {

class World {
    public:
        World();
        ~World();

        const Curve& demand(int round) const {
            if (round % 2 == 0) {
                return *lowResidualDemand;
            } else {
                return *highResidualDemand;
            }
        }

    private:
        InterpolatingCurve* lowResidualDemand;
        InterpolatingCurve* highResidualDemand;
};

}
