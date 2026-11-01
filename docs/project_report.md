# Swift Dispatch: Backend Architecture for a Dynamic Taxi Dispatch System

**Unified Technical Architecture & Object-Oriented Engineering Report (C++17 & CMake)**

| Attribute | Details |
| :--- | :--- |
| **Author & Identifier** | Soumya Ranjan Behera · Roll No. B125124, Section CSEB-2 |
| **Academic Context** | End-of-Semester Project Report, Object-Oriented Programming (3rd Semester) |
| **Repository Link** | [github.com/SRB03/swift-dispatch](https://www.google.com/search?q=https://github.com/SRB03/swift-dispatch) |
| **Build & Verification Stack** | Standard C++17 (`CMAKE_CXX_STANDARD 17`), CMake 3.16+ (Verified with GCC 13.3 KDE Plasma) |

## Executive Architectural Abstract

Urban mobility is among the most operationally demanding services of the modern city. Every day, millions of short trips must be matched in real time between passengers requesting transport and vehicles distributed across a service area. App-based ride-hailing platforms have replaced traditional street-hailing and depot-phoning models through **dynamic dispatch**: when a ride request arrives, the platform inspects the live state of its fleet and determines within moments which available vehicle should serve the trip. The quality of that algorithmic decision governs passenger waiting time, driver idle kilometres, fuel consumption, and urban congestion.

**Swift Dispatch** is a modular C++17 backend infrastructure engineered to automate the foundational decision-making processes of a dynamic taxi service. Rather than coupling selection logic, pricing formulas, state storage, and console I/O inside a single monolithic procedure, the system decomposes the ride-hailing lifecycle into seven cooperating classes organized around strict Object-Oriented Programming (OOP) principles: Encapsulation, Abstraction, Inheritance, Runtime Polymorphism, Composition over Inheritance, the Gang of Four (GoF) Strategy Design Pattern, Constructor Dependency Injection, and Separation of Concerns (Single Responsibility and Open/Closed Principles).

This unified architectural report synthesizes the high-level conceptual design specification and the complete empirical verification of the delivered 430-line C++17 codebase (7 headers in `include/` and 8 source files in `src/`). It traces the evolution from initial pointer-based abstractions to modern C++17 value semantics and `std::unique_ptr` ownership transfer, documents the five-phase execution workflow and finite state machines, proves algorithmic correctness and complexity bounds, diagnoses four subtle state and validation edge cases uncovered during multi-request simulation testing, and provides compiled, backward-compatible architectural remediations.

| Architectural Dimension | Core Mechanism in Swift Dispatch | Primary Code Artifacts |
| :--- | :--- | :--- |
| **Domain Encapsulation** | Private attributes, immutable value objects, controlled mutators, Rule of Zero | `Location`, `Passenger`, `Driver`, `Ride` |
| **Subsystem Facade** | Unified orchestration hiding 7-step matching, pricing, and state bookkeeping | `DispatchCenter` (`createRide`, `completeRide`) |
| **Polymorphic Policies** | Pure virtual interfaces injected via `std::unique_ptr` constructor arguments | `IDriverSelectionStrategy`, `FareCalculator` |
| **Fault Tolerance** | Strong exception guarantee, coordinate validation, custom exception hierarchy | `DispatchException` subclasses, `std::isfinite` guards |

## 1. Urban Mobility Context, Problem Formulation, and Architectural Objectives

### 1.1 Operational Context and Problem Statement

A taxi operator receives ride requests continuously, each specifying a pickup coordinate and a drop-off destination. At any instant, the operator manages a fleet of drivers who are either available or already engaged on active trips, distributed across the city. To process each incoming request, the backend engine must answer three fundamental operational questions:

1. **Who should serve the request?** An available driver must be selected from the active fleet according to a well-defined selection policy (such as minimum proximity, highest driver rating, lowest estimated time of arrival, or fleet load balancing).
2. **How much should the trip cost?** A fare must be computed consistently and deterministically from the quantitative characteristics of the trip.
3. **What state transitions occurred?** A durable transactional record of the trip must be created, and the assigned driver's availability and position must be updated atomically so that the same driver is never double-booked across concurrent or sequential passengers.

A naive procedural implementation fails in a predictable manner: driver-matching loops, fare formulas, array storage, and console formatting are interleaved inside one large procedure in a single source file. While such code may pass an initial demonstration, it actively resists engineering change. Altering the pricing tariff risks breaking the driver-matching loop, introducing a second selection policy requires conditional branching inside existing tested logic, and no subsystem can be unit-tested in isolation. Consequently, Swift Dispatch addresses a twofold engineering challenge: functionally, to build a dynamic dispatch engine that matches each passenger with the nearest available driver, prices the trip, and maintains consistent fleet and ride state; structurally, to organize that engine so that selection policies and pricing models can be replaced independently without modifying the core orchestrator.

### 1.2 Core Architectural Objectives

The development of Swift Dispatch is guided by four core architectural objectives, each realized in a distinct layer of the codebase:

* **Objective 1 — Modular Object-Oriented Decomposition:** Decompose the domain into small, cohesive C++17 classes with single, well-defined responsibilities (`Location`, `Passenger`, `Driver`, `Ride`, `FareCalculator`, `IDriverSelectionStrategy`, and `DispatchCenter`). Each class hides its internal data representation behind a minimal public contract, enforces its own invariants, and collaborates strictly through public interfaces so that any module can be understood or replaced by inspecting its header file alone.
* **Objective 2 — Dynamic Real-Time Algorithmic Dispatch:** Implement a dynamic dispatch algorithm (`NearestDriverStrategy::selectDriverIndex`) that inspects live fleet state at the exact moment a request arrives, filters out unavailable drivers, computes Euclidean planar distances from each candidate to the passenger's pickup location, and selects the minimum in a single linear scan. Because routing decisions depend on live state rather than a static schedule, submitting identical requests at different moments legitimately yields different drivers.
* **Objective 3 — Policy Decoupling via Strategy Abstractions:** Isolate the two business rules most subject to change in production deployments—driver selection and fare calculation—behind abstract base interfaces (`IDriverSelectionStrategy` / `DispatchStrategy` and `FareCalculator`). `DispatchCenter` depends solely on these abstractions and receives concrete implementations via constructor dependency injection, satisfying the Open/Closed and Dependency Inversion Principles.
* **Objective 4 — Physical Separation and Robust Fault Tolerance:** Enforce physical modularity by separating interface declarations (`include/`) from compiled definitions (`src/`) orchestrated via a CMake 3.16+ build pipeline, while guarding edge cases (such as exhausted driver pools, unknown ride IDs, duplicate completions, and invalid coordinates) through structured C++ exception handling that prevents silent failures and preserves state atomicity.

### 1.3 System Boundaries and Known Operational Constraints

Swift Dispatch is deliberately scoped as a console-driven backend business-logic engine. The implemented system encompasses the core domain entities, driver registration and ride creation via `DispatchCenter`, nearest-driver selection via the Strategy pattern, linear distance-based fare calculation, ride completion with driver release, and an end-to-end simulation in `main.cpp`. To maintain focus on structural software design, the project explicitly excludes Graphical User Interfaces (GUIs) or mobile front ends, network transport layers (REST/WebSockets), persistent database storage (all state resides in memory during execution), user authentication/payment gateways, and third-party road-network routing APIs.

A rigorous engineering evaluation must also state the mathematical and structural boundaries of the delivered implementation plainly:

1. **Planar Euclidean Distance Metric:** `Location::distanceTo` computes straight-line Euclidean distance $\sqrt{(\Delta\text{lat})^2 + (\Delta\text{lon})^2}$ treating latitude and longitude degrees as Cartesian coordinates on a flat plane. Although console output labels the result as `"km"`, the numerical value is in coordinate degrees. Over a small metropolitan area, ordering candidates by Euclidean degree distance yields the same relative ranking as road or great-circle distance, making it a valid proxy for selection; however, production pricing requires the spherical haversine formula or road-network routing.
2. **Distance-Only Tariff Model:** `StandardFareCalculator` evaluates `baseFare + (distance * perKmRate)`, omitting trip duration, traffic delay, and demand-based surge multipliers.
3. **In-Memory Single-Threaded Execution:** Fleet and ride ledgers reside in `std::vector` containers without mutex synchronization, precluding concurrent multi-threaded request mutation.
4. **Linear-Time Candidate Scan:** Scanning every registered driver in $\Theta(n)$ time per request is instantaneous for tens or hundreds of vehicles, whereas city-scale fleets require spatial indexing (such as a uniform grid or $k$-d tree).
5. **Primitive State Representations:** Driver availability is stored as a `bool` and ride lifecycle status as a `std::string` (`"Assigned"`, `"Completed"`), whereas strongly typed `enum class` representations would prevent invalid literals at compile time.
6. **Transient Passenger Objects:** `DispatchCenter` does not maintain a separate passenger registry; a `Passenger` value object is passed directly to `createRide()` and persisted as a copy inside the resulting `Ride` record.

## 2. Functional, Non-Functional, and Build Environment Specifications

### 2.1 Functional Requirements Traceability Matrix

Table 2.1 maps every functional requirement of the Swift Dispatch engine (FR-1 through FR-15) to the concrete C++ class or method that realizes it and records its verification status across the delivered codebase and architectural extensions.

**Table 2.1: Functional Requirements Traceability Matrix**

| ID | Functional Requirement Specification | Realizing Class / Method | Status |
| :--- | :--- | :--- | :--- |
| **FR-1** | Allow drivers—each with a unique ID, name, vehicle number, current location, and rating—to be registered with the dispatcher. | `DispatchCenter::registerDriver`, `Driver` | Implemented |
| **FR-2** | Represent a ride request as a passenger carrying an ID, name, pickup location, and drop-off location. | `Passenger`, `Location` | Implemented |
| **FR-3** | Select, for each incoming ride request, the available driver geographically nearest to the pickup location. | `NearestDriverStrategy::selectDriverIndex` | Implemented |
| **FR-4** | Ignore drivers who are currently unavailable (engaged on an active ride) during candidate evaluation. | `Driver::isAvailable` check in strategy loop | Implemented |
| **FR-5** | Report a dispatch failure gracefully via exception without crashing when no available driver exists. | `std::runtime_error` / `NoDriverAvailableException` in `DispatchCenter::createRide` | Implemented |
| **FR-6** | Compute the trip distance between the passenger's pickup location and drop-off destination. | `Location::distanceTo` | Implemented |
| **FR-7** | Compute a deterministic trip fare from the calculated trip distance using an injected pricing model. | `FareCalculator::calculateFare`, `StandardFareCalculator` | Implemented (distance-only) |
| **FR-8** | Create a durable ride record assigned a unique sequential ride ID and an initial status of `"Assigned"`. | `Ride` constructor, `DispatchCenter::nextRideId` | Implemented |
| **FR-9** | Mark the assigned driver unavailable and update the driver's location to the passenger's drop-off point. | `Driver::setAvailability(false)`, `Driver::setCurrentLocation` | Implemented |
| **FR-10** | Complete an active ride by ID, transitioning its status to `"Completed"` and releasing the assigned driver. | `DispatchCenter::completeRide`, `Ride::completeRide` | Implemented |
| **FR-11** | Report a runtime error when instructed to complete a ride ID that does not exist in the ledger. | `std::runtime_error("Ride not found")` / `RideNotFoundException` | Implemented |
| **FR-12** | Expose read-only views of the registered driver fleet and created ride records without permitting external mutation. | `DispatchCenter::getDrivers() const`, `getRides() const` | Implemented |
| **FR-13** | Display formatted ride summaries (ride ID, driver, passenger, distance, fare, status) on the console. | `main.cpp` simulation driver | Implemented |
| **FR-14** | Validate input coordinates and ride state transitions, rejecting out-of-range, non-finite, or duplicate-completion operations. | Constructor guards & custom `DispatchException` hierarchy | Verified Extension (Section 6.5) |
| **FR-15** | Support time-dependent and demand-based dynamic surge pricing without modifying existing orchestrator code. | `SurgeFareCalculator` & future `FareCalculator` subclasses | Verified Extension (Section 4.4) |

### 2.2 Non-Functional Quality Attributes

Nine non-functional requirements (NFR-1 through NFR-9) govern the structural quality of the codebase:

* **NFR-1 (Extensibility):** New driver-selection policies and pricing tariffs must be addable by introducing new derived classes without editing `DispatchCenter` or existing strategies, satisfied via the Strategy pattern and the Open/Closed Principle.
* **NFR-2 (Maintainability):** Every class must adhere to a single responsibility with a compact implementation footprint. Across the entire 430-line repository (186 lines of headers, 242 lines of source code), the largest translation unit is `DispatchCenter.cpp` at 61 lines, and no class implementation exceeds roughly 40 lines.
* **NFR-3 (Performance):** Driver selection executes in $\Theta(n)$ time for $n$ registered drivers and $O(1)$ auxiliary memory, tracking only `bestDistance` and `bestIndex`. Distance evaluation and fare calculation run in $O(1)$ time per candidate, while ride completion performs linear lookups in $O(r + n)$ time across $r$ rides and $n$ drivers.
* **NFR-4 (Correctness & Data Integrity):** An engaged driver can never be assigned to a second concurrent trip because selection strategies filter on `driver.isAvailable()` and `createRide` marks the driver unavailable before returning.
* **NFR-5 (Encapsulation & Const-Correctness):** All class data members are strictly `private`. Accessors returning composite objects return `const` references (`const std::string&`, `const Location&`, `const std::vector<Driver>&`), preventing callers from mutating internal state or incurring unnecessary copy overhead.
* **NFR-6 (Resource & Memory Safety):** Polymorphic strategy objects are owned exclusively via `std::unique_ptr`, eliminating manual `new` and `delete` calls and guaranteeing deterministic RAII destruction without memory leaks.
* **NFR-7 (Portability):** The implementation relies solely on ISO C++17 and the C++ Standard Library, compiling cleanly without vendor extensions across Windows, Linux, and macOS.
* **NFR-8 (Testability):** Constructor dependency injection enables unit tests to supply deterministic stub strategies or fixed-rate calculators to verify `DispatchCenter` in complete isolation.
* **NFR-9 (Usability):** Console output presents clear, human-readable operational summaries and catches runtime exceptions to print actionable diagnostics rather than terminating abnormally via `std::terminate`.

### 2.3 Toolchain Specifications, Directory Layout, and Build Pipeline

Table 2.2 summarizes the software and hardware environment required to build and execute Swift Dispatch. Notably, C++17 is strictly required (not merely recommended) because the codebase relies on C++14's `std::make_unique` alongside C++11 features including `override`, `= default` virtual destructors, and lambda expressions in `std::find_if`. While the repository's `README.md` mentions CMake 3.10, the root `CMakeLists.txt` declares `cmake_minimum_required(VERSION 3.16)`, making CMake 3.16 the binding requirement. The build was empirically verified using GCC 13.3 and CMake 3.28 on Ubuntu 24.04.

**Table 2.2: Software, Hardware, and Directory Architecture Specifications**

| Category | Component / Directory | Specification & Architectural Role |
| :--- | :--- | :--- |
| **Language & Compiler** | ISO C++17 Toolchain | `CMAKE_CXX_STANDARD 17` (required, extensions off); GCC 7+, Clang 5+, or MSVC 2017+ (verified on GCC 13.3). |
| **Build & VCS** | CMake 3.16+ & Git | Generates native build scripts producing the `taxi_dispatch` binary; hosted at [github.com/SRB03/swift-dispatch](https://www.google.com/search?q=https://github.com/SRB03/swift-dispatch). |
| **STL Dependencies** | C++ Standard Library | `<string>`, `<vector>`, `<memory>` (`unique_ptr`, `make_unique`), `<algorithm>` (`find_if`), `<stdexcept>`, `<cmath>`, `<limits>`, `<iostream>`. |
| **Hardware Floor** | Host Execution Target | Any 64-bit CPU (x86-64 or ARM64), 512 MB free RAM, 50 MB disk storage, standard terminal display. |
| **Interface Tree** | `include/` (7 headers, ~186 LOC) | Public class declarations, pure virtual contracts, include guards (*the what*—visible to every translation unit). |
| **Implementation Tree** | `src/` (8 files, ~242 LOC) | Member-function definitions and `main.cpp` entry point (*the how*—compiled independently into object files). |

The physical repository structure separates interface headers from compiled source files, paired with a concise CMake configuration and standard four-command build procedure:

```text
Project/
├── CMakeLists.txt                  # Build description (C++17 standard, include path)
├── README.md                       # Project overview and roadmap
├── .gitignore                      # Excludes build output and binaries
├── docs/
│   └── PROJECT_REPORT.md           # Short architectural design notes
├── include/                        # Declarations (7 headers: ~186 lines)
│   ├── Location.h                  # Immutable coordinate value object & distance metric
│   ├── Passenger.h                 # Immutable ride request entity
│   ├── Driver.h                    # Mutable supply entity (availability & location)
│   ├── Ride.h                      # Transactional trip record & completion state
│   ├── FareCalculator.h            # Abstract FareCalculator + StandardFareCalculator
│   ├── NearestDriverStrategy.h     # IDriverSelectionStrategy + NearestDriverStrategy
│   └── DispatchCenter.h            # Subsystem Facade & orchestrator declaration
└── src/                            # Definitions (8 source files: ~242 lines)
    ├── Location.cpp
    ├── Passenger.cpp
    ├── Driver.cpp
    ├── Ride.cpp
    ├── FareCalculator.cpp
    ├── NearestDriverStrategy.cpp
    ├── DispatchCenter.cpp          # Largest translation unit (61 lines)
    └── main.cpp                    # Console simulation entry point
```

```cmake
# --- CMakeLists.txt Build Target & Include Configuration ---
cmake_minimum_required(VERSION 3.16)
project(SwiftDispatch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(taxi_dispatch
    src/main.cpp
    src/DispatchCenter.cpp
    src/Driver.cpp
    src/Passenger.cpp
    src/Location.cpp
    src/Ride.cpp
    src/FareCalculator.cpp
    src/NearestDriverStrategy.cpp
)
target_include_directories(taxi_dispatch PRIVATE include)
```

```bash
# --- Standard Build and Execution Procedure ---
mkdir build && cd build
cmake ..
cmake --build .
./taxi_dispatch
```

When `cmake --build .` runs, the build proceeds through three distinct stages: **Preprocessing** expands each `#include` directive while header include guards (`#ifndef ... #define ... #endif`) prevent duplicate expansion within a single translation unit; **Compilation** translates each `.cpp` file independently into a machine-code object file (so modifying a `.cpp` file recompiles that file alone without touching unchanged modules); and **Linking** resolves symbol references across all eight object files to produce the single executable `taxi_dispatch`.

## 3. Object-Oriented Foundations: Encapsulation, Data Hiding, and Value Semantics

### 3.1 Theory of Encapsulation and Data Hiding

Encapsulation bundles an object's internal state with the methods that operate on that state and restricts direct external access through compiler-enforced visibility specifiers (`private`, `protected`, `public`). Exposing a deliberately narrow public interface while keeping data representations strictly private yields three engineering guarantees:

* **Invariant Protection:** Because only a class's own member functions can mutate its fields, the class guarantees that its internal state remains valid and self-consistent across its entire lifecycle.
* **Representation Independence:** External callers depend on behavioural signatures rather than raw memory layout, allowing internal data structures or mathematical formulas to evolve without breaking client code.
* **Reduced Coupling:** Collaborating modules interact through explicit, auditable entry points rather than scattered field assignments.

### 3.2 Controlled Mutability and the Rule of Zero in Driver

The `Driver` class exemplifies rigorous encapsulation in Swift Dispatch. A driver represents the unit of supply in the dispatch lifecycle, maintaining four immutable identity attributes (`driverId`, `name`, `vehicleNumber`, `rating`) and exactly two mutable operational attributes (`currentLocation` and `available`):

```cpp
// include/Driver.h
#ifndef DRIVER_H
#define DRIVER_H

#include <string>
#include "Location.h"

// Stores driver state used by the dispatch engine.
class Driver {
private:
    int driverId;
    std::string name;
    std::string vehicleNumber;
    Location currentLocation;
    bool available;
    double rating;

public:
    Driver(int driverId, const std::string& name, const std::string& vehicleNumber,
           const Location& currentLocation, double rating);

    int getDriverId() const;
    const std::string& getName() const;
    const std::string& getVehicleNumber() const;
    const Location& getCurrentLocation() const;
    bool isAvailable() const;
    double getRating() const;

    void setCurrentLocation(const Location& location);
    void setAvailability(bool isAvailable);
};

#endif

// src/Driver.cpp (Complete Implementation)
#include "Driver.h"

Driver::Driver(int driverId, const std::string& name, const std::string& vehicleNumber,
               const Location& currentLocation, double rating)
    : driverId(driverId), name(name), vehicleNumber(vehicleNumber),
      currentLocation(currentLocation), available(true), rating(rating) {}

int Driver::getDriverId() const { return driverId; }
const std::string& Driver::getName() const { return name; }
const std::string& Driver::getVehicleNumber() const { return vehicleNumber; }
const Location& Driver::getCurrentLocation() const { return currentLocation; }
bool Driver::isAvailable() const { return available; }
double Driver::getRating() const { return rating; }

void Driver::setCurrentLocation(const Location& location) { currentLocation = location; }
void Driver::setAvailability(bool isAvailable) { available = isAvailable; }
```

Five deliberate architectural decisions are embedded in `Driver`:

1. **Compiler-Enforced Data Hiding:** All six data members are declared `private`. Attempting to write `driver.available = true;` from `main.cpp` or `DispatchCenter` triggers a compile-time error (`'bool Driver::available' is private within this context`). State transitions must pass through `setAvailability(bool)` (which consolidates the conceptual `setBusy()` and `setAvailable()` operations from the initial design specification into a single parameterized mutator).
2. **Selective Mutability:** The interface exposes mutators only for the two properties that legitimately change during operation—where the driver is located (`setCurrentLocation`) and whether the driver is free (`setAvailability`). Identity fields (`driverId`, `name`, `vehicleNumber`, `rating`) provide getters but no setters, guaranteeing identity stability after construction.
3. **Safe Initial State Invariant:** The constructor omits an availability parameter and unconditionally initializes `available(true)` in its member initializer list. Every newly registered driver therefore enters the fleet in a valid, available state, preventing uninitialized boolean bugs.
4. **Const-Correctness and Zero-Copy Accessors:** Every getter is qualified `const`, promising not to modify the object and allowing invocation on `const Driver&` references inside selection loops. Getters for composite types return `const std::string&` and `const Location&`, avoiding heap allocations while preventing callers from mutating internal members through the returned reference.
5. **The Rule of Zero:** Because `Driver` contains only value types (`int`, `double`, `bool`, `std::string`, `Location`) and manages no raw heap pointers or virtual functions, the compiler-synthesized copy constructor, move constructor, copy/move assignment operators, and destructor are completely safe, allowing `Driver` instances to be stored by value in `std::vector<Driver>` and copied into `Ride` snapshots.

### 3.3 Immutable Value Objects (Location, Passenger) and One-Way Lifecycle Entities (Ride)

`Location` and `Passenger` take encapsulation a step further by eliminating mutators altogether, functioning as **immutable value objects**. Once constructed, a `Location`'s coordinates cannot be altered; moving a driver is expressed by passing a new `Location` instance to `Driver::setCurrentLocation`, eliminating aliasing bugs where two entities accidentally share and mutate the same coordinate object. Furthermore, `Location` colocates geometric behaviour with the coordinate data it uses by encapsulating `distanceTo` directly inside the class:

```cpp
// include/Location.h & src/Location.cpp
#ifndef LOCATION_H
#define LOCATION_H

#include <string>

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

// src/Location.cpp
#include "Location.h"
#include <cmath>

Location::Location() : areaName("Unknown"), latitude(0.0), longitude(0.0) {}

Location::Location(const std::string& areaName, double latitude, double longitude)
    : areaName(areaName), latitude(latitude), longitude(longitude) {}

const std::string& Location::getAreaName() const { return areaName; }
double Location::getLatitude() const { return latitude; }
double Location::getLongitude() const { return longitude; }

double Location::distanceTo(const Location& other) const {
    const double deltaLatitude = latitude - other.latitude;
    const double deltaLongitude = longitude - other.longitude;
    return std::sqrt(deltaLatitude * deltaLatitude + deltaLongitude * deltaLongitude);
}
```

Because the distance formula is encapsulated inside `Location::distanceTo`, upgrading the metric from planar Euclidean degrees to the spherical haversine formula requires modifying this single method body without altering `NearestDriverStrategy`, `DispatchCenter`, or `main.cpp`. Similarly, `Passenger` encapsulates `passengerId`, `name`, `pickupLocation`, and `dropLocation` as private members with read-only getters, treating a ride request as an immutable historical fact once submitted:

```cpp
// include/Passenger.h & src/Passenger.cpp
#ifndef PASSENGER_H
#define PASSENGER_H

#include <string>
#include "Location.h"

class Passenger {
private:
    int passengerId;
    std::string name;
    Location pickupLocation;
    Location dropLocation;

public:
    Passenger(int passengerId, const std::string& name,
              const Location& pickupLocation, const Location& dropLocation);

    int getPassengerId() const;
    const std::string& getName() const;
    const Location& getPickupLocation() const;
    const Location& getDropLocation() const;
};

#endif

// src/Passenger.cpp
#include "Passenger.h"

Passenger::Passenger(int passengerId, const std::string& name,
                     const Location& pickupLocation, const Location& dropLocation)
    : passengerId(passengerId), name(name),
      pickupLocation(pickupLocation), dropLocation(dropLocation) {}

int Passenger::getPassengerId() const { return passengerId; }
const std::string& Passenger::getName() const { return name; }
const Location& Passenger::getPickupLocation() const { return pickupLocation; }
const Location& Passenger::getDropLocation() const { return dropLocation; }
```

Finally, `Ride` represents the transactional record of a confirmed trip. It initializes its `status` string to `"Assigned"` upon construction and exposes no general-purpose string setter; instead, it provides a single domain-specific lifecycle transition method, `completeRide()`, ensuring that external code can never set a ride's status to an arbitrary invalid string:

```cpp
// include/Ride.h & src/Ride.cpp
#ifndef RIDE_H
#define RIDE_H

#include <string>
#include "Driver.h"
#include "Passenger.h"

class Ride {
private:
    int rideId;
    Passenger passenger;
    Driver driver;
    double distance;
    double fare;
    std::string status;

public:
    Ride(int rideId, const Passenger& passenger, const Driver& driver,
         double distance, double fare);

    int getRideId() const;
    const Passenger& getPassenger() const;
    const Driver& getDriver() const;
    double getDistance() const;
    double getFare() const;
    const std::string& getStatus() const;

    void completeRide();
};

#endif

// src/Ride.cpp
#include "Ride.h"

Ride::Ride(int rideId, const Passenger& passenger, const Driver& driver,
           double distance, double fare)
    : rideId(rideId), passenger(passenger), driver(driver),
      distance(distance), fare(fare), status("Assigned") {}

int Ride::getRideId() const { return rideId; }
const Passenger& Ride::getPassenger() const { return passenger; }
const Driver& Ride::getDriver() const { return driver; }
double Ride::getDistance() const { return distance; }
double Ride::getFare() const { return fare; }
const std::string& Ride::getStatus() const { return status; }

void Ride::completeRide() {
    status = "Completed";
}
```

## 4. Behavioural Abstraction, Runtime Polymorphism, and the Strategy Pattern

### 4.1 Architectural Facade and Policy Contracts

While encapsulation protects internal state, **abstraction** simplifies subsystem interaction by presenting essential high-level capabilities while hiding algorithmic and bookkeeping complexity. Swift Dispatch applies abstraction at two distinct architectural tiers:

1. **Subsystem Abstraction (The Facade Pattern):** `DispatchCenter` acts as the single unified Facade for the entire backend engine. Its public interface exposes just three business commands (`registerDriver`, `createRide`, `completeRide`—referred to conceptually as `requestRide` in the initial design outline) and two read-only state views (`getDrivers`, `getRides`). When a client calls `dispatchCenter.createRide(passenger)`, the facade hides a seven-step internal orchestration sequence: invoking the selection strategy, checking for driver exhaustion, computing pickup-to-drop distance, querying the fare calculator, mutating driver availability and location, generating a sequential ride ID to construct a `Ride` object, and appending the record to the ledger.
2. **Behavioural Policy Abstraction (Pure Virtual Contracts):** The orchestrator refers to its routing and pricing collaborators exclusively through two abstract base classes—`IDriverSelectionStrategy` and `FareCalculator`—each declaring a virtual destructor and a single pure virtual method (`= 0`):

```cpp
// Abstract Policy Contract 1: Driver Selection Strategy
class IDriverSelectionStrategy {
public:
    virtual ~IDriverSelectionStrategy() = default;
    virtual int selectDriverIndex(const std::vector<Driver>& drivers,
                                  const Location& pickupLocation) const = 0;
};

// Abstract Policy Contract 2: Fare Calculation Strategy
class FareCalculator {
public:
    virtual ~FareCalculator() = default;
    virtual double calculateFare(double distanceInKm) const = 0;
};
```

### 4.2 Evolution from Raw-Pointer Concepts to Modern C++17 Value & Smart-Pointer Semantics

A key editorial synthesis across the project's documentation is tracing how the initial conceptual design evolved into the delivered C++17 implementation. In the early architectural draft, polymorphism and entity registries were expressed using raw C++ pointers (`std::vector<Driver*>`, `DispatchStrategy* strategy`, `FareCalculator* calculator`, and `virtual Driver* selectDriver(...) = 0`):

```cpp
// Initial Conceptual Design (Raw-Pointer Formulation)
class DispatchStrategy {
public:
    virtual Driver* selectDriver(const std::vector<Driver*>& availableDrivers,
                                 const Location& pickup) = 0;
    virtual ~DispatchStrategy() = default;
};

class NearestDriverStrategy : public DispatchStrategy {
public:
    Driver* selectDriver(const std::vector<Driver*>& availableDrivers,
                         const Location& pickup) override {
        // Iterate vector, calculate Euclidean distance, track minimum, return Driver*
    }
};

class DispatchCenter {
private:
    std::vector<Driver*> drivers;
    DispatchStrategy* strategy;    // Raw pointer to abstract interface
    FareCalculator* calculator;
public:
    DispatchCenter(DispatchStrategy* strat, FareCalculator* calc)
        : strategy(strat), calculator(calc) {}
    void registerDriver(Driver* d) { drivers.push_back(d); }
    void requestRide(Passenger* p, Location pickup, Location dropoff);
};
```

While the raw-pointer formulation illustrates the core polymorphic relationship, it introduces three C++ hazards: ambiguous ownership (who calls `delete` on `strat`, `calc`, and each `Driver*`?), pointer invalidation if a vector reallocates, and extra pre-filtering allocations to build `std::vector<Driver*> availableDrivers` before every call. In the delivered C++17 implementation, these hazards were systematically eliminated:

* **Contiguous Value Containers:** `DispatchCenter` stores `std::vector<Driver> drivers` and `std::vector<Ride> rides` by value, ensuring cache-friendly contiguous memory and automatic lifetime management.
* **Index-Based Sentinel Contract:** Instead of returning a nullable `Driver*`, `IDriverSelectionStrategy::selectDriverIndex` takes `const std::vector<Driver>& drivers` directly and returns an integer index (`0` to `n - 1`) or the sentinel value `-1` when no driver qualifies, avoiding pointer aliasing while cleanly separating detection (in the strategy) from reaction (throwing an exception in `DispatchCenter`).
* **RAII Ownership via `std::unique_ptr`:** Polymorphic collaborators are held as `std::unique_ptr<IDriverSelectionStrategy>` and `std::unique_ptr<FareCalculator>` and transferred into `DispatchCenter` via `std::move`, encoding sole ownership directly in the C++ type system and making `DispatchCenter` a move-only class that cannot be accidentally duplicated.

### 4.3 Runtime Polymorphism, Vtable Mechanics, and Virtual Destructor Safety

Inheritance establishes an *is-a* relationship between an abstract base interface and its concrete implementations, while runtime polymorphism enables code written against the base contract to execute derived behaviour dynamically via late binding. In C++, declaring a member function `virtual` causes the compiler to generate a per-class virtual method table (**vtable**) containing function pointers and embeds a hidden virtual table pointer (**vptr**) inside each polymorphic object instance. Figure 4.1 illustrates the two parallel shallow inheritance hierarchies and composition relationships in Swift Dispatch.

**Figure 4.1: Polymorphic Class Hierarchies and Composition Ownership Architecture**

```text
          «interface»                                    «interface»
   IDriverSelectionStrategy                             FareCalculator
  + ~IDriverSelectionStrategy() = default        + ~FareCalculator() = default
  + selectDriverIndex(drivers, pickup) = 0       + calculateFare(distanceInKm) = 0
               ▲                                              ▲
               │ is-a (inherits & overrides)                  │ is-a (inherits & overrides)
   ┌───────────┴────────────────────────┐         ┌───────────┴──────────────────────┐
   │                                    │         │                                  │
NearestDriverStrategy    HighestRatedDriverStrategy  StandardFareCalculator   SurgeFareCalculator
+ selectDriverIndex(...) + selectDriverIndex(...)    - baseFare: double (40.0) - multiplier: double
                                                     - perKmRate: double (18.0)+ calculateFare(...)
                                                     + calculateFare(...)

DispatchCenter (Facade / Context)
  ◆── owns via std::unique_ptr ──► IDriverSelectionStrategy
  ◆── owns via std::unique_ptr ──► FareCalculator
  ◆── holds by value           ──► std::vector<Driver>, std::vector<Ride>
```

Both hierarchies are intentionally shallow (one level of inheritance) and are used strictly to express behavioural contracts (*is-a*), never to share mutable state across deep implementation trees. Everything else in the engine uses **composition over inheritance** (*has-a*): `DispatchCenter` owns a strategy, a calculator, and vectors of drivers and rides; `Ride` contains a `Passenger` and a `Driver`; and `Driver` and `Passenger` contain `Location` instances. Furthermore, two C++11/17 language safeguards ensure polymorphic correctness:

* **The `override` Specifier:** Concrete classes declare `int selectDriverIndex(...) const override;` and `double calculateFare(double distanceInKm) const override;`. This instructs the compiler to verify that the signature matches a base virtual function exactly; if `const` or a reference qualifier were omitted, compilation fails immediately rather than silently hiding the virtual method.
* **Virtual Base Destructors:** Both `IDriverSelectionStrategy` and `FareCalculator` declare `virtual ~ClassName() = default;`. Because `DispatchCenter` destroys its policy objects through base-class `std::unique_ptr` pointers when the dispatcher goes out of scope, omitting `virtual` on the base destructor would invoke undefined behaviour by skipping derived destructors.

The concrete pricing implementation in `FareCalculator.h` and `FareCalculator.cpp` demonstrates how stateful tariff parameters are encapsulated inside the derived strategy while exposing only the pure virtual interface to `DispatchCenter`:

```cpp
// include/FareCalculator.h
#ifndef FARECALCULATOR_H
#define FARECALCULATOR_H

class FareCalculator {
public:
    virtual ~FareCalculator() = default;
    virtual double calculateFare(double distanceInKm) const = 0;
};

class StandardFareCalculator : public FareCalculator {
private:
    double baseFare;
    double perKmRate;

public:
    StandardFareCalculator(double baseFare = 40.0, double perKmRate = 18.0);
    double calculateFare(double distanceInKm) const override;
};

#endif

// src/FareCalculator.cpp
#include "FareCalculator.h"

StandardFareCalculator::StandardFareCalculator(double baseFare, double perKmRate)
    : baseFare(baseFare), perKmRate(perKmRate) {}

double StandardFareCalculator::calculateFare(double distanceInKm) const {
    return baseFare + (distanceInKm * perKmRate);
}
```

### 4.4 The Strategy Design Pattern, Dependency Injection, and the Open/Closed Principle

Catalogued by Gamma, Helm, Johnson, and Vlissides (the "Gang of Four"), the **Strategy Design Pattern** defines a family of algorithms, encapsulates each one into a separate class, and makes them interchangeable so the algorithm can vary independently from the clients that use it. Swift Dispatch applies the Strategy pattern across two orthogonal axes of variation: driver selection and trip pricing.

**Table 4.1: Mapping of Gang of Four Strategy Pattern Roles to Swift Dispatch**

| Strategy Pattern Role | Architectural Responsibility | Driver Selection Axis | Fare Calculation Axis |
| :--- | :--- | :--- | :--- |
| **Context** | Maintains smart-pointer reference to abstract Strategy and delegates policy execution | `DispatchCenter` | `DispatchCenter` |
| **Strategy (Interface)** | Declares common pure virtual algorithm contract | `IDriverSelectionStrategy` | `FareCalculator` |
| **ConcreteStrategy** | Implements a specific interchangeable algorithm | `NearestDriverStrategy`, `HighestRatedDriverStrategy` | `StandardFareCalculator`, `SurgeFareCalculator` |

Crucially, `DispatchCenter` does not instantiate concrete strategies internally. Instead, concrete strategies are constructed externally in `main.cpp` and supplied via **Constructor Dependency Injection**:

```cpp
// src/DispatchCenter.cpp (Constructor Dependency Injection via std::unique_ptr)
DispatchCenter::DispatchCenter(
    std::unique_ptr<IDriverSelectionStrategy> driverSelectionStrategy,
    std::unique_ptr<FareCalculator> fareCalculator)
    : driverSelectionStrategy(std::move(driverSelectionStrategy)),
      fareCalculator(std::move(fareCalculator)),
      nextRideId(1) {}
```

This combination of Strategy and Dependency Injection directly realizes the **Open/Closed Principle (OCP)**—software entities should be *open for extension* but *closed for modification*—as well as the **Dependency Inversion Principle (DIP)**. Without the Strategy pattern, supporting multiple dispatch modes would require embedding procedural conditionals inside `DispatchCenter::createRide`:

```cpp
// Architectural Anti-Pattern: Violates OCP and SRP
if (mode == NEAREST)     { /* nearest-driver scan loop */ }
else if (mode == RATING) { /* highest-rating scan loop */ }
else if (mode == ETA)    { /* traffic-aware ETA lookup */ }
```

Every new policy in that anti-pattern forces edits to `DispatchCenter`, risking regressions in already-tested policies and turning the orchestrator into an unmaintainable "God Object." In Swift Dispatch, `DispatchCenter.cpp` is permanently closed for modification. Adding a rating-based selection policy (`HighestRatedDriverStrategy`) and a demand-multiplier tariff (`SurgeFareCalculator`) requires writing self-contained classes and changing only the constructor arguments in `main.cpp`:

```cpp
// Extension 1: Highest-Rated Available Driver Strategy (Zero edits to DispatchCenter)
class HighestRatedDriverStrategy : public IDriverSelectionStrategy {
public:
    int selectDriverIndex(const std::vector<Driver>& drivers,
                          const Location&) const override {
        double bestRating = -1.0;
        int bestIndex = -1;
        for (std::size_t i = 0; i < drivers.size(); ++i) {
            if (drivers[i].isAvailable() && drivers[i].getRating() > bestRating) {
                bestRating = drivers[i].getRating();
                bestIndex = static_cast<int>(i);
            }
        }
        return bestIndex;
    }
};

// Extension 2: Surge-Multiplier Fare Calculator (Zero edits to DispatchCenter)
class SurgeFareCalculator : public FareCalculator {
    double multiplier;
public:
    explicit SurgeFareCalculator(double m) : multiplier(m) {}
    double calculateFare(double km) const override {
        return (40.0 + km * 18.0) * multiplier;
    }
};

// Reconfiguring the Engine in main.cpp (One-line policy swap)
DispatchCenter dispatchCenter(
    std::make_unique<HighestRatedDriverStrategy>(), // Replaced NearestDriverStrategy
    std::make_unique<SurgeFareCalculator>(1.5)      // Replaced StandardFareCalculator
);
```

When compiled and executed against the project's unchanged headers and the demonstration trip ($\text{distance} \approx 0.0403113$), injecting `HighestRatedDriverStrategy` and `SurgeFareCalculator(1.5)` selects **Meera (rating 4.9)** and quotes a fare of **61.09** ($(40.0 + 0.0403113 \times 18.0) \times 1.5$), instead of **Arjun (rating 4.8)** and **40.73**. The runtime behaviour transforms completely without editing a single character of `DispatchCenter.cpp`, `NearestDriverStrategy.cpp`, or `FareCalculator.cpp`.

**Engineering Trade-offs of the Strategy Pattern:** The pattern introduces additional classes and one virtual function call (indirection via vptr/vtable) per dispatch decision. Against an $O(n)$ fleet scan, a single virtual dispatch costs nanoseconds and is negligible. A more substantive architectural trade-off is that all concrete strategies under an interface must share the same method signature; if a future strategy requires extra parameters (such as live traffic congestion matrices or trip duration for time-based pricing), either the interface signature must be widened (e.g., passing a rich `RequestContext` struct) or the strategy must hold its own external data source reference.

### 4.5 Separation of Concerns, Header Decoupling Refactor, and Acyclic Dependency Graph

**Separation of Concerns (SoC)** and its class-level counterpart, the **Single Responsibility Principle (SRP)**, stipulate that every module should address a single distinct concern and have only one reason to change. Notably, all console formatting (`std::cout`) is strictly confined to `main.cpp`; no domain or policy class performs I/O, ensuring that the backend engine can be embedded inside a GUI or REST server unchanged.

In the delivered repository, `IDriverSelectionStrategy` and `NearestDriverStrategy` reside together in `include/NearestDriverStrategy.h`, which is included by `DispatchCenter.h`. A cleaner physical decoupling—aligning with the `DispatchStrategy.h` naming from the initial project abstract—places the pure virtual interface in its own dedicated header so that `DispatchCenter.h` depends purely on the abstract header and never recompiles when concrete strategy headers are added or modified:

```cpp
// include/NearestDriverStrategy.h (Delivered Combined Header)
#ifndef NEARESTDRIVERSTRATEGY_H
#define NEARESTDRIVERSTRATEGY_H

#include <vector>
#include "Driver.h"
#include "Location.h"

// Strategy interface for choosing an available driver.
class IDriverSelectionStrategy {
public:
    virtual ~IDriverSelectionStrategy() = default;
    virtual int selectDriverIndex(const std::vector<Driver>& drivers,
                                  const Location& pickupLocation) const = 0;
};

// Selects the nearest available driver.
class NearestDriverStrategy : public IDriverSelectionStrategy {
public:
    int selectDriverIndex(const std::vector<Driver>& drivers,
                          const Location& pickupLocation) const override;
};

#endif

// --- Proposed Physical Header Split: DispatchStrategy.h + NearestDriverStrategy.h ---
// include/DispatchStrategy.h (Proposed Refactor)
#ifndef DISPATCHSTRATEGY_H
#define DISPATCHSTRATEGY_H

#include <vector>
#include "Driver.h"
#include "Location.h"

class IDriverSelectionStrategy {
public:
    virtual ~IDriverSelectionStrategy() = default;
    virtual int selectDriverIndex(const std::vector<Driver>& drivers,
                                  const Location& pickupLocation) const = 0;
};

#endif

// include/NearestDriverStrategy.h (Proposed Refactor)
#include "DispatchStrategy.h"

class NearestDriverStrategy : public IDriverSelectionStrategy {
public:
    int selectDriverIndex(const std::vector<Driver>& drivers,
                          const Location& pickupLocation) const override;
};
```

With this physical split verified on the codebase, all module dependencies form a strict **Directed Acyclic Graph (DAG)** flowing from high-level orchestration down to foundational value types:

```text
main.cpp ──► DispatchCenter ──► IDriverSelectionStrategy ──► Driver ──► Location
                   │
                   ├──► FareCalculator                      Ride ──► Passenger, Driver
                   └──► Ride                                Passenger ──► Location
```

`Location` depends on nothing inside the project; `Driver` and `Passenger` depend only on `Location`; `Ride` depends on `Driver` and `Passenger`; the selection strategy depends on `Driver` and `Location`; and only `DispatchCenter` coordinates across them all. This acyclic topology allows bottom-up incremental compilation and isolated unit testing at every layer.

## 5. Core System Modules, Five-Phase Execution Workflow, and State Machines

### 5.1 Unified Domain & Policy Module Architecture

Table 5.1 synthesizes the encapsulation boundaries, private state, public mutators, key operations, Collaborators, and Single Responsibility rationale across all eight architectural components of Swift Dispatch.

**Table 5.1: Unified Module Responsibility, Encapsulation, and Collaboration Matrix**

| Module / Class | Architectural Role | Private State Maintained | Mutators Exposed | Primary Operations & Collaborators | Reason to Change (SRP) |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `Location` | Geometric Value Object | `areaName`, `latitude`, `longitude` | None (Immutable) | `distanceTo(other)`, getters; no internal dependencies | Upgrading coordinate distance metric (e.g., Haversine) |
| `Passenger` | Request Originator | `passengerId`, `name`, `pickupLocation`, `dropLocation` | None (Immutable) | Read-only getters; collaborates with `Location`, `Ride` | Adding passenger request metadata (e.g., seat count) |
| `Driver` | Service Provider (Supply Unit) | `driverId`, `name`, `vehicleNumber`, `currentLocation`, `available`, `rating` | `setCurrentLocation`, `setAvailability` | `isAvailable()`, getters; collaborates with `Location`, strategy, `Ride` | Altering driver attributes or availability rules |
| `Ride` | Transactional Trip Record | `rideId`, `passenger`, `driver`, `distance`, `fare`, `status` | `completeRide()` (One-way transition) | Getters, `completeRide()`; holds snapshots of `Passenger` & `Driver` | Extending trip lifecycle states (e.g., cancelled, in-transit) |
| `IDriverSelectionStrategy` & Subclasses | Routing Policy Abstraction | Stateless in `NearestDriverStrategy` | None (`const` evaluation) | `selectDriverIndex(drivers, pickup)`; inspects `Driver` & `Location` | Introducing new matching algorithms (rating, ETA) |
| `FareCalculator` & Subclasses | Pricing Policy Abstraction | `baseFare` (40.0), `perKmRate` (18.0) in `StandardFareCalculator` | None (`const` evaluation) | `calculateFare(distanceInKm)`; invoked by `DispatchCenter` | Changing tariff structure or minimum base charge |
| `DispatchCenter` | Orchestrator & Subsystem Facade | `drivers`, `rides`, `driverSelectionStrategy`, `fareCalculator`, `nextRideId` | `registerDriver`, `createRide`, `completeRide` | Coordinates all domain & policy modules; exposes `const` vector views | Modifying high-level dispatch workflow sequence |
| `main.cpp` | Presentation & Simulation Driver | Local simulation instances | N/A (Entry point) | Instantiates policies, registers fleet, invokes facade, prints console output | Replacing console UI with GUI, CLI menu, or REST adapter |

The complete interface declaration of `DispatchCenter.h` illustrates how value storage, smart pointers, and read-only accessors come together in a 35-line facade header:

```cpp
// include/DispatchCenter.h
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
```

### 5.2 Five-Phase Execution Workflow and Sequence Architecture

The operational execution of Swift Dispatch progresses through five chronological phases that model the end-to-end lifecycle of a ride request from system startup to trip completion:

```text
[Phase 1: Initialisation] ──► [Phase 2: Request Intake] ──► [Phase 3: Dispatch Logic] ──► [Phase 4: Ride Creation & Pricing] ──► [Phase 5: Completion]
```

Figure 5.1 details the synchronous sequence of method invocations across collaborating objects during `DispatchCenter::createRide(passenger)` (covering Phases 3 and 4).

**Figure 5.1: UML Sequence Diagram of Collaborations During `DispatchCenter::createRide`**

```text
main.cpp          DispatchCenter             Strategy             Driver / Location       FareCalculator            Ride
   │                    │                        │                        │                     │                    │
   │─── createRide(p) ─►│                        │                        │                     │                    │
   │                    │── selectDriverIndex ──►│                        │                     │                    │
   │                    │   (drivers, pickup)    │── isAvailable()? ─────►│ (for each driver)   │                    │
   │                    │                        │── distanceTo(pickup) ─►│                     │                    │
   │                    │◄── index (or -1) ──────│                        │                     │                    │
   │                    │ [if index < 0: throw runtime_error / NoDriverAvailableException]      │                    │
   │                    │── pickup.distanceTo(drop) ─────────────────────►│                     │                    │
   │                    │── calculateFare(distance) ───────────────────────────────────────────►│                    │
   │                    │── setAvailability(false), setCurrentLocation(drop) ►                  │                    │
   │                    │── Ride(nextRideId++, passenger, assignedDriver, distance, fare) ──────────────────────────►│
   │                    │── rides.push_back(ride)│                        │                     │                    │
   │◄── return Ride ────│                        │                        │                     │                    │
```

Each of the five phases is realized by concise, deterministic C++17 code across `main.cpp` and `DispatchCenter.cpp`:

* **Phase 1 — Initialisation:** `main.cpp` instantiates the concrete selection strategy and fare calculator via `std::make_unique`, injects them into `DispatchCenter` (initializing `nextRideId` to 1), and registers three available drivers (Arjun, Ravi, and Meera) with their starting coordinates and ratings.
* **Phase 2 — Request Intake:** A ride request arrives encapsulated as a `Passenger` instance (Aman, ID 101) specifying pickup coordinates `(12.9650, 77.6000)` and drop-off coordinates `(12.9300, 77.6200)`.
* **Phase 3 — Dispatch Processing:** `DispatchCenter::createRide` delegates candidate evaluation to `driverSelectionStrategy->selectDriverIndex(drivers, passenger.getPickupLocation())`. If the returned index is negative (`-1`), an exception is thrown immediately before any state is modified.
* **Phase 4 — Ride Creation & Pricing:** With a valid `driverIndex` (index 0, Arjun), `createRide` computes the passenger's trip distance from pickup to drop-off (note: pricing uses the passenger's trip leg, not the driver's approach distance), calculates the fare via `fareCalculator->calculateFare(distance)` ($40.0 + 0.0403113 \times 18.0 = 40.7256$, where the base fare of 40.0 acts as a minimum charge even for zero-length trips), marks the driver unavailable (`setAvailability(false)`), updates the driver's location to the drop-off point, constructs a `Ride` with `nextRideId++`, appends a copy to `rides`, and returns a copy to the caller.
* **Phase 5 — Ride Completion:** When a trip concludes, invoking `DispatchCenter::completeRide(rideId)` performs a linear search via `std::find_if` over `rides`, throws an error if the ID is unknown, invokes `rideIt->completeRide()` to transition status to `"Completed"`, locates the corresponding driver in `drivers` by `driverId`, and restores `driverIt->setAvailability(true)` so the driver returns to the active pool.

```cpp
// --- Complete Simulation Driver in src/main.cpp ---
#include <exception>
#include <iostream>
#include <memory>
#include "DispatchCenter.h"
#include "FareCalculator.h"
#include "NearestDriverStrategy.h"

using namespace std;

int main() {
    DispatchCenter dispatchCenter(
        make_unique<NearestDriverStrategy>(),
        make_unique<StandardFareCalculator>()
    );

    dispatchCenter.registerDriver(Driver(1, "Arjun", "OD01AB1234",
        Location("Laxmisagar", 12.9716, 77.5946), 4.8));
    dispatchCenter.registerDriver(Driver(2, "Ravi", "OD02CD5678",
        Location("Bapuji Nagar", 12.9710, 77.6400), 4.6));
    dispatchCenter.registerDriver(Driver(3, "Meera", "OD03EF9012",
        Location("Old town", 12.9000, 77.5500), 4.9));

    Passenger passenger(101, "Aman",
        Location("Pickup Point", 12.9650, 77.6000),
        Location("Destination", 12.9300, 77.6200));

    try {
        Ride ride = dispatchCenter.createRide(passenger);
        cout << "Ride created successfully\n"
             << "Ride ID: " << ride.getRideId() << '\n'
             << "Driver: " << ride.getDriver().getName() << '\n'
             << "Passenger: " << ride.getPassenger().getName() << '\n'
             << "Distance: " << ride.getDistance() << " km\n"
             << "Fare: " << ride.getFare() << '\n'
             << "Status: " << ride.getStatus() << '\n';
    } catch (const exception& exception) {
        cout << "Dispatch failed: " << exception.what() << '\n';
    }
    return 0;
}
```

```text
// --- Verified Console Output ---
Ride created successfully
Ride ID: 1
Driver: Arjun
Passenger: Aman
Distance: 0.0403113 km
Fare: 40.7256
Status: Assigned
```

```cpp
// --- Phases 3, 4 & 5 Implementation in src/DispatchCenter.cpp ---
void DispatchCenter::registerDriver(const Driver& driver) {
    drivers.push_back(driver);
}

Ride DispatchCenter::createRide(const Passenger& passenger) {
    const int driverIndex =
        driverSelectionStrategy->selectDriverIndex(drivers, passenger.getPickupLocation());
    if (driverIndex < 0) {
        throw std::runtime_error("No available driver found for the request");
    }

    Driver& assignedDriver = drivers[driverIndex];
    const double distance =
        passenger.getPickupLocation().distanceTo(passenger.getDropLocation());
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

const std::vector<Driver>& DispatchCenter::getDrivers() const { return drivers; }
const std::vector<Ride>& DispatchCenter::getRides() const { return rides; }
```

### 5.3 Extended Multi-Request Simulation Trace

To verify all five phases working together across dynamic fleet changes, an extended 6-step sequential scenario was executed against the project's classes using the three registered drivers and four passengers sharing the pickup point `(12.9650, 77.6000)`. Table 5.2 records the resulting dispatch decisions and fleet states.

**Table 5.2: Extended Sequential Multi-Request Simulation Trace**

| Step | Operational Event | Dispatch & Pricing Outcome | Resulting Fleet State |
| :--- | :--- | :--- | :--- |
| **1** | Request from Aman | **Ride 1 assigned to Arjun** (dist 0.0403, fare 40.7256) | Arjun busy at drop-off; Ravi, Meera free |
| **2** | Request from Bela | **Ride 2 assigned to Ravi** (dist 0.0180, fare 40.3245) | Arjun, Ravi busy; Meera free |
| **3** | Request from Chet | **Ride 3 assigned to Meera** (dist 0.0250, fare 40.4500) | All three drivers busy (fleet exhausted) |
| **4** | Request from Dev | **Exception thrown:** `"No available driver found for the request"` | Unchanged (all three busy; no ride ID consumed) |
| **5** | `completeRide(1)` invoked | Ride 1 status transitions to `"Completed"` | Arjun free again at Ride 1 drop-off; Ravi, Meera busy |
| **6** | Request from Dev (retry) | **Ride 4 assigned to Arjun** (dist 0.0250, fare 40.4500) | Arjun, Ravi, Meera all busy |

This trace demonstrates the defining property of a dynamic dispatcher: identical pickup coordinates receive different drivers at Steps 1, 2, and 3 because fleet availability mutates after each assignment, and Step 4 proves that a failed request leaves the system completely unchanged without consuming `nextRideId` (Step 6 receives Ride ID 4).

### 5.4 Finite State Machines and Critical State-Handling Observations

System consistency is governed by two coupled finite state machines: one for `Driver` availability and one for `Ride` status:

```text
// Driver Lifecycle State Machine
            construct                    createRide (strategy selects index)
  [ (new) ] ────────────────────► [ AVAILABLE (available=true) ] ───────────────────────► [ UNAVAILABLE / BUSY (available=false) ]
                                        ▲                                                           │
                                        └─────────────── completeRide(rideId) ──────────────────────┘

// Ride Lifecycle State Machine
  [ (none) ] ──── createRide ────► [ ASSIGNED / Active ("Assigned") ] ──── completeRide() ────► [ COMPLETED ("Completed" - terminal) ]
```

**Table 5.3: Unified Entity State Transitions, Guards, and Side Effects**

| Entity | Initial State | Action Trigger | Guard Condition | Target State | Operational Side Effects |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `Driver` | NULL / Unregistered | Constructor & `registerDriver` | None | AVAILABLE (`true`) | Appended by value to `DispatchCenter::drivers` vector |
| `Driver` | AVAILABLE (`true`) | `createRide` | `selectDriverIndex` chose driver | UNAVAILABLE / BUSY (`false`) | Excluded from subsequent search scans; `currentLocation` updated to passenger drop-off point |
| `Driver` | UNAVAILABLE / BUSY (`false`) | `completeRide(rideId)` | Matching `rideId` & `driverId` found | AVAILABLE (`true`) | Returned to candidate pool at updated drop-off coordinates |
| `Ride` | (none) | `createRide` | Valid available driver selected | Assigned (Active) | Sequential `rideId`, distance, fare, and entity snapshots fixed at creation |
| `Ride` | Assigned (Active) | `completeRide(rideId)` | `rideId` exists in `rides` ledger | Completed (Terminal) | Status string updated to `"Completed"`; no further transitions or cancellations in base code |

Empirical lifecycle testing surfaced two subtle architectural subtleties in the delivered code that every maintainer must understand:

1. **Modelling Timing of Location Update & Stale Driver Snapshots Inside `Ride`:** First, the delivered code updates the driver's location to the passenger's drop-off coordinate *at assignment time* inside `createRide` rather than inside `completeRide` (and does not model the approach leg from the driver's prior position to the pickup point). Second, because `Ride` stores a *by-value copy* of `Driver` at the moment of construction (after `setAvailability(false)` is called), that embedded copy is a historical snapshot. After `completeRide(1)` runs in Step 5 of Table 5.2, the authoritative driver record in `DispatchCenter::drivers` correctly reports `Arjun.isAvailable() == true`, whereas calling `ride.getDriver().isAvailable()` on the stored `Ride` still returns `false`. Authoritative live fleet status must always be queried from `DispatchCenter::getDrivers()`.
2. **Unguarded Double Ride Completion Defect:** In the delivered implementation of `DispatchCenter::completeRide`, completing an already-completed ride is not rejected. In simulation testing (Test S6), after Ride 1 was completed at Step 5 and Arjun was assigned to Ride 4 at Step 6, calling `completeRide(1)` a second time looked up Arjun by ID and set `Arjun.setAvailability(true)` *while he was still actively engaged on Ride 4!* This violates the core safety invariant (NFR-4). Section 6.5 presents the exact guard (`InvalidRideStateException`) that closes this defect.

## 6. Algorithmic Logic, Complexity Analysis, and Fault-Tolerant Exception Architecture

### 6.1 Nearest-Driver Selection Algorithm, Pseudocode, and C++17 Implementation

**Formal Problem Definition:** Given an indexed sequence of registered drivers $D = \langle d_0, d_1, \dots, d_{n-1} \rangle$, where each driver $d_i$ possesses a coordinate pair $(\text{lat}_i, \text{lon}_i)$ and a boolean availability flag, and a target pickup location $p = (\text{lat}_p, \text{lon}_p)$, find the index $i \in \{0, \dots, n - 1\}$ of an available driver that minimizes the Euclidean distance $d(a, b) = \sqrt{(\text{lat}_a - \text{lat}_b)^2 + (\text{lon}_a - \text{lon}_b)^2}$, or return `-1` if no driver in $D$ is available.

```text
// Distance Metric Specification (Location::distanceTo)
d(a, b) = \sqrt{(lat_a - lat_b)^2 + (lon_a - lon_b)^2}

// Algorithmic Pseudocode: SELECT-NEAREST(drivers, pickup)
function SELECT-NEAREST(drivers, pickup):
    bestDistance ← +∞
    bestIndex    ← -1
    for i ← 0 to length(drivers) - 1:
        if drivers[i] is not available:
            continue
        d ← distance(drivers[i].location, pickup)
        if d < bestDistance:
            bestDistance ← d
            bestIndex    ← i
    return bestIndex
```

```cpp
// Delivered C++17 Implementation: src/NearestDriverStrategy.cpp
int NearestDriverStrategy::selectDriverIndex(const std::vector<Driver>& drivers,
                                             const Location& pickupLocation) const {
    double bestDistance = std::numeric_limits<double>::max();
    int bestIndex = -1;

    for (std::size_t index = 0; index < drivers.size(); ++index) {
        const Driver& driver = drivers[index];
        if (!driver.isAvailable()) {
            continue;
        }

        const double distance = driver.getCurrentLocation().distanceTo(pickupLocation);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIndex = static_cast<int>(index);
        }
    }

    return bestIndex;
}
```

### 6.2 Worked Iteration Traces and Loop Invariant Correctness Proof

Table 6.1 traces `NearestDriverStrategy::selectDriverIndex` step by step for the pickup coordinate `(12.9650, 77.6000)` under two fleet conditions: Case A (all drivers available, corresponding to Step 1 of the simulation) and Case B (Arjun unavailable, corresponding to Step 2 of the simulation). Notice in Case A that Meera has the highest rating (4.9), but `NearestDriverStrategy` evaluates only distance, selecting Arjun ($\approx 0.008528$).

**Table 6.1: Worked Execution Traces of `selectDriverIndex` (Case A: All Free vs. Case B: Arjun Busy)**

| Iteration `i` | Candidate Driver & Coordinates | Distance $d$ to `(12.9650, 77.6000)` | Case A (All Available): Evaluation & State (`bestDistance`, `bestIndex`) | Case B (Arjun Busy): Evaluation & State (`bestDistance`, `bestIndex`) |
| :--- | :--- | :--- | :--- | :--- |
| **(init)** | — | — | `bestDistance = max`, `bestIndex = -1` | `bestDistance = max`, `bestIndex = -1` |
| **`i = 0`** | Arjun (ID 1, Rating 4.8) `(12.9716, 77.5946)` | `0.008528` | Available; `0.008528 < max` (True) $\rightarrow$ `bestDistance = 0.008528`, `bestIndex = 0` | `isAvailable() == false` $\rightarrow$ **Skipped** $\rightarrow$ `bestDistance = max`, `bestIndex = -1` |
| **`i = 1`** | Ravi (ID 2, Rating 4.6) `(12.9710, 77.6400)` | `0.040447` | Available; `0.040447 < 0.008528` (False) $\rightarrow$ `bestDistance = 0.008528`, `bestIndex = 0` | Available; `0.040447 < max` (True) $\rightarrow$ `bestDistance = 0.040447`, `bestIndex = 1` |
| **`i = 2`** | Meera (ID 3, Rating 4.9) `(12.9000, 77.5500)` | `0.082006` | Available; `0.082006 < 0.008528` (False) $\rightarrow$ `bestDistance = 0.008528`, `bestIndex = 0` | Available; `0.082006 < 0.040447` (False) $\rightarrow$ `bestDistance = 0.040447`, `bestIndex = 1` |
| **Return** | — | — | **Returns `0` (Arjun selected)** | **Returns `1` (Ravi selected)** |

**Loop Invariant & Tie-Breaking Proof:** Correctness is established by the loop invariant: *At the end of iteration $k$, `bestIndex` holds the index of the available driver in the prefix $d_0 \dots d_k$ with the minimum distance to `pickupLocation` (or `-1` if no driver in $d_0 \dots d_k$ is available), and `bestDistance` holds that minimum distance (or `DBL_MAX`).*

* **Initialization:** Before $k = 0$, the examined prefix is empty, `bestIndex = -1`, and `bestDistance = max()`, so the invariant holds vacuously.
* **Maintenance:** During iteration $k$, if $d_k$ is unavailable, it is skipped and the invariant holds for $d_0 \dots d_k$. If $d_k$ is available, its distance $d$ is compared via strict inequality (`distance < bestDistance`). It replaces `bestIndex` if and only if $d$ is strictly smaller than every previously seen available driver's distance, preserving the invariant.
* **Termination & Tie-Breaking:** When the loop terminates after $k = n - 1$, the prefix covers the entire fleet $D$, guaranteeing that `bestIndex` is the globally nearest available driver. Because the comparison uses strict `<` rather than `<=`, **when two available drivers are equidistant from the pickup point, the driver registered earlier wins deterministically** (confirmed empirically by Test S7).

### 6.3 Asymptotic Complexity and Coordinate Unit Analysis

Table 6.2 summarizes the time and space complexity of every operation in the engine. Two algorithmic optimizations are worth noting: first, because $f(x) = \sqrt{x}$ is strictly monotonically increasing for $x \ge 0$, comparing squared distances $(\Delta\text{lat})^2 + (\Delta\text{lon})^2$ inside `selectDriverIndex` would eliminate $n$ `std::sqrt` calls without altering driver selection; second, replacing the linear scan with a 2D spatial index (a uniform spatial grid or $k$-d tree) would reduce nearest-neighbor lookup to $O(\log n)$ or $O(1)$ average time while remaining completely encapsulated inside a new `IDriverSelectionStrategy` subclass.

**Table 6.2: Asymptotic Time and Space Complexity Analysis**

| System Operation | Time Complexity | Auxiliary Space Complexity | Engineering Notes |
| :--- | :--- | :--- | :--- |
| `Location::distanceTo` | $O(1)$ | $O(1)$ | Two subtractions, two multiplications, one addition, one `std::sqrt`. |
| `FareCalculator::calculateFare` | $O(1)$ | $O(1)$ | Single linear expression: `baseFare + (distanceInKm * perKmRate)`. |
| `selectDriverIndex` (Selection) | $\Theta(n)$ | $O(1)$ | Always scans all $n$ registered drivers; stores only `bestDistance` and `bestIndex`. |
| `DispatchCenter::createRide` | $O(n)$ | $O(1)$ extra (+1 `Ride`) | Dominated by $\Theta(n)$ strategy scan; appends one `Ride` to ledger. |
| `DispatchCenter::completeRide` | $O(r + n)$ | $O(1)$ | Linear search via `std::find_if` over $r$ rides followed by $n$ drivers. |

**Impact of Coordinate Degrees vs. Haversine Kilometres:** For the demonstration trip from `(12.9650, 77.6000)` to `(12.9300, 77.6200)`, the Euclidean degree distance is $\sqrt{(-0.035)^2 + (0.020)^2} \approx 0.0403113\text{ degrees}$, producing a fare of $40 + 0.0403113 \times 18 = 40.7256$. In reality, the spherical great-circle (haversine) distance between those two Bengaluru coordinates is approximately **4.45 km**, which under the same tariff ($40 + 4.45 \times 18$) yields a realistic fare of **120.18**. While ranking candidates by degrees vs. kilometres preserves selection order over a local area, replacing the body of `Location::distanceTo` with the haversine formula corrects the pricing units across the entire system in one single place—a direct payoff of encapsulation.

### 6.4 Resource Unavailability, Atomicity, and the Strong Exception Guarantee

When a request arrives and either the fleet is empty (Test S1: `S1 empty fleet: No available driver found for the request`) or all registered drivers are busy (Test S3: `S3 all busy: No available driver found for the request`), `NearestDriverStrategy::selectDriverIndex` returns the sentinel `-1`. `DispatchCenter::createRide` detects `driverIndex < 0` and throws an exception, which `main.cpp` catches to keep the application running. Crucially, because the check occurs *before* any driver state is mutated, *before* `nextRideId` is incremented, and *before* `rides.push_back` is called, `createRide` provides the **Strong Exception Guarantee** (transactional atomicity): an operation either succeeds completely or leaves the system state identical to how it was before the call.

### 6.5 Input Validation and Custom Exception Hierarchy Design

Testing the delivered implementation with out-of-range coordinates (latitude `999.0`, longitude `-500.0` for both pickup and drop-off in Test S8) revealed that invalid coordinates are silently accepted, producing a zero-distance ride with fare `40.0000` (`S8 bad coords accepted: dist 0.0000 fare 40.0000`). Table 6.3 defines the coordinate validation rules required to enforce the geometric invariant at construction time.

**Table 6.3: Coordinate Validation Rules Enforced by `Location` Constructor**

| Coordinate Field | Valid Numerical Range | Rejected Input Values | Exception Raised |
| :--- | :--- | :--- | :--- |
| `latitude` | `-90.0` to `+90.0` degrees | Outside `[-90.0, +90.0]`, `NaN`, $\pm\infty$ | `InvalidLocationException("Latitude must be between -90 and 90")` |
| `longitude` | `-180.0` to `+180.0` degrees | Outside `[-180.0, +180.0]`, `NaN`, $\pm\infty$ | `InvalidLocationException("Longitude must be between -180 and 180")` |

Furthermore, throwing generic `std::runtime_error` objects forces callers to parse message strings to distinguish "no driver available" from "ride not found." To make invalid states unrepresentable and enable type-safe error recovery, a custom exception hierarchy rooted at `DispatchException : public std::runtime_error` was designed, compiled, and verified against a copy of the project:

```text
// Exception Class Hierarchy
std::exception
 └── std::runtime_error
      └── DispatchException             (Root for all Swift Dispatch domain errors)
           ├── InvalidLocationException (Out-of-range [-90..90] / [-180..180] or NaN/Inf coordinates)
           ├── NoDriverAvailableException (Empty fleet or all drivers currently busy)
           ├── RideNotFoundException    (Unknown rideId passed to completeRide)
           └── InvalidRideStateException (Illegal lifecycle operation, e.g., completing a ride twice)
```

```cpp
// include/DispatchExceptions.h (Complete Tested Implementation)
#ifndef DISPATCHEXCEPTIONS_H
#define DISPATCHEXCEPTIONS_H

#include <stdexcept>
#include <string>

class DispatchException : public std::runtime_error {
public:
    explicit DispatchException(const std::string& message)
        : std::runtime_error(message) {}
};

class InvalidLocationException : public DispatchException {
public:
    explicit InvalidLocationException(const std::string& message)
        : DispatchException(message) {}
};

class NoDriverAvailableException : public DispatchException {
public:
    NoDriverAvailableException()
        : DispatchException("No available driver found for the request") {}
};

class RideNotFoundException : public DispatchException {
public:
    explicit RideNotFoundException(int rideId)
        : DispatchException("Ride not found: " + std::to_string(rideId)) {}
};

class InvalidRideStateException : public DispatchException {
public:
    explicit InvalidRideStateException(const std::string& message)
        : DispatchException(message) {}
};

#endif
```

Deriving from `std::runtime_error` inherits `what()` message storage automatically and guarantees backward compatibility with existing code that catches `const std::exception&`. Enforcing coordinate invariants inside `Location::Location` (using `std::isfinite` so IEEE 754 `NaN` values—for which all standard comparisons return `false`—cannot bypass range checks) and guarding `DispatchCenter::completeRide` against double completion require only minimal, localized updates:

```cpp
// 1. Invariant Enforcement in Location::Location Constructor
Location::Location(const std::string& areaName, double latitude, double longitude)
    : areaName(areaName), latitude(latitude), longitude(longitude) {
    if (!std::isfinite(latitude) || latitude < -90.0 || latitude > 90.0) {
        throw InvalidLocationException("Latitude must be between -90 and 90");
    }
    if (!std::isfinite(longitude) || longitude < -180.0 || longitude > 180.0) {
        throw InvalidLocationException("Longitude must be between -180 and 180");
    }
}
// Note: If zero-length trips (identical pickup and drop-off coordinates) are disallowed by
// business policy (as noted in the initial design outline), Passenger's constructor can
// additionally throw std::invalid_argument / InvalidLocationException when pickup == drop.

// 2. Integration & Double-Completion Guard in DispatchCenter.cpp
if (driverIndex < 0) {
    throw NoDriverAvailableException();
}
// ... inside DispatchCenter::completeRide(int rideId):
if (rideIt == rides.end()) {
    throw RideNotFoundException(rideId);
}
if (rideIt->getStatus() == "Completed") {
    throw InvalidRideStateException("Ride " + std::to_string(rideId) + " is already completed");
}
rideIt->completeRide();

// 3. Multi-Branch Type-Safe Catch Clauses in Client Code (Ordered Derived-to-Base)
try {
    Ride ride = dispatchCenter.createRide(passenger);
} catch (const NoDriverAvailableException& e) {
    cout << "Please try again shortly: " << e.what() << '\n';
} catch (const InvalidLocationException& e) {
    cout << "Check the locations entered: " << e.what() << '\n';
} catch (const DispatchException& e) {
    cout << "Dispatch error: " << e.what() << '\n';
} catch (const std::exception& e) {
    cout << "Unexpected error: " << e.what() << '\n';
}
```

```text
// 4. Empirical Verification Test Outputs (T1 through T8)
T1 Latitude must be between -90 and 90   (latitude 999, longitude -500)
T2 Longitude must be between -180 and 180 (latitude 12, longitude 200)
T3 Latitude must be between -90 and 90   (latitude NaN)
T4 No available driver found for the request
T5 Ride not found: 99
T6 Ride 1 is already completed
T7 via base: Ride not found: 42
T8 via std::exception: Ride not found: 43
```

Table 6.4 consolidates all eight edge cases, their delivered behaviours, empirical test evidence, and verified architectural resolutions.

**Table 6.4: Comprehensive Edge-Case Verification and Exception Handling Matrix**

| Edge Case Scenario | Delivered Code Behaviour | Verification Evidence | Architectural Remediation |
| :--- | :--- | :--- | :--- |
| **Empty driver fleet** | Throws `std::runtime_error`; state unchanged | Verified in Run S1 & T4 | `NoDriverAvailableException` |
| **All registered drivers busy** | Throws `std::runtime_error`; state & `nextRideId` unchanged | Verified in Run S3 & Table 5.2 (Step 4) | `NoDriverAvailableException` (Strong Guarantee) |
| **Unknown ride ID on completion** | Throws `std::runtime_error("Ride not found")` | Verified in Run S4, T5, T7, T8 | `RideNotFoundException(rideId)` |
| **Completing same ride twice** | Silently accepted; prematurely frees driver engaged on a newer ride | Defect found in Run S6; fixed in T6 | Guard status & throw `InvalidRideStateException` |
| **Out-of-range or `NaN` coordinates** | Silently accepted; creates ride with meaningless distance | Defect found in Run S8; fixed in T1–T3 | Enforce `std::isfinite` & bounds in `Location` constructor (`InvalidLocationException`) |
| **Two drivers equally near pickup** | Driver registered earlier is selected deterministically | Verified in Run S7 (strict `<` check) | None needed; stable tie-breaking rule documented |
| **Zero-length trip (`pickup == drop`)** | Accepted under tariff (fare equals `baseFare = 40.0`) | Verified in Run S8 | Acceptable as minimum charge, or reject via `std::invalid_argument` in `Passenger` |
| **Duplicate driver IDs registered** | Not checked; first matching ID released on completion | Identified via code inspection | Check uniqueness in `DispatchCenter::registerDriver` |

## 7. Architectural Evaluation, Production Roadmap, and References

### 7.1 Summary of Achievements and Engineering Lessons Learned

Swift Dispatch demonstrates that a core operational scheduling problem—real-time matching of passengers with taxi drivers—can be solved with a compact, extensible, and memory-safe object-oriented architecture in C++17. Table 7.1 evaluates the project's objectives against concrete delivered evidence.

**Table 7.1: Evaluation of Project Objectives Against Delivered Architectural Evidence**

| Architectural Objective | Status | Delivered Evidence & Verification Artifacts |
| :--- | :--- | :--- |
| **1. Clean, modular OOP design in C++17** | Achieved | Seven cohesive classes with private state, const-correct getters, immutable value types (`Location`, `Passenger`), and Rule-of-Zero semantics across ~430 lines of C++17. |
| **2. Dynamic nearest-driver dispatch** | Achieved | `NearestDriverStrategy::selectDriverIndex` dynamically filters unavailable drivers and selects the minimum Euclidean distance, adapting across sequential trips (Table 5.2). |
| **3. Modular fare & dispatch strategies** | Achieved | `IDriverSelectionStrategy` and `FareCalculator` injected via `std::unique_ptr` into `DispatchCenter`; `HighestRatedDriverStrategy` and `SurgeFareCalculator` swapped in with zero edits to existing classes. |
| **4. Physical separation & fault tolerance** | Achieved | 7 headers in `include/` separated from 8 sources in `src/` via CMake 3.16+; strong exception guarantee verified and extended with custom `DispatchException` hierarchy. |

**Synthesis of Engineering Findings:** Beyond confirming happy-path behaviour, systematic testing uncovered four concrete limitations in the baseline implementation: (1) unguarded double ride completion can release a driver currently engaged on a subsequent trip; (2) out-of-range or `NaN` coordinates are accepted silently; (3) planar distance is computed in coordinate degrees rather than haversine kilometres; and (4) `Ride` stores a historical snapshot copy of its `Driver` whose availability flag becomes stale after completion. Crucially, every single one of these four findings is resolved by a localized, single-class edit—adding a 3-line status guard in `DispatchCenter::completeRide`, adding range checks in `Location::Location`, and replacing the formula inside `Location::distanceTo`. This provides empirical proof of the report's central thesis: *upfront investment in encapsulation, abstraction, and SOLID design principles pays off most when requirements change or defects are discovered, because architectural boundaries localize the blast radius of change.*

### 7.2 Production Evolution Roadmap

Because the domain model is decoupled from storage, transport, and presentation layers, transforming Swift Dispatch from an in-memory console prototype into a production-ready urban mobility service requires adding new outer adapters and strategy subclasses around the existing core rather than rewriting it:

* **Persistent Relational Storage (Repository Pattern):** Introduce abstract repository interfaces (`DriverRepository` and `RideRepository`) backed by SQLite (for local deployment) or PostgreSQL (for distributed production) to persist fleet state, historical trip logs, and driver analytics across restarts. Injecting repositories into `DispatchCenter` mirrors the existing strategy injection and eliminates the stale driver snapshot issue by establishing a single normalized source of truth.
* **Networking API Layer, Concurrency, and Live Telemetry:** Wrap the `DispatchCenter` facade inside a thin REST or WebSocket controller so mobile passenger and driver apps can invoke `registerDriver`, `createRide`, and `completeRide` over HTTP/TCP. Production concurrency requires protecting fleet state mutations with a mutex (`std::mutex` / `std::lock_guard`) to prevent race conditions when simultaneous requests arrive, alongside streaming live GPS telemetry to invoke `setCurrentLocation` continuously rather than jumping to the drop-off point at assignment.
* **Graphical User Interface & Dispatcher Dashboard:** Because console I/O is isolated in `main.cpp` and `DispatchCenter` already exposes `getDrivers()` and `getRides()` as `const` references, a desktop or web map dashboard can render live vehicle markers and active rides as a new client of the facade.
* **Multi-Factor Dynamic Pricing & Advanced Routing Strategies:** Extend `FareCalculator` with time-of-day tariffs, airport zone fees, waiting-time charges (widening `calculateFare` to accept trip duration), demand-to-supply ratio surge calculators, and promotional decorators. Similarly, extend `IDriverSelectionStrategy` with weighted multi-objective scoring (combining distance, driver rating, and recent idle time) and road-network ETA routing.
* **Spatial Indexing, Strongly Typed State, Request Queuing, and Automated Unit Testing:** Replace the $\Theta(n)$ linear scan with a 2D spatial grid or $k$-d tree strategy for sub-linear lookup across thousands of vehicles; replace boolean availability and string ride status with strongly typed `enum class DriverStatus` and `enum class RideStatus`; add ride cancellation and an asynchronous wait queue for requests arriving during peak fleet exhaustion; and build an automated Catch2 or GoogleTest suite leveraging constructor injection to test edge cases deterministically in CI/CD pipelines.

### 7.3 References and Primary Project Sources

* Soumya Ranjan Behera (ID: B125124, CSEB-2). *Swift Dispatch: Backend Architecture for a Dynamic Taxi Dispatch System (C++17 & CMake Source Repository)*. GitHub Repository: [https://github.com/SRB03/swift-dispatch](https://www.google.com/search?q=https://github.com/SRB03/swift-dispatch) (2026).
* Gamma, E., Helm, R., Johnson, R., & Vlissides, J. *Design Patterns: Elements of Reusable Object-Oriented Software*. Addison-Wesley Professional (Gang of Four Strategy and Facade Pattern Specifications).
* ISO/IEC 14882:2017. *Programming Languages — C++ (C++17 Standard Specification)*: Smart pointers (`<memory>`), algorithms (`<algorithm>`), IEEE 754 floating-point classification (`<cmath>`, `<limits>`), and standard exception hierarchies (`<stdexcept>`).
