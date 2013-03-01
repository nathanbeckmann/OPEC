#pragma once

#include "actor.hpp"

// #define USE_CUBIC_SPLINE

namespace opec {

class World {
    public:
        World();
        ~World();

        const Curve& demand(int round) const {
            if (round >= NumRounds) {
                return *sellOffDemand;
            } else if (round % 2 == 0) {
                return *lowResidualDemand;
            } else {
                return *highResidualDemand;
            }
        }

    private:
#ifdef USE_CUBIC_SPLINE
        InterpolatingCurve* lowResidualDemand;
        InterpolatingCurve* highResidualDemand;
#else
        LinearCurve* highResidualDemand;
        LinearCurve* lowResidualDemand;
#endif
        ConstantCurve* sellOffDemand;
};

}
