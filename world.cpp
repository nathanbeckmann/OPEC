#include "world.hpp"

using namespace opec;

namespace {

double lowPrices[] = {145.43, 145.21, 134.41, 121.29, 114.24, 107.88, 103.73, 94.62, 86.70, 75.07, 73.26, 67.35};
double lowROWProduction[] = {46919, 46956, 46902, 46093, 46054, 45296, 45284, 44516, 44020, 43774, 43754, 43157};
double lowWorldProduction[] = {61440, 62083, 62769, 64494, 66023, 67769, 69652, 70206, 73530, 74540, 76258, 75502};

double highPrices[] = {158.68, 159.49, 157.13, 140.48, 135.59, 125.93, 121.31, 102.97, 100.41, 96.37, 88.96, 86.35};
double highROWProduction[] = {47828, 47730, 47964, 47365, 46720, 46283, 45829, 45151, 44819, 44783, 44511, 44173};
double highWorldProduction[] = {61440, 62083, 62769, 64494, 66023, 67769, 69652, 70206, 73530, 74540, 76258, 75502};

const int NumEntries = sizeof(highPrices) / sizeof(highPrices[0]);

std::vector<double> makeVec(double* array) {
    return std::vector<double>(&array[0], &array[NumEntries]);
}

}

World::World() {
    double lowOPECProduction[NumEntries];
    for (int i = 0; i < NumEntries; i++) {
        lowOPECProduction[i] = lowWorldProduction[i] - lowROWProduction[i];
    }
    lowResidualDemand = new InterpolatingCurve(makeVec(lowPrices), makeVec(lowOPECProduction));

    double highOPECProduction[NumEntries];
    for (int i = 0; i < NumEntries; i++) {
        highOPECProduction[i] = highWorldProduction[i] - highROWProduction[i];
    }
    highResidualDemand = new InterpolatingCurve(makeVec(highPrices), makeVec(highOPECProduction));
}

World::~World() {
    delete lowResidualDemand;
    delete highResidualDemand;
}
