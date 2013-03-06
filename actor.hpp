#pragma once

#include <string>
#include "curve.hpp"
#include "optimize.hpp"

namespace opec {

using std::string;

class Actor {
    public:
        Actor(string _name) : name(_name), reserves(0), capacity(0) {}
        virtual ~Actor() {}

        const string name;
        int reserves;
        int capacity;

        virtual void update(Solution& solution, RowRoundVectorRef quantities);

        RoundVector value(const RoundVector& quantity, const RoundVector& prices) const;
        virtual RoundVector marginalCost(const RoundVector& quantity) const;
        RoundVector marginalRevenue(const RoundVector& prices,
                                    const RoundVector& quantity,
                                    const Market& market,
                                    const RoundVector& production) const;
        
        void isConstrained(const RoundVector& quantity) const;

        virtual const Curve& supply(int round) const = 0;
};

}
