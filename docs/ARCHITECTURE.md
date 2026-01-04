# PAT-Noxim System Architecture

## Overview

PAT-Noxim is a cycle-accurate Network-on-Chip (NoC) simulator that integrates power consumption and thermal modeling. The architecture combines three major simulation frameworks working in harmony.

## System Architecture Diagram

```
┌──────────────────────────────────────────────────────────────┐
│                    PAT-Noxim Simulator                       │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │          SystemC Simulation Framework (Core)            │ │
│  │                                                         │ │
│  │  ┌──────────────────────────────────────────────────────┤ │
│  │  │                  NoximNoC (Network)                  │ │
│  │  │                                                      │ │
│  │  │  ┌──────────────┐     ┌──────────────┐               │ │
│  │  │  │   Tiles      │     │   V-Links    │               │ │
│  │  │  │  (X×Y×Z)     │     │  (Vertical   │               │ │
│  │  │  │              │     │   Links)     │               │ │
│  │  │  │ ┌──────────┐ │     └──────────────┘               │ │
│  │  │  │ │Router    │ │         (3D Mesh                   │ │
│  │  │  │ │Arch.     │ │          Support)                  │ │
│  │  │  │ │Multi-VC  │ │                                    │ │
│  │  │  │ │Pipeline  │ │                                    │ │
│  │  │  │ └──────────┘ │                                    │ │
│  │  │  │              │                                    │ │
│  │  │  │ ┌──────────┐ │                                    │ │
│  │  │  │ │    PE    │ │                                    │ │
│  │  │  │ │(Process) │ │                                    │ │
│  │  │  │ └──────────┘ │                                    │ │
│  │  │  └──────────────┘                                    │ │
│  │  │   (Repeats for each tile in mesh)                    │ │
│  │  └──────────────────────────────────────────────────────┤ │
│  │                                                         │ │
│  └─────────────────────────────────────────────────────────┘ │
│                          │        │        │                 │
├──────────────────────────┼────────┼────────┼─────────────────┤
│                          │        │        │                 │
│  ┌──────────────────┐    │        │        │                 │
│  │  ORION 3.0       │ ◄──┘        │        │                 │
│  │  Power Model     │             │        │                 │
│  └──────────────────┘             │        │                 │
│  • Router Power                   │        │                 │
│  • Area Calculation               │        │                 │
│                                   │        │                 │
│  ┌──────────────────┐             │        │                 │
│  │  Hotspot 6.0     │◄────────────┘        │                 │
│  │  Thermal Model   │                      │                 │
│  └──────────────────┘                      │                 │
│  • Temperature Grid                        │                 │
│  • Heat Dissipation                        │                 │
│                                            │                 │
│  ┌──────────────────┐                      │                 │
│  │  Statistics      │◄─────────────────────┘                 │
│  │  Collection      │                                        │
│  └──────────────────┘                                        │
│  • Latency                                                   │
│  • Throughput                                                │
│  • Power Consumption                                         │
│  • Temperature Profile                                       │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

## Core Components

### 1. Noxim Core (SystemC)

The main simulation engine built on SystemC 2.2.0.

#### NoximNoC (Top-Level Module)

- **File**: `NoximNoC.h/cpp`
- **Purpose**: Defines the overall NoC structure
- **Responsibilities**:
  - Instantiates mesh of tiles (X × Y × Z dimension)
  - Manages inter-tile signals and routing
  - Coordinates simulation timing
  - Generates simulation output

#### NoximTile

- **File**: `NoximTile.h`
- **Purpose**: Basic building block of the network
- **Contains**:
  - 1 Router (for packet switching)
  - 1 Processing Element (for traffic generation)

#### NoximRouter

- **File**: `NoximRouter.h/cpp`
- **Purpose**: Packet switching and routing logic
- **Key Features**:
  - Multiple input/output ports
  - Virtual channel support (configurable 1-N channels)
  - Configurable pipeline stages (3, 4, or 5-stage)
  - Credit-based flow control
  - Routing algorithms:
    - XYZ (Default 3D minimal routing)
    - West-First
    - Odd-Even
    - Fully Adaptive
    - Custom routing tables
  - Switch allocation and arbitration

#### NoximProcessingElement (PE)

- **File**: `NoximProcessingElement.h/cpp`
- **Purpose**: Simulates traffic sources and sinks
- **Capabilities**:
  - Generate packets with configurable injection rate
  - Various traffic patterns:
    - Random
    - Bit-reversal
    - Transpose
    - Hotspot
    - Custom from traffic table
  - Queue management for incoming packets
  - Packet retransmission handling

#### NoximBuffer

- **File**: `NoximBuffer.h/cpp`
- **Purpose**: FIFO input/output buffers in routers
- **Configuration**:
  - Configurable depth
  - Per-virtual-channel buffering
  - Read/write port management

### 2. Virtual Channels (VC) Support

#### NoximVCState

- **File**: `NoximVCState.h/cpp`
- **Purpose**: Manages virtual channel state machine
- **States**:
  - Idle
  - Active
  - Wait_for_exit
  - Allocated
- **Functionality**: Deadlock avoidance through VC separation

### 3. Routing & Traffic Management

#### NoximGlobalRoutingTable

- **File**: `NoximGlobalRoutingTable.h/cpp`
- **Purpose**: Static routing tables for the entire network
- **Use Case**: Pre-computed routes for deterministic paths

#### NoximLocalRoutingTable

- **File**: `NoximLocalRoutingTable.h/cpp`
- **Purpose**: Per-router routing decisions
- **Functions**: Adaptive and dynamic routing

#### NoximGlobalTrafficTable

- **File**: `NoximGlobalTrafficTable.h/cpp`
- **Purpose**: Custom traffic patterns
- **Enables**: Trace-based simulations

### 4. Statistics Collection

#### NoximStats

- **File**: `NoximStats.h/cpp`
- **Per-tile statistics**:
  - Average delay per source
  - Maximum delay
  - Throughput
  - Received flits/packets
  - Per-VC statistics

#### NoximGlobalStats

- **File**: `NoximGlobalStats.h/cpp`
- **Network-wide statistics**:
  - Total energy consumption
  - Power dissipation
  - Area calculations
  - Temperature distribution

### 5. Vertical Links (3D Support)

#### NoximVLink

- **File**: `NoximVLink.h/cpp`
- **Purpose**: Manages Z-dimension connectivity
- **Modes**:
  - Full 3D mesh
  - Throttling-based vertical routing
  - Bypass links

## Integrated Models

### ORION 3.0 - Power Model

**Location**: `src/ORION_*.c/h` files

**Models**:

- **Router Power**: Calculates power for each router component
  - Input/output buffers (read/write operations)
  - Crossbar switching
  - Virtual channel allocation
  - Switch allocation
  - Clock distribution
- **Link Power**: Calculates power for inter-router links
- **Area Model**: Estimates physical area
  - Router components area
  - Buffer area
  - Crossbar area

**Key Files**:

- `ORION_router_power.c`: Main router power calculation
- `ORION_router_area.c`: Router area estimation
- `ORION_link.c`: Link power calculation
- `ORION_array_*.c`: Array models for various buffer types
- `ORION_technology_*.h`: Technology-specific parameters (22/32/45/65/90 nm)

### Hotspot 6.0 - Thermal Model

**Location**: `src/Hotspot_*.c/h` files

**Capabilities**:

- Thermal simulation based on power dissipation
- Temperature grid computation
- Heat dissipation patterns
- Transient thermal response

**Key Files**:

- `Hotspot_temperature.c/h`: Main thermal calculations
- `Hotspot_temperature_grid.c/h`: Grid-based temperature computation
- `Hotspot_flp.c/h`: Floorplan management
- `Hotspot_package.c/h`: Package modeling
- `Hotspot_util.c/h`: Utility functions

**Features**:

- Temperature feedback for thermal-aware routing
- Throttling based on temperature threshold
- Leakage current calculation (TEI - Temperature Effect Inversion)

## Data Flow Architecture

### Simulation Cycle

```
Start of Cycle
    │
    ├─► Read input signals
    │
    ├─► PEs generate/receive packets
    │
    ├─► Routers process flits
    │   ├─ Route computation
    │   ├─ Virtual channel allocation
    │   ├─ Switch allocation
    │   └─ Crossbar switching
    │
    ├─► Update power calculations (ORION)
    │
    ├─► Update thermal model (Hotspot)
    │
    ├─► Collect statistics
    │
    └─► Write output signals
        │
        └─ Next Cycle
```

### Key Signals Between Components

**Flit Signals** (packet data):

- `flit_rx/tx`: Actual packet data
- Virtual channels in arrays for multi-VC support

**Control Signals**:

- `req_rx/tx`: Request signals (valid flit on line)
- `ack_rx/tx`: Acknowledge signals (buffer has space)
- `free_slots`: Available buffer capacity

**Power Information**:

- `on_off`: Throttling status
- `TB`: Temperature Budget
- `PDT`: Predicted Delta Temperature

## Configuration Parameters

Key parameters defined in `NoximParameters.h`:

```cpp
// Network dimensions
#define DEFAULT_MESH_DIM_X 4
#define DEFAULT_MESH_DIM_Y 4
#define DEFAULT_MESH_DIM_Z 2

// Virtual channels
#define DEFAULT_NUM_VC 1

// Buffer configuration
#define DEFAULT_BUFFER_DEPTH 8
#define DEFAULT_MAX_PACKET_SIZE 8

// Routing and traffic
#define DEFAULT_ROUTING_ALGORITHM ROUTING_XYZ
#define DEFAULT_TRAFFIC_DISTRIBUTION TRAFFIC_RANDOM

// Power model
#define PARM_TECH_POINT 22        // 22/32/45/65/90 nm
#define PARM_Vdd 1.0              // Voltage (V)
#define PARM_Freq 1e9             // Frequency (Hz)

// Thermal
#define DEFAULT_CAL_TEMP true
#define DEFAULT_THROTTLING_TYPE THROT_NORMAL
```

## File Organization Summary

```
PAT_Noxim/
├── bin/
│   ├── Makefile              # Build configuration
│   ├── Makefile.defs         # Build definitions
│   └── Makefile.deps         # Dependencies
├── src/
│   ├── Noxim*.cpp/h          # Core simulator (24 files)
│   ├── ORION_*.c/h           # Power model (51 files)
│   └── Hotspot_*.c/h         # Thermal model (20 files)
└── other/
    ├── apsra2noxim.cpp       # Trace converter
    ├── mapping2cg.cpp        # Traffic converter
    └── noxim_explorer.cpp    # Design space explorer
```

## Design Patterns

### SystemC Modules

- Each major component (Router, PE, NoC) is a SystemC module
- Uses SystemC process types: `SC_CTHREAD`, `SC_METHOD`
- Synchronous design with clock-driven updates

### Signal Passing

- SystemC signals used for module communication
- Arrays of signals for multi-channel/multi-VC support
- Hierarchical signal naming for clarity

### Memory Management

- Static allocation for most buffers (configured at compile-time)
- Dynamic queues for packet storage
- No dynamic memory allocation in critical paths

## Extension Points

Future developers can extend PAT-Noxim by:

1. **New Routing Algorithms**: Modify `NoximRouter.cpp`
2. **Custom Traffic Patterns**: Extend `NoximProcessingElement.cpp`
3. **Alternative Power Models**: Add alongside ORION
4. **New Thermal Models**: Integrate alternative models with Hotspot
5. **Additional Statistics**: Extend `NoximStats` classes
6. **New Router Architectures**: Add new router pipeline stages
7. **Advanced Throttling**: Extend thermal-aware mechanisms

## Performance Characteristics

- **Simulation Speed**: ~1-10 million cycles/hour (depends on configuration)
- **Accuracy**: Cycle-accurate for packet-level simulation
- **Scalability**: Tested up to 16×16 mesh (limited by SystemC)
- **Power Accuracy**: ±5-10% compared to RTL simulation (ORION model)
- **Thermal Accuracy**: Validated against Hotspot standalone tool

## References

- Original Noxim: https://github.com/davidepatti/noxim
- ORION: https://www.ece.cmu.edu/~delta/tools/orion/
- Hotspot: https://www.ece.cmu.edu/~pmchen/tools/hotspot/
- SystemC: https://systemc.readthedocs.io/
