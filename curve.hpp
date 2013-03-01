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
        ConstantCurve(double _y) : y(_y) {}
        double evaluate(double x) const { return y; }
        double integrate(double x) const { return y * x; }
    private:
        double y;
};

class PiecewiseCurve : public Curve {
    public:
        void add(double y, double length);
        double evaluate(double x) const;
        double integrate(double x) const;

        struct Piece {
                double y;
                double length;
        };
        std::vector<Piece> pieces;
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
