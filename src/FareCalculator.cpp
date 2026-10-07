#include "FareCalculator.h"

StandardFareCalculator::StandardFareCalculator(double baseFare, double perKmRate){
    this->baseFare = baseFare;
    this->perKmRate = perKmRate;
}

double StandardFareCalculator::calculateFare(double distanceInKm) const {
    return baseFare + (distanceInKm * perKmRate);
}
