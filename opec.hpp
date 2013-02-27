#pragma once

#include <algorithm>
#include "country.hpp"

namespace opec {

class Opec {
    public:
        enum Countries {
            SaudiArabia,
            Iran,
            Iraq,
            Kuwait,
            UAE,
            Venezuela,
            Nigeria,
            NumCountries
        };

        Country& operator[] (Countries country) { return countries[country]; }
        Country& operator[] (int country) { return (*this)[Countries(country)]; }

        Opec()
                : countries({Country("Saudi Arabia", 108000, 12000, 9),
                             Country("Iran", 41400, 4600, 10),
                             Country("Iraq", 33300, 3700, 16),
                             Country("Kuwait", 29700, 3300, 13),
                             Country("UAE", 27000, 3000, 5),
                             Country("Venezula", 39600, 4400, 20),
                             Country("Nigeria", 24300, 2700, 7)}) {
            auto opecReserves = std::accumulate(&countries[0], &countries[NumCountries],
                                                0, [] (int sum, const Country& c) { return sum + c.reserves; });
            assert(opecReserves == 303300);
            auto opecCapacity = std::accumulate(&countries[0], &countries[NumCountries],
                                                0, [] (int sum, const Country& c) { return sum + c.capacity; });
            assert(opecCapacity == 33700);
        }

    private:
        Country countries[NumCountries];
};

extern Opec opec;

}
