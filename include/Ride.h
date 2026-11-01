#ifndef RIDE_H
#define RIDE_H

#include <string>
#include "Driver.h"
#include "Passenger.h"

// Represents a confirmed trip between a passenger and a driver.
class Ride {
private:
    int rideId;
    Passenger passenger;
    Driver driver;
    double distance;
    double fare;
    std::string status;

public:
    Ride(int rideId, const Passenger& passenger, const Driver& driver, double distance, double fare);

    int getRideId() const;
    const Passenger& getPassenger() const;
    const Driver& getDriver() const;
    double getDistance() const;
    double getFare() const;
    const std::string& getStatus() const;

    void completeRide();
};

#endif
