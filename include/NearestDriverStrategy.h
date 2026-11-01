#ifndef NEARESTDRIVERSTRATEGY_H
#define NEARESTDRIVERSTRATEGY_H

#include <vector>
#include "Driver.h"
#include "Location.h"

// Strategy interface for choosing an available driver.
class IDriverSelectionStrategy {
public:
    virtual ~IDriverSelectionStrategy() = default;
    virtual int selectDriverIndex(const std::vector<Driver>& drivers, const Location& pickupLocation) const = 0;
};

// Selects the nearest available driver.
class NearestDriverStrategy : public IDriverSelectionStrategy {
public:
    int selectDriverIndex(const std::vector<Driver>& drivers, const Location& pickupLocation) const override;
};

#endif
