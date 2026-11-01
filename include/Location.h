#ifndef LOCATION_H
#define LOCATION_H

#include <string>

// Represents a point in the service area.
class Location {
private:
    std::string areaName;
    double latitude;
    double longitude;

public:
    Location();
    Location(const std::string& areaName, double latitude, double longitude);

    const std::string& getAreaName() const;
    double getLatitude() const;
    double getLongitude() const;

    double distanceTo(const Location& other) const;
};

#endif
