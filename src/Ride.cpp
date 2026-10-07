#include "Ride.h"
using namespace std;

Ride::Ride(int rideId, const Passenger& passenger, const Driver& driver, double distance, double fare): rideId(rideId), passenger(passenger), driver(driver), distance(distance), fare(fare), status("Assigned") {}

int Ride::getRideId() const {
    return rideId;
}

const Passenger& Ride::getPassenger() const {
    return passenger;
}

const Driver& Ride::getDriver() const {
    return driver;
}

double Ride::getDistance() const {
    return distance;
}

double Ride::getFare() const {
    return fare;
}

const string& Ride::getStatus() const {
    return status;
}

void Ride::completeRide() {
    status = "Completed";
}