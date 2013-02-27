#include "curve.hpp"

using namespace opec;

InterpolatingCurve::InterpolatingCurve(std::vector<double>&& _vy,
                                       std::vector<double>&& _vx)
        : vy(_vy), vx(_vx) {
}

double InterpolatingCurve::evaluate(double x) const {
    int i;
    for (i = 0; i < (int)vx.size(); i++) {
        if (vx[i] >= x) break;
    }

    if (i == 0) i = 1;
    if (i == (int)vx.size()) --i;

    double x0 = vx[i-1];
    double x1 = vx[i];
    double y0 = vy[i-1];
    double y1 = vy[i];
    
    double alpha = (x - x0) / (x1 - x0);
    return y0 + alpha * (y1 - y0);
}

double InterpolatingCurve::integrate(double x) const {
    double integral = 0.;
    
    int i;
    for (i = 0; i < (int)vx.size(); i++) {
        if (vx[i] >= x) {
            break;
        } else if (i > 0) {
            double x0 = vx[i-1];
            double x1 = vx[i];
            double y0 = vy[i-1];
            double y1 = vy[i];
            integral += (x1 - x0) * (y0 + y1) / 2.;
        }
    }

    if (i == 0) i = 1;
    if (i == (int)vx.size()) --i;

    double x0 = vx[i-1];
    double x1 = vx[i];
    double y0 = vy[i-1];
    double y1 = vy[i];
    
    double alpha = (x - x0) / (x1 - x0);
    double y = y0 + alpha * (y1 - y0);

    integral += (x - x0) * (y0 + y) / 2.;
    
    return integral;
}
