#pragma once

#include <cassert>
#include <cmath>
#include "constants.hpp"
#include "actor.hpp"
#include "market.hpp"

namespace opec {

class Country : public Actor {
    public:
        Country(string _name, int _reserves, int _capacity, int _marginalCost)
                : Actor(_name), marginalCost(_marginalCost) {
            reserves = _reserves;
            capacity = _capacity;
        }
        
        Curve& supply() { return marginalCost; }

    private:
        ConstantCurve marginalCost;
};

}
