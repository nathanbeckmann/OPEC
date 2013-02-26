#pragma once

#include <vector>

namespace opec {

class Curve {
    public:
        virtual ~Curve() {}
        virtual double quantity(double price) const = 0;
        double operator() (double price) const { return quantity(price); }
};

class ConstantCurve : public Curve {
    public:
        ConstantCurve(double _q) : q(_q) {}
        double quantity(double price) const { return q; }
    private:
        double q;
};

class PiecewiseCurve : public Curve {
        // ...
};

class InterpolatingCurve : public Curve {
    public:
        InterpolatingCurve(std::vector<double>&& _prices,
                           std::vector<double>&& _quantities);
        double quantity(double price) const;
    private:
        std::vector<double> prices, quantities;
};

}
