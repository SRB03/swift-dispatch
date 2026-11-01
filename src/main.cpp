#include <iostream>
#include <memory>
using namespace std;

#include "DispatchCenter.h"

int main() {
    DispatchCenter dispatchCenter(
        make_unique<NearestDriverStrategy>(),
        make_unique<StandardFareCalculator>()
    );

    dispatchCenter.registerDriver(Driver(1, "Arjun", "OD01AB1234", Location("Laxmisagar", 12.9716, 77.5946), 4.8));
    dispatchCenter.registerDriver(Driver(2, "Ravi", "OD02CD5678", Location("Bapuji Nagar", 12.9710, 77.6400), 4.6));
    dispatchCenter.registerDriver(Driver(3, "Meera", "OD03EF9012", Location("Old town", 12.9000, 77.5500), 4.9));

    Passenger passenger(101, "Aman", Location("Pickup Point", 12.9650, 77.6000), Location("Destination", 12.9300, 77.6200));

    try {
        Ride ride = dispatchCenter.createRide(passenger);
        cout << "Ride created successfully\n";
        cout << "Ride ID: " << ride.getRideId() << '\n';
        cout << "Driver: " << ride.getDriver().getName() << '\n';
        cout << "Passenger: " << ride.getPassenger().getName() << '\n';
        cout << "Distance: " << ride.getDistance() << " km\n";
        cout << "Fare: " << ride.getFare() << '\n';
        cout << "Status: " << ride.getStatus() << '\n';
    } catch (const exception& exception) {
        cout << "Dispatch failed: " << exception.what() << '\n';
    }

    return 0;
}
