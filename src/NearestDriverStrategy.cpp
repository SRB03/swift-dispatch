#include "NearestDriverStrategy.h"

#include <limits>
using namespace std;

int NearestDriverStrategy::selectDriverIndex(const vector<Driver>& drivers, const Location& pickupLocation) const {
    double bestDistance = numeric_limits<double>::max();
    int bestIndex = -1;

    for (int i=0; i<drivers.size(); i++) {
        const Driver& driver = drivers[i];
        if (!driver.isAvailable())
            continue;

        const double distance = driver.getCurrentLocation().distanceTo(pickupLocation);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIndex = i;
        }
    }

    return bestIndex;
}
