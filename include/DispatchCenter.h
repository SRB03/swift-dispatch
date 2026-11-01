#ifndef DISPATCHCENTER_H
#define DISPATCHCENTER_H

#include <memory>
#include <vector>
#include "FareCalculator.h"
#include "NearestDriverStrategy.h"
#include "Ride.h"

// Facade for the taxi dispatch backend.
class DispatchCenter {
private:
    std::vector<Driver> drivers;
    std::vector<Ride> rides;
    std::unique_ptr<IDriverSelectionStrategy> driverSelectionStrategy;
    std::unique_ptr<FareCalculator> fareCalculator;
    int nextRideId;

public:
    DispatchCenter(std::unique_ptr<IDriverSelectionStrategy> driverSelectionStrategy,
                   std::unique_ptr<FareCalculator> fareCalculator);

    void registerDriver(const Driver& driver);
    Ride createRide(const Passenger& passenger);
    void completeRide(int rideId);

    const std::vector<Driver>& getDrivers() const;
    const std::vector<Ride>& getRides() const;
};

#endif
