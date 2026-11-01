// main.cpp — Interactive CLI front-end for the Swift Dispatch ride-dispatching system.
//
// Relies ONLY on the interfaces already used by the original main.cpp:
//   DispatchCenter(unique_ptr<NearestDriverStrategy>, unique_ptr<StandardFareCalculator>)
//   DispatchCenter::registerDriver(Driver), DispatchCenter::createRide(Passenger) -> Ride
//   Driver(int, string, string, Location, double), Passenger(int, string, Location, Location)
//   Location(string, double, double)
//   Ride::getRideId/getDriver/getPassenger/getDistance/getFare/getStatus

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#include "DispatchCenter.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// ============================================================================
//  ANSI styling
// ============================================================================
namespace Style {
    const string Reset  = "\033[0m";
    const string Bold   = "\033[1m";
    const string Dim    = "\033[2m";
    const string Red    = "\033[31m";
    const string Green  = "\033[32m";
    const string Yellow = "\033[33m";
    const string Cyan   = "\033[36m";
}

// Enables UTF-8 output and ANSI escape processing (needed on Windows consoles).
void initTerminal() {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode)) {
        SetConsoleMode(out, mode | 0x0004); // ENABLE_VIRTUAL_TERMINAL_PROCESSING
    }
    #endif
}

string paint(const string& text, const string& style) {
    return style + text + Style::Reset;
}

// ============================================================================
//  Box / card drawing
// ============================================================================

// Width of a string as displayed: skips ANSI sequences, counts UTF-8 code points.
size_t visibleWidth(const string& s) {
    size_t width = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\033') {                       // skip ESC [ ... m
            while (i < s.size() && s[i] != 'm') ++i;
            continue;
        }
        if ((static_cast<unsigned char>(s[i]) & 0xC0) != 0x80) ++width;
    }
    return width;
}

string repeat(const string& unit, size_t count) {
    string result;
    for (size_t i = 0; i < count; ++i) result += unit;
    return result;
}

// Draws a titled card:  ┌─ Title ────┐ / │ line │ / └────────────┘
void printBox(const string& title, const vector<string>& lines, const string& color) {
    size_t titleWidth = visibleWidth(title);
    size_t contentWidth = titleWidth + 1;
    for (const auto& line : lines) contentWidth = max(contentWidth, visibleWidth(line));
    const size_t inner = contentWidth + 2;          // one space padding each side

    cout << color << "┌─ " << Style::Bold << title << Style::Reset << color << " "
    << repeat("─", inner - titleWidth - 3) << "┐" << Style::Reset << '\n';

    for (const auto& line : lines) {
        cout << color << "│" << Style::Reset << ' ' << line
        << string(contentWidth - visibleWidth(line), ' ') << ' '
        << color << "│" << Style::Reset << '\n';
    }

    cout << color << "└" << repeat("─", inner) << "┘" << Style::Reset << '\n';
}

string labelled(const string& label, const string& value, size_t labelWidth = 12) {
    return paint(label + string(labelWidth > label.size() ? labelWidth - label.size() : 0, ' '),
                 Style::Dim) + value;
}

// ============================================================================
//  UI components
// ============================================================================
void printMenuHeader() {
    cout << '\n';
    printBox("SWIFT DISPATCH", {
        paint("1.", Style::Cyan + Style::Bold) + " Register Driver",
             paint("2.", Style::Cyan + Style::Bold) + " Book a Ride / Register Passenger",
             paint("3.", Style::Cyan + Style::Bold) + " View Active Drivers",
             paint("4.", Style::Cyan + Style::Bold) + " Exit"
    }, Style::Cyan);
}

void printErrorBanner(const string& message) {
    printBox("ERROR", { paint(message, Style::Red) }, Style::Red);
}

void printWarningBanner(const string& message) {
    printBox("NOTICE", { paint(message, Style::Yellow) }, Style::Yellow);
}

void printSuccessBanner(const string& message) {
    printBox("SUCCESS", { paint(message, Style::Green) }, Style::Green);
}

string formatDecimal(double value, int precision) {
    ostringstream oss;
    oss << fixed << setprecision(precision) << value;
    return oss.str();
}

void printRideReceipt(const Ride& ride) {
    ostringstream rideId, status;
    rideId << ride.getRideId();
    status << ride.getStatus();

    printBox("RIDE RECEIPT", {
        labelled("Ride ID",   paint(rideId.str(), Style::Bold)),
        labelled("Driver",    ride.getDriver().getName()),
        labelled("Passenger", ride.getPassenger().getName()),
        labelled("Distance",  formatDecimal(ride.getDistance(), 2) + " km"),
             labelled("Fare",      paint("₹" + formatDecimal(ride.getFare(), 2), Style::Green + Style::Bold)),
             labelled("Status",    paint(status.str(), Style::Yellow))
    }, Style::Green);
}

// ============================================================================
//  Robust input helpers
//  (mixes `cin >>` for numbers and `getline` for text; buffer is cleared
//   after every numeric read so the two never interfere)
// ============================================================================
bool inputClosed() { return cin.eof(); }

string trim(const string& s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

void clearInputLine() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string prompt(const string& label) {
    return paint("  ▸ ", Style::Cyan) + label + paint(": ", Style::Dim);
}

// Reads a non-empty line of text. Returns false only if input stream ended.
bool readText(const string& label, string& out) {
    while (true) {
        cout << prompt(label) << flush;
        if (!getline(cin, out)) return false;
        out = trim(out);
        if (!out.empty()) return true;
        printErrorBanner("Input cannot be empty. Please try again.");
    }
}

// Reads a number with cin >>, rejecting non-numeric / trailing-garbage input.
template <typename T>
bool readNumber(const string& label, T& out, T minValue, T maxValue) {
    while (true) {
        cout << prompt(label) << flush;
        T value{};
        if (cin >> value) {
            if (cin.peek() != '\n' && cin.peek() != EOF) {  // e.g. "12abc"
                clearInputLine();
                printErrorBanner("Please enter a valid number.");
                continue;
            }
            clearInputLine();
            if (value < minValue || value > maxValue) {
                ostringstream msg;
                msg << "Value must be between " << minValue << " and " << maxValue << ".";
                printErrorBanner(msg.str());
                continue;
            }
            out = value;
            return true;
        }
        if (inputClosed()) return false;
        clearInputLine();                                    // non-numeric input
        printErrorBanner("Invalid input — a number is required.");
    }
}

bool readLocation(const string& heading, Location& out) {
    cout << paint("\n  " + heading, Style::Bold) << '\n';
    string name;
    double lat = 0.0, lon = 0.0;
    if (!readText("Place name", name)) return false;
    if (!readNumber<double>("Latitude  (-90 to 90)", lat, -90.0, 90.0)) return false;
    if (!readNumber<double>("Longitude (-180 to 180)", lon, -180.0, 180.0)) return false;
    out = Location(name, lat, lon);
    return true;
}

// ============================================================================
//  Application state
//  A lightweight display record is kept here so "View Active Drivers" needs
//  nothing beyond the constructors/getters already known to exist.
// ============================================================================
struct DriverRecord {
    int id;
    string name;
    string plate;
    string locationName;
    double rating;
};

struct AppState {
    DispatchCenter center;
    vector<DriverRecord> drivers;
    set<int> usedDriverIds;
    int nextPassengerId = 101;

    AppState()
    : center(make_unique<NearestDriverStrategy>(),
             make_unique<StandardFareCalculator>()) {}
};

// ============================================================================
//  Menu actions
// ============================================================================
void actionRegisterDriver(AppState& app) {
    printBox("REGISTER DRIVER", { paint("Enter the driver's details below.", Style::Dim) }, Style::Cyan);

    int id = 0;
    string name, plate;
    double rating = 0.0;
    Location location("", 0.0, 0.0);

    while (true) {
        if (!readNumber<int>("Driver ID", id, 1, numeric_limits<int>::max())) return;
        if (app.usedDriverIds.count(id) == 0) break;
        printErrorBanner("Driver ID " + to_string(id) + " is already registered.");
    }
    if (!readText("Driver name", name)) return;
    if (!readText("Vehicle plate (e.g. OD01AB1234)", plate)) return;
    if (!readNumber<double>("Rating (0.0 - 5.0)", rating, 0.0, 5.0)) return;
    if (!readLocation("Driver's current location", location)) return;

    try {
        app.center.registerDriver(Driver(id, name, plate, location, rating));
        app.usedDriverIds.insert(id);
        // Re-read the place name for display from what the user typed.
        app.drivers.push_back({id, name, plate, "", rating});
        printSuccessBanner("Driver \"" + name + "\" registered successfully.");
    } catch (const exception& e) {
        printErrorBanner(string("Registration failed: ") + e.what());
    }
}

void actionBookRide(AppState& app) {
    if (app.drivers.empty()) {
        printWarningBanner("No drivers are registered yet. Register a driver first (option 1).");
        return;
    }

    printBox("BOOK A RIDE", { paint("Register as a passenger and request a ride.", Style::Dim) }, Style::Cyan);

    string name;
    Location pickup("", 0.0, 0.0), destination("", 0.0, 0.0);

    if (!readText("Passenger name", name)) return;
    if (!readLocation("Pickup location", pickup)) return;
    if (!readLocation("Destination", destination)) return;

    Passenger passenger(app.nextPassengerId, name, pickup, destination);

    try {
        Ride ride = app.center.createRide(passenger);
        ++app.nextPassengerId;                      // consume ID only on success
        cout << '\n';
        printSuccessBanner("Ride created successfully!");
        printRideReceipt(ride);
    } catch (const exception& e) {
        cout << '\n';
        printErrorBanner(string("Dispatch failed: ") + e.what());
    }
}

void actionViewDrivers(const AppState& app) {
    if (app.drivers.empty()) {
        printWarningBanner("No drivers registered yet.");
        return;
    }

    vector<string> lines;
    {
        ostringstream header;
        header << left << setw(5) << "ID" << setw(16) << "Name"
        << setw(14) << "Plate" << "Rating";
        lines.push_back(paint(header.str(), Style::Bold));
    }
    for (const auto& d : app.drivers) {
        ostringstream row;
        row << left << setw(5) << d.id << setw(16) << d.name
        << setw(14) << d.plate << formatDecimal(d.rating, 1) << " ★";
        lines.push_back(row.str());
    }
    lines.push_back("");
    lines.push_back(paint("Total drivers: " + to_string(app.drivers.size()), Style::Dim));

    printBox("ACTIVE DRIVERS", lines, Style::Cyan);
}

// ============================================================================
//  Main loop
// ============================================================================
int main() {
    initTerminal();
    AppState app;

    while (true) {
        printMenuHeader();

        int choice = 0;
        if (!readNumber<int>("Select an option (1-4)", choice, 1, 4)) break; // EOF → exit
        cout << '\n';

        switch (choice) {
            case 1: actionRegisterDriver(app); break;
            case 2: actionBookRide(app);       break;
            case 3: actionViewDrivers(app);    break;
            case 4:
                printBox("GOODBYE", { paint("Thanks for using Swift Dispatch. Safe travels!", Style::Green) },
                         Style::Green);
                return 0;
        }
    }

    cout << '\n' << paint("Input closed. Exiting.", Style::Dim) << '\n';
    return 0;
}
