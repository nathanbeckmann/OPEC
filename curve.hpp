#pragma once

#include <cassert>
#include <vector>

namespace opec {

class Curve {
    public:
        virtual ~Curve() {}
        virtual double evaluate(double x) const = 0;
        virtual double integrate(double x) const = 0;
        virtual double derivative(double x) const;
};

class ConstantCurve : public Curve {
    public:
        ConstantCurve(double _q) : q(_q) {}
        double evaluate(double x) const { return q; }
        double integrate(double x) const { return q * x; }
    private:
        double q;
};

class PiecewiseCurve : public Curve {
        // ...
};

class InterpolatingCurve : public Curve {
    public:
        InterpolatingCurve(std::vector<double>&& _vy,
                           std::vector<double>&& _vx);
        double evaluate(double x) const;
        double integrate(double x) const;
    private:
        std::vector<double> vy, vx, vdy;
};

}
