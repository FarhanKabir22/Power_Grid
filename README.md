# ⚡ Intelligent Power Grid Failure Recovery and Load Restoration System

A C++ simulation of an intelligent power grid recovery system that detects network failures, restores connectivity using available backup transmission lines, manages transmission capacity, and prioritizes critical loads during power shortages.

The project combines graph traversal, minimum-cost network restoration, maximum flow, and greedy load prioritization into a complete failure-recovery pipeline.

---

## 📌 Project Overview

Power grid failures can disconnect important consumers and reduce the amount of power that can be delivered through the network.

This project simulates a power grid containing:

- A central power plant
- Multiple substations
- Transmission lines with capacity and cost constraints
- Primary and backup transmission lines
- Load centers with different priority levels
- Transmission line failures

After a failure occurs, the system analyzes the remaining network and attempts to restore service while considering:

1. Network connectivity
2. Restoration cost
3. Transmission capacity
4. Load priority

---

## ⚙️ Recovery Pipeline

The complete recovery process follows:

```text
GRID FAILURE
     │
     ▼
BFS Connectivity Analysis
     │
     ▼
Kruskal + DSU Restoration
     │
     ▼
Maximum Flow
     │
     ▼
Greedy Priority Restoration
     │
     ▼
FINAL RESTORATION REPORT
```

---

## 🧠 Algorithms Used

### 1. BFS — Connectivity Analysis

Breadth-First Search starts from the central power plant and traverses all active, non-failed transmission lines.

It identifies:

- Connected load centers
- Isolated load centers
- Total demand lost because of disconnection

---

### 2. Kruskal Algorithm + DSU — Network Restoration

After disconnected regions are identified, the system considers available backup transmission lines.

Backup lines are sorted according to restoration cost.

A Disjoint Set Union (DSU) structure is used to determine whether activating a backup line connects two previously separate components.

This allows the system to restore connectivity while avoiding unnecessary cycles and preferring lower-cost backup connections.

DSU uses:

- Path compression
- Union by rank

---

### 3. Maximum Flow — Capacity Analysis

Restoring connectivity does not necessarily mean that every consumer can receive its required power.

Each transmission line has a limited capacity, and the power plant also has a generation limit.

The restored grid is therefore modeled as a flow network:

```text
Super Source
     │
     ▼
Power Plant
     │
     ▼
Transmission Network
     │
     ▼
Selected Load Centers
     │
     ▼
Super Sink
```

A Maximum Flow algorithm determines whether the available network capacity can fully supply the selected loads.

---

### 4. Greedy Priority-Based Load Restoration

During limited generation or transmission capacity, critical consumers should be restored before lower-priority consumers.

Loads are considered in the following order:

```text
HIGH → MEDIUM → LOW
```

For loads having the same priority, the higher-demand load is considered first.

Each load is temporarily added to the restoration set and Maximum Flow is recalculated.

- If all accepted loads can receive their full demand → the load is accepted.
- Otherwise → the load is shed.

---

## 🏙️ Default Power Grid

The simulation contains eight nodes.

| ID | Node | Type | Priority | Demand / Supply |
|---:|---|---|---|---:|
| 0 | Central Power Plant | Power Plant | N/A | 100 MW Supply |
| 1 | North Substation | Substation | N/A | — |
| 2 | Metro Substation | Substation | N/A | — |
| 3 | East Substation | Substation | N/A | — |
| 4 | General Hospital | Load Center | HIGH | 25 MW |
| 5 | Municipal Water Works | Load Center | HIGH | 20 MW |
| 6 | Metro Residential | Load Center | MEDIUM | 35 MW |
| 7 | Heavy Industrial Park | Load Center | LOW | 30 MW |

The default scenario begins with one primary transmission line in a failed state.

Additional backup transmission lines are available for network restoration.

---

## 💻 Features

The interactive simulator allows the user to:

- View all grid nodes
- View transmission lines
- View the current active topology
- Create a transmission line failure
- Repair or activate a transmission line
- Run BFS connectivity analysis
- Run Kruskal + DSU restoration
- Run Maximum Flow with Greedy restoration
- Run the complete recovery simulation
- View the final restoration report
- Change power plant generation capacity
- Reset the grid to its original scenario

---

## 🖥️ User Interface

The project uses a console-based interactive interface.

Example menu:

```text
============================================================
              INTELLIGENT POWER GRID SIMULATOR
============================================================

 1. View Nodes
 2. View Transmission Lines
 3. View Current Topology
 4. Create Line Failure
 5. Repair / Activate Line
 6. BFS Connectivity Analysis
 7. Kruskal + DSU Restoration
 8. Maximum Flow + Greedy Restoration
 9. Run Complete Simulation
10. View Final Report
11. Change Plant Capacity
12. Reset Grid
 0. Exit
```

The program performs input validation for menu choices, transmission line IDs, and plant capacity values.

---

## 📊 Final Restoration Report

After the recovery process, the program reports:

- Load priority
- Required demand
- Received power
- Restoration status
- Total system demand
- Total delivered power
- Overall delivery percentage
- Number of critical loads successfully restored

A load can be reported as:

```text
FULL
PARTIAL
SHED
```

---

## 🛠️ Technologies

- **Language:** C++
- **Standard Library:** STL
- **Data Structures:** Graph, Queue, Vector, DSU
- **Algorithms:** BFS, Kruskal, Maximum Flow, Greedy
- **Interface:** Command-line / Console

---

## ▶️ Compilation and Execution

Using a C++ compiler such as `g++`:

```bash
g++ PowerGrid_Simulator.cpp -o PowerGrid_Simulator
```

Run on Windows:

```bash
PowerGrid_Simulator.exe
```

On Linux/macOS:

```bash
./PowerGrid_Simulator
```

---

## 🧪 Example Workflow

A typical demonstration can be performed as follows:

```text
1. View Nodes
        ↓
2. View Transmission Lines
        ↓
6. Run BFS Connectivity Analysis
        ↓
7. Run Kruskal + DSU Restoration
        ↓
8. Run Maximum Flow + Greedy Restoration
        ↓
10. View Final Report
```

Alternatively, option **9 — Run Complete Simulation** automatically executes the recovery stages in sequence.

---

## 📁 Project Structure

```text
Power_Grid/
│
├── PowerGrid_Simulator.cpp
└── README.md
```

`PowerGrid_Simulator.cpp` contains the complete implementation of the simulator.

---

## 🎥 Demonstration

A recorded demonstration of the Intelligent Power Grid Failure Recovery and Load Restoration System is available on YouTube.

[Watch the project demonstration on YouTube](https://youtu.be/3e7COnLcXTs)

---

## 👥 Team Members

Add the names and IDs of the three team members here.

| Member | Student ID |
|---|---|
| Farhan Kabir | 230041207 |
| Fariha Musfirat Shifa | 230041223|
| Rakib Islam | 230041243|

---

## 🎯 Conclusion

The Intelligent Power Grid Failure Recovery and Load Restoration System demonstrates how multiple graph and optimization algorithms can work together to solve a realistic infrastructure recovery problem.

**BFS** detects connectivity problems, **Kruskal with DSU** restores disconnected regions using available backup lines, **Maximum Flow** evaluates transmission capacity, and a **Greedy strategy** prioritizes critical loads when resources are limited.

The result is an interactive simulation of:

> **Detect → Reconnect → Allocate → Prioritize → Restore ⚡**
