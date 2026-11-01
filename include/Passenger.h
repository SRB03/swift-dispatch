#ifndef PASSENGER_H
#define PASSENGER_H

#include <string>
#include "Location.h"

// Stores passenger profile and ride request details.
class Passenger {
private:
    int passengerId;
    std::string name;
    Location pickupLocation;
    Location dropLocation;

public:
    Passenger(int passengerId, const std::string& name,
              const Location& pickupLocation, const Location& dropLocation);

    int getPassengerId() const;
    const std::string& getName() const;
    const Location& getPickupLocation() const;
    const Location& getDropLocation() const;
};

#endif
