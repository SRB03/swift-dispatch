#ifndef DRIVER_H
#define DRIVER_H

#include <string>
#include "Location.h"

// Stores driver state used by the dispatch engine.
class Driver {
private:
    int driverId;
    std::string name;
    std::string vehicleNumber;
    Location currentLocation;
    bool available;
    double rating;

public:
    Driver(int driverId, const std::string& name, const std::string& vehicleNumber,
           const Location& currentLocation, double rating);

    int getDriverId() const;
    const std::string& getName() const;
    const std::string& getVehicleNumber() const;
    const Location& getCurrentLocation() const;
    bool isAvailable() const;
    double getRating() const;

    void setCurrentLocation(const Location& location);
    void setAvailability(bool isAvailable);
};

#endif
