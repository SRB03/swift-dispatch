#include "Location.h"

#include <cmath>
using namespace std;

Location::Location(){
    areaName = "Unknown";
    latitude = 0.0;
    longitude = 0.0;
}

Location::Location(const string& areaName, double latitude, double longitude){        
    this->areaName = areaName;
    this->latitude = latitude;
    this->longitude = longitude;
}

const string& Location::getAreaName() const {
    return areaName;
}

double Location::getLatitude() const {
    return latitude;
}

double Location::getLongitude() const {
    return longitude;
}

double Location::distanceTo(const Location& other) const {
    const double deltaLatitude = latitude - other.latitude;
    const double deltaLongitude = longitude - other.longitude;
    return sqrt(deltaLatitude * deltaLatitude + deltaLongitude * deltaLongitude);
}
