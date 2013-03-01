#pragma once

#include "optimize.hpp"
#include "world.hpp"
#include "actor.hpp"

namespace opec {

class Market {
    public:
        Market(std::vector<Actor*> _actors) : actors(_actors) {}

        int size() const { return actors.size(); }
        std::vector<Actor*> actors;

        double price(int round, double quantity) const;
        double dPrice(int round, double quantity) const;
        
        static Vector inflate(const Vector& prices);
        
    private:
        World world;
};

}
