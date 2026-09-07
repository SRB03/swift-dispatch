#include "Passenger.h"
using namespace std;

Passenger::Passenger(int passengerId, const string& name, const Location& pickupLocation, const Location& dropLocation){
    this->passengerId = passengerId;
    this->name = name;
    this->pickupLocation = pickupLocation;
    this->dropLocation = dropLocation;
}

int Passenger::getPassengerId() const {
    return passengerId;
}

const string& Passenger::getName() const {
    return name;
}

const Location& Passenger::getPickupLocation() const {
    return pickupLocation;
}

const Location& Passenger::getDropLocation() const {
    return dropLocation;
}
