#include "Driver.h"
using namespace std;

Driver::Driver(int driverId, const string& name, const string& vehicleNumber, const Location& currentLocation, double rating){
    this->available = true; // Set the driver as available by default
    this->rating = rating;
    this->currentLocation = currentLocation;
    this->vehicleNumber = vehicleNumber;
    this->name = name;
    this->driverId = driverId;
}

int Driver::getDriverId() const {
    return driverId;
}

const string& Driver::getName() const {
    return name;
}

const string& Driver::getVehicleNumber() const {
    return vehicleNumber;
}

const Location& Driver::getCurrentLocation() const {
    return currentLocation;
}

bool Driver::isAvailable() const {
    return available;
}

double Driver::getRating() const {
    return rating;
}

void Driver::setCurrentLocation(const Location& location) {
    currentLocation = location;
}

void Driver::setAvailability(bool isAvailable) {
    available = isAvailable;
}