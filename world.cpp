#include "world.hpp"

using namespace opec;

namespace {

double lowPrices[] = {145.43, 145.21, 134.41, 121.29, 114.24, 107.88, 103.73, 94.62, 86.70, 75.07, 73.26, 67.35};
double lowOPECProduction[] = {14521, 15127, 15867, 18401, 19969, 22473, 24368, 25690, 29510, 30766, 32504, 34345};
// double lowROWProduction[] = {46919, 46956, 46902, 46093, 46054, 45296, 45284, 44516, 44020, 43774, 43754, 43157};
// double lowWorldProduction[] = {61440, 62083, 62769, 64494, 66023, 67769, 69652, 70206, 73530, 74540, 76258, 77502};

double highPrices[] = {158.68, 159.49, 157.13, 140.48, 135.59, 125.93, 121.31, 102.97, 100.41, 96.37, 88.96, 86.35};
double highOPECProduction[] = {14951, 16072, 17812, 19411, 21993, 24934, 25992, 28134, 31395, 33821, 34490, 35438};
// double highROWProduction[] = {47828, 47730, 47964, 47365, 46720, 46283, 45829, 45151, 44819, 44783, 44511, 44173};
// double highWorldProduction[] = {62779, 63802, 63776, 66776, 68713, 71217, 69821, 73285, 76214, 78604, 79001, 79611};

const int NumEntries = sizeof(highPrices) / sizeof(highPrices[0]);

std::vector<double> makeVec(double* array) {
    return std::vector<double>(&array[0], &array[NumEntries]);
}

}

World::World() {
    lowResidualDemand = new InterpolatingCurve(makeVec(lowPrices), makeVec(lowOPECProduction));
    highResidualDemand = new InterpolatingCurve(makeVec(highPrices), makeVec(highOPECProduction));
    sellOffDemand = new ConstantCurve(SellOffPrice);
}

World::~World() {
    delete sellOffDemand;
    delete highResidualDemand;
    delete lowResidualDemand;
}
