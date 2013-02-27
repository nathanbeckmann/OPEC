#pragma once

#include "optimize.hpp"
#include "world.hpp"
#include "actor.hpp"

namespace opec {

class Market {
    public:
        Market(std::vector<const Actor*>&& _actors) : actors(_actors) {}

        double price(int round, Vector quantity) const;

        int size() const { return actors.size(); }
        
        std::vector<const Actor*> actors;

    private:
        World world;
};

}
