#include "DispatchCenter.h"

#include <algorithm>
#include <stdexcept>

DispatchCenter::DispatchCenter(std::unique_ptr<IDriverSelectionStrategy> driverSelectionStrategy,
                               std::unique_ptr<FareCalculator> fareCalculator)
    : driverSelectionStrategy(std::move(driverSelectionStrategy)),
      fareCalculator(std::move(fareCalculator)),
      nextRideId(1) {}

void DispatchCenter::registerDriver(const Driver& driver) {
    drivers.push_back(driver);
}

Ride DispatchCenter::createRide(const Passenger& passenger) {
    const int driverIndex = driverSelectionStrategy->selectDriverIndex(drivers, passenger.getPickupLocation());
    if (driverIndex < 0) {
        throw std::runtime_error("No available driver found for the request");
    }

    Driver& assignedDriver = drivers[driverIndex];
    const double distance = passenger.getPickupLocation().distanceTo(passenger.getDropLocation());
    const double fare = fareCalculator->calculateFare(distance);

    assignedDriver.setAvailability(false);
    assignedDriver.setCurrentLocation(passenger.getDropLocation());

    Ride ride(nextRideId++, passenger, assignedDriver, distance, fare);
    rides.push_back(ride);
    return ride;
}

void DispatchCenter::completeRide(int rideId) {
    auto rideIt = std::find_if(rides.begin(), rides.end(), [rideId](const Ride& ride) {
        return ride.getRideId() == rideId;
    });

    if (rideIt == rides.end()) {
        throw std::runtime_error("Ride not found");
    }

    rideIt->completeRide();

    const int driverId = rideIt->getDriver().getDriverId();
    auto driverIt = std::find_if(drivers.begin(), drivers.end(), [driverId](const Driver& driver) {
        return driver.getDriverId() == driverId;
    });

    if (driverIt != drivers.end()) {
        driverIt->setAvailability(true);
    }
}

const std::vector<Driver>& DispatchCenter::getDrivers() const {
    return drivers;
}

const std::vector<Ride>& DispatchCenter::getRides() const {
    return rides;
}
