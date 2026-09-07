# SwiftDispatch

**Backend Architecture for a Dynamic Taxi Dispatch System**

## Overview
SwiftDispatch is a robust backend architecture designed to manage dynamic taxi dispatching operations. Developed as a core Object-Oriented Programming (OOP) project, this system focuses on applying foundational OOP principles—such as Encapsulation, Abstraction, and Design Patterns (e.g., Strategy Pattern)—to solve real-world problems in real-time routing and user-driver matching.

## Core Objectives
- Implement a clean, modular object-oriented design in C++.
- Develop a dynamic dispatch algorithm to match passengers with the nearest drivers.
- Modularize fare calculation and dispatch strategies.
- Organize code cleanly by separating headers (`include/`) and implementation (`src/`).

## Tech Stack
- **Language:** C++
- **Build System:** CMake
- **Key Concepts:** Object-Oriented Design, Strategy Pattern, Separation of Concerns

## Project Roadmap

This section tracks the ongoing development progress. Commits will be pushed regularly as these phases are completed.

### Phase 1: Architecture & Setup
- [x] Initialize repository and project directory structure (`src`, `include`, `docs`).
- [ ] Configure `CMakeLists.txt` for the build system.

### Phase 2: Core Entities
- [x] Implement `Location` class for grid/coordinate mapping.
- [x] Implement `Passenger` and `Driver` classes to store user states.
- [ ] Create `Ride` class to handle individual trip data.

### Phase 3: Dispatch & Business Logic
- [ ] Implement `FareCalculator` to determine ride costs based on distance/time.
- [ ] Implement `NearestDriverStrategy` (applying the Strategy Design Pattern).
- [ ] Build the `DispatchCenter` to manage active entities and incoming requests.

### Phase 4: Integration & Execution
- [ ] Integrate components within `main.cpp` to simulate end-to-end workflows.
- [ ] Add basic exception handling and edge case management (e.g., no available drivers).

### Phase 5: Refinement
- [ ] Refactor code for optimization and strict OOP compliance.
- [ ] Add inline comments for complex logic blocks.

## Getting Started

### Prerequisites
- C++17 (or C++11/14) compatible compiler (GCC, Clang, or MSVC)
- CMake (version 3.10 or higher)

### How to Build and Run
*(Detailed instructions will be added as the codebase is initialized.)*
- [Stay Tuned]
<!-- 1. Clone the repository and navigate into the project directory.
2. Create a build directory and navigate into it:
   ```bash
   mkdir build && cd build
   ```
3. Generate the build files using CMake:
   ```bash
   cmake ..
   ```
4. Compile the project:
   ```bash
   make
   ```
5. Run the executable:
   ```bash
   ./taxi_dispatch
   ``` -->
