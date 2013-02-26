#pragma once

#include "actor.hpp"

namespace opec {

class Cartel : public Actor {
    public:
        Cartel(string name) : Actor(name) {}

        void add(Country& country);
        Curve& supply() { return piecewise; }

    private:
        PiecewiseCurve piecewise;
};

}
