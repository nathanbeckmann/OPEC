#pragma once

#include <optimize.hpp>

namespace opec {

template<int NumActors>
class Market {
    public:
        Market(vector<const Actor*>&& _actors);

        double price(int round, Vector quantity) const;

        int size() const { return actors.size(); }
        
        vector<const Actor*> actors;
};

}
