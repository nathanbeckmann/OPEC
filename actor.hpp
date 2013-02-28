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
        virtual Curve& supply() = 0;
        virtual const Curve& supply() const { return const_cast<Actor*>(this)->supply(); }

        double value(const RoundVector& quantity, const RoundVector& prices) const;
        RoundVector costs(const RoundVector& quantity) const;
        virtual void update(Solution& solution, RowRoundVectorRef quantities) const;

        void isConstrained(const RoundVector& quantity) const;
};

}
