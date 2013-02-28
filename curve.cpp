#include <iostream>
#include "curve.hpp"

using namespace opec;

// General derivative code using cenetered difference approximation.
//
// This gives O(Delta^2) error.
double Curve::derivative(double x) const {
    // Centered difference approximation. O(Delta^2) error.
    const double Delta = 0.25;
    return (evaluate(x+Delta) - evaluate(x-Delta)) / (2 * Delta);
}

// We use a cubic spline to interpolate values. This is necessary to
// smooth the derivative, because marginal revenue depends on the
// derivative of the demand curve. If the derivative is
// non-continuous, it prevents the solver from converging as it
// thrashes around the discontinuity.
//
// The cubic spline equations are described on wikipedia:
// http://en.wikipedia.org/wiki/Spline_interpolation
//
// The goal is to have the derivatives and values continuous at every
// raw data point. Values are read directly from the input data;
// derivatives are set according to the average slope of the linear
// interpolation at intermediate points, or the linear interpolation
// directly at the terminals.
InterpolatingCurve::InterpolatingCurve(std::vector<double>&& _vy,
                                       std::vector<double>&& _vx)
        : vy(_vy), vx(_vx), vdy(vy.size(), 0.) {
    assert(vx.size() >= 2);
    assert(vx.size() == vy.size());
    auto sz = vx.size();

    // compute derivatives at each point
    vdy[0]        = (vy[1]    - vy[0])    / (vx[1]    - vx[0]);
    vdy[sz-1]     = (vy[sz-1] - vy[sz-2]) / (vx[sz-1] - vx[sz-2]);
    for (int i = 1; i < (int)vx.size() - 1; i++) {
        vdy[i]    = (vy[i+1]  - vy[i-1])  / (vx[i+1]  - vx[i-1]);
    }
}

double InterpolatingCurve::evaluate(double x) const {
    int i;
    for (i = 0; i < (int)vx.size(); i++) {
        if (vx[i] >= x) break;
    }

    if (i == 0) i = 1;
    if (i == (int)vx.size()) --i;

    auto x0 = vx[i-1];
    auto x1 = vx[i];
    auto y0 = vy[i-1];
    auto y1 = vy[i];
    auto dy0 = vdy[i-1];
    auto dy1 = vdy[i];

    auto t = (x - x0) / (x1 - x0);
    auto a = dy0 * (x1 - x0) - (y1 - y0);
    auto b = -dy1 * (x1 - x0) + (y1 - y0);

    auto y = (1 - t) * y0
            + t * y1
            + t * (1 - t) * (a * (1 - t) + b * t);
    
    return y;
}

// We use a linear interpolation of the integral. This should be good
// enough; and I don't think it matters if the interpolation method is
// less accurate for the integral, since it is only used to compute
// total value.
double InterpolatingCurve::integrate(double x) const {
    auto integral = 0.;
    
    int i;
    for (i = 0; i < (int)vx.size(); i++) {
        if (vx[i] >= x) {
            break;
        } else if (i > 0) {
            auto x0 = vx[i-1];
            auto x1 = vx[i];
            auto y0 = vy[i-1];
            auto y1 = vy[i];
            integral += (x1 - x0) * (y0 + y1) / 2.;
        }
    }

    if (i == 0) i = 1;
    if (i == (int)vx.size()) --i;

    auto x0 = vx[i-1];
    auto x1 = vx[i];
    auto y0 = vy[i-1];
    auto y1 = vy[i];
    
    auto alpha = (x - x0) / (x1 - x0);
    auto y = y0 + alpha * (y1 - y0);

    integral += (x - x0) * (y0 + y) / 2.;
    
    return integral;
}
