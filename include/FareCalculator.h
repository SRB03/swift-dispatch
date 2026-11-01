#ifndef FARECALCULATOR_H
#define FARECALCULATOR_H

// Provides fare calculation logic for rides.
class FareCalculator {
public:
    virtual ~FareCalculator() = default;
    virtual double calculateFare(double distanceInKm) const = 0;
};

// demo not in real but just for running
class StandardFareCalculator : public FareCalculator {
private:
    double baseFare;
    double perKmRate;

public:
    StandardFareCalculator(double baseFare = 40.0, double perKmRate = 18.0);
    double calculateFare(double distanceInKm) const override;
};

#endif
