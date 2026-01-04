# PAT-Noxim File Guide

Comprehensive guide to each source file in PAT-Noxim, organized by component category.

## Core Noxim Simulator Files

### Main Entry Point

#### `NoximMain.cpp` / `NoximMain.h`

- **Purpose**: Top-level simulator entry point
- **Key Functions**:
  - `sc_main()`: SystemC simulation kernel entry
  - Global parameter initialization
  - Command-line argument parsing coordination
  - Log file setup
- **Global Variables**:
  - `NoximGlobalParams`: Static configuration parameters
  - Output streams for various log files
- **When to Use**: Understanding overall simulation flow
- **For Developers**: Start here to trace complete simulation execution

### Network Controller

#### `NoximNoC.cpp` / `NoximNoC.h`

- **Purpose**: Top-level NoC (Network-on-Chip) module
- **Responsibilities**:
  - Instantiates 3D mesh of tiles (X × Y × Z dimension)
  - Creates routing and traffic tables
  - Manages global network signals
  - Coordinates inter-tile communication
  - Produces final statistics output
- **Key Classes**: `SC_MODULE(NoximNoC)`
- **Key Methods**:
  - `end_of_simulation()`: Generates final reports
  - Signal binding for all routers and PEs
- **Signal Management**:
  - Flit transmission signals (east/west/south/north/up/down)
  - Request/acknowledge signals for flow control
  - Free slot availability signals
- **When to Use**: Understanding network topology and signal flow
- **For Developers**: Modify here to change network dimensions or topology

### Router Implementation

#### `NoximRouter.cpp` / `NoximRouter.h`

- **Purpose**: Router module implementing packet switching and routing
- **Architecture Features**:
  - Configurable input/output ports
  - Multi-stage pipeline (3, 4, or 5-stage selectable)
  - Virtual channel support (1-N channels)
  - Credit-based flow control
- **Key Components**:
  - **Route Computation**: Determines packet destination port
  - **Virtual Channel Allocation**: Assigns available VC
  - **Switch Allocation**: Arbitrates crossbar access
  - **Crossbar Switching**: Connects input to output ports
- **Routing Algorithms Supported**:
  - `ROUTING_XYZ`: Default 3D minimal routing
  - `ROUTING_WEST_FIRST`: West-first deadlock avoidance
  - `ROUTING_ODD_EVEN_3D`: Odd-even routing
  - `ROUTING_FULLY_ADAPTIVE`: Any path adaptive routing
- **I/O Ports**:
  - `flit_rx[DIRECTIONS+2]`: Input flit streams
  - `flit_tx[DIRECTIONS+2]`: Output flit streams
  - `req_rx/tx`: Request signals per VC
  - `ack_rx/tx`: Acknowledge signals per VC
  - Flow control: `free_slots`, `free_slots_neighbor`
  - Thermal signals: `on_off`, `TB`, `PDT`
- **Key Methods**:
  - `routing()`: Route computation process
  - `vc_allocation()`: Virtual channel assignment
  - `switch_allocation()`: Switch arbitration
  - `power_and_area()`: Power calculation trigger
- **When to Use**: Modifying routing algorithms, router pipeline, or arbitration policies
- **For Developers**: This is where most routing algorithm modifications occur

### Processing Element

#### `NoximProcessingElement.cpp` / `NoximProcessingElement.h`

- **Purpose**: Traffic generator and sink (simulates computational element)
- **Responsibilities**:
  - Generate packets with configurable injection rate
  - Receive and process incoming packets
  - Manage message queues
  - Track per-VC communication
- **Traffic Generation Modes**:
  - `TRAFFIC_RANDOM`: Uniform random destinations
  - `TRAFFIC_BITREVERSE`: Bit-reversal permutation
  - `TRAFFIC_TRANSPOSE`: Transpose communication pattern
  - `TRAFFIC_HOTSPOT`: Concentrated traffic to hotspot nodes
  - `TRAFFIC_TABLE_BASED`: Custom patterns from file
- **Key Data Structures**:
  - `message_queue`: High-level message queue
  - `packet_queue`: Per-VC packet queues
  - `temp_p[DEFAULT_NUM_VC]`: Temporary packet storage
- **Virtual Channel Management**:
  - Per-VC credit tracking
  - Per-VC packet queueing
  - Round-robin VC selection for transmission
- **Key Methods**:
  - `genPacket()`: Packet generation process
  - `consume()`: Packet reception process
  - `bufferUpdate()`: Credit management
- **Configuration Parameters**:
  - `packet_injection_rate`: Packet generation probability
  - `min/max_packet_size`: Packet size range
  - `probability_of_retransmission`: Failed transmission retry
- **When to Use**: Modifying traffic patterns, injection rates, or packet handling
- **For Developers**: Add custom traffic generation patterns here

### Buffer Management

#### `NoximBuffer.cpp` / `NoximBuffer.h`

- **Purpose**: FIFO buffer for router input/output ports
- **Features**:
  - Configurable depth (typically 4-8 flits)
  - Per-virtual-channel FIFO queues
  - Read/write operations with availability checking
- **Key Methods**:
  - `write()`: Insert flit into buffer
  - `read()`: Extract flit from buffer
  - `isFull()`: Check buffer status
  - `isEmpty()`: Check if empty
- **Operations Per Cycle**:
  - Can read and write in same cycle (dual-port behavior)
  - Maintains separate FIFO for each virtual channel
- **When to Use**: Adjusting buffer architecture or capacity
- **For Developers**: Understanding flow control mechanisms

## Routing and Table Management

#### `NoximGlobalRoutingTable.cpp` / `NoximGlobalRoutingTable.h`

- **Purpose**: Pre-computed routing tables for entire network
- **Use Cases**:
  - Deterministic routing
  - Custom routing specifications
  - Network verification
- **Data Structure**: `[source][destination] = port`
- **Key Methods**:
  - `getRoute()`: Retrieve pre-computed route
  - `load()`: Load routes from file
- **File Format**: Text file with source, destination, port triples
- **When to Use**: Setting up custom network routes
- **For Developers**: Understanding static routing mechanisms

#### `NoximLocalRoutingTable.cpp` / `NoximLocalRoutingTable.h`

- **Purpose**: Per-router routing decision logic
- **Responsibilities**:
  - Local routing decisions
  - Adaptive routing based on congestion
  - Alternative path computation
- **Integration**: Used by NoximRouter during route computation
- **When to Use**: Implementing new adaptive routing strategies
- **For Developers**: Extending routing algorithm capabilities

#### `NoximGlobalTrafficTable.cpp` / `NoximGlobalTrafficTable.h`

- **Purpose**: Custom traffic trace management
- **Features**:
  - Store pre-defined communication patterns
  - Time-based traffic injection
  - Deterministic workload specification
- **File Format**: Time, source, destination, packet_size
- **Key Methods**:
  - `load()`: Read traffic table from file
  - `isCommunicationAvailable()`: Check if traffic active
  - `getDestination()`: Get traffic destination
- **When to Use**: Running trace-based simulations
- **For Developers**: Understanding workload injection

#### `NoximReservationTable.cpp` / `NoximReservationTable.h`

- **Purpose**: Track packet resources per virtual channel
- **Responsibilities**:
  - Manage buffer reservation
  - Track VC availability
  - Prevent deadlock through resource tracking
- **Usage**: Per-PE and per-Router basis
- **When to Use**: Understanding credit-based flow control
- **For Developers**: Modifying credit management policies

## Statistics and Monitoring

#### `NoximStats.cpp` / `NoximStats.h`

- **Purpose**: Per-tile statistics collection
- **Collected Metrics**:
  - **Latency**: Average, maximum, per-source
  - **Throughput**: Flits/cycles, per-source
  - **Packet Counts**: Received packets/flits
  - **Delay Distribution**: Histogram of delays
  - **Per-VC Statistics**: Separate counters for each VC
- **Key Methods**:
  - `receivedFlit()`: Record flit arrival
  - `getAverageDelay()`: Compute average delay
  - `getAverageThroughput()`: Compute throughput
  - `showStats()`: Print statistics
- **Warm-up Period**: Configurable to skip transient behavior
- **When to Use**: Adding custom per-tile metrics
- **For Developers**: Extending performance measurement capabilities

#### `NoximGlobalStats.cpp` / `NoximGlobalStats.h`

- **Purpose**: Network-wide statistics aggregation
- **Responsibilities**:
  - Collect statistics from all tiles
  - Compute aggregate metrics
  - Coordinate power and thermal measurements
  - Generate final reports
- **Metrics**:
  - Total network power
  - Peak temperature
  - Average latency across network
  - Network throughput
- **Key Methods**:
  - `updatePowerStats()`: Update power calculations
  - `updateThermalStats()`: Update thermal measurements
  - `showStats()`: Print global statistics
- **Integration Points**:
  - Receives data from NoximStats (per-tile)
  - Coordinates ORION power calculations
  - Coordinates Hotspot thermal calculations
- **When to Use**: Understanding network-wide performance
- **For Developers**: Modifying global metric collection

### Virtual Channel State Machine

#### `NoximVCState.cpp` / `NoximVCState.h`

- **Purpose**: Implement virtual channel state machine
- **States**:
  - **IDLE**: VC not in use
  - **ACTIVE**: VC has packet being routed
  - **WAIT_FOR_EXIT**: VC packet at output port
  - **ALLOCATED**: VC allocated to input packet
- **Transitions**: Based on routing decisions and packet status
- **Deadlock Prevention**: VC separation prevents circular wait
- **Key Methods**:
  - `setState()`: Change VC state
  - `getState()`: Query current state
  - `canAllocate()`: Check if allocatable
- **When to Use**: Understanding deadlock avoidance
- **For Developers**: Modifying VC state machine (rarely needed)

## 3D Network Support

#### `NoximVLink.cpp` / `NoximVLink.h`

- **Purpose**: Vertical link implementation for 3D mesh
- **Features**:
  - Connects layers in Z-dimension
  - Supports full 3D mesh topology
  - Throttling-based flow control
  - Bypass link support
- **Modes**:
  - `VERTICAL_MESH`: Full 3D mesh connectivity
  - `VERTICAL_BYPASS`: Bypass intermediate layers
  - `VERTICAL_THROTTLE`: Thermal-based gating
- **I/O Signals**: Similar to horizontal links
- **When to Use**: Working with 3D network topologies
- **For Developers**: Extending vertical link capabilities

## Tile Definition

#### `NoximTile.h`

- **Purpose**: Tile module definition (no .cpp file - pure declaration)
- **Composition**:
  - 1 NoximRouter instance
  - 1 NoximProcessingElement instance
  - Connection logic between router and PE
- **Hierarchy**: Each tile is instantiated for every (x,y,z) position
- **When to Use**: Understanding tile structure
- **For Developers**: Modifying tile internal organization

## Command-Line Interface

#### `NoximCmdLineParser.cpp` / `NoximCmdLineParser.h`

- **Purpose**: Parse and apply command-line arguments
- **Supported Options**:
  - Network dimensions: `-dim_x`, `-dim_y`, `-dim_z`
  - Traffic: `-traffic`, `-injection_rate`, `-packet_size`
  - Routing: `-routing`, `-routing_table`
  - Simulation: `-simtime`, `-seed`, `-detailed`
  - Power/Thermal: `-tech`, `-voltage`, `-frequency`
  - And many more...
- **Usage**: `./noxim -option1 value1 -option2 value2 ...`
- **Key Function**: `parseCmdLine(arg_count, arg_vector)`
- **When to Use**: Understanding available command-line options
- **For Developers**: Adding new simulation parameters

#### `NoximParameters.h`

- **Purpose**: Define all default simulation parameters
- **Content Type**: #define preprocessor directives
- **Categories**:
  - Network topology defaults
  - Traffic parameters
  - Routing configuration
  - Power model settings
  - Thermal model settings
  - Statistics collection options
- **Technology Nodes**: Support for 22, 32, 45, 65, 90 nm
- **When to Use**: Understanding default configuration
- **For Developers**: Changing default behavior

#### `NoximPEParam.h`

- **Purpose**: Processing element specific parameters
- **Content**: PE-related configuration and constants
- **Usage**: Referenced by NoximProcessingElement and power models
- **When to Use**: Configuring PE behavior
- **For Developers**: Extending PE capabilities

## Logging and Output

#### `NoximLog.cpp` / `NoximLog.h`

- **Purpose**: Initialize output log files and streams
- **Log Files Generated**:
  - `results_log_pwr_router`: Router power measurements
  - `results_log_pwr_mac`: Processor power measurements
  - `results_log_pwr_mem`: Memory power measurements
  - `transient_log_throughput`: Network throughput over time
  - `static_log_power`: Static power consumption
  - `temp_tei_power`: Temperature-based leakage power
- **Key Methods**:
  - `openFiles()`: Open all log files
  - `closeFiles()`: Finalize and close
- **When to Use**: Configuring output logging
- **For Developers**: Adding new log files or metrics

## Power Modeling (ORION Integration)

#### `NoximPower.cpp` / `NoximPower.h`

- **Purpose**: Main power calculation interface
- **Responsibilities**:
  - Calculate router power consumption
  - Calculate area of router components
  - Manage leakage power (TEI method)
  - Generate power reports
- **Power Components Calculated**:
  - Input buffer R/W power
  - Output buffer R/W power
  - Central buffer R/W power
  - Crossbar power
  - Switch allocation power
  - VC allocation power
  - Clock distribution power
  - Link power
- **Area Components**:
  - Buffer area
  - Crossbar area
  - VA allocator area
  - SA allocator area
- **Key Methods**:
  - `calculateArea()`: Compute static area
  - `updatePower()`: Calculate dynamic power
  - `clearPower()`: Reset power counters
  - `getTotalArea()`: Get total router area
  - `getTotalPower()`: Get total power dissipation
- **Technology Support**: Easily switch between 22-90 nm nodes
- **Temperature Integration**: Leakage updates based on Hotspot temperature
- **When to Use**: Understanding power calculations
- **For Developers**: Extending power model or adding new components

#### `NoximThermal_IF.cpp` / `NoximThermal_IF.h`

- **Purpose**: Interface between simulator and Hotspot thermal model
- **Responsibilities**:
  - Coordinate with Hotspot thermally
  - Pass temperature data to routers
  - Implement thermal feedback
  - Handle throttling decisions
- **Key Functions**:
  - Temperature grid update
  - Hotspot communication
  - Thermal-aware throttling
- **When to Use**: Modifying thermal feedback
- **For Developers**: Integrating alternative thermal models

## ORION Power Model Files

All ORION files are in `src/ORION_*.c/h` format. These are C files integrated with C++ wrappers.

### Router Power Calculation

- **`ORION_router_power.c/h`**: Main router power computation
- **`ORION_router_area.c/h`**: Router area calculation
- **`ORION_router.c/h`**: Router model structure

### Component Models

- **`ORION_crossbar.c/h`**: Crossbar switching power
- **`ORION_arbiter.c/h`**: Arbitration logic power
- **`ORION_link.c/h`**: Inter-router link power
- **`ORION_clock.c/h`**: Clock distribution power

### Array Models

- **`ORION_array_l.c/h`**: Large SRAM array model
- **`ORION_array_m.c/h`**: Medium SRAM array model
- **`ORION_cam.c/h`**: Content Addressable Memory

### Computational Units

- **`ORION_ALU.c/h`**: ALU power model
- **`ORION_permu.c/h`**: Permutation network model

### Technology and Utilities

- **`ORION_technology_*.h`**: Technology node parameters (v1, v2, v2_NEW)
- **`ORION_parameter.h`**: ORION configuration parameters
- **`ORION_util.c/h`**: Utility functions
- **`ORION_time.c/h`**: Timing calculations
- **`ORION_misc.c/h`**: Miscellaneous functions

## Hotspot Thermal Model Files

All Hotspot files are in `src/Hotspot_*.c/h` format.

### Core Thermal Simulation

- **`Hotspot_temperature.c/h`**: Main thermal computation engine
- **`Hotspot_temperature_grid.c/h`**: Spatial temperature distribution
- **`Hotspot_temperature_block.c/h`**: Block-level thermal modeling

### Support Models

- **`Hotspot_flp.c/h`**: Floorplan management (component placement)
- **`Hotspot_package.c/h`**: Package and substrate modeling
- **`Hotspot_shape.c/h`**: Heat dissipation shape models
- **`Hotspot_npe.c/h`**: Non-processing element modeling

### Utilities

- **`Hotspot_util.c/h`**: Utility functions
- **`Hotspot_RCutil.c`**: RC network calculations for thermal resistance/capacitance

## Utility and Tool Files

### Optional Tools (in `other/` directory)

#### `apsra2noxim.cpp`

- **Purpose**: Convert APSRA output to Noxim format
- **Use Case**: Integrate APSRA application mapping
- **Build**: `make` in other/ directory

#### `mapping2cg.cpp`

- **Purpose**: Convert communication traces to mapped formats
- **Use Case**: Prepare custom workloads
- **Build**: `make` in other/ directory

#### `noxim_explorer.cpp`

- **Purpose**: Design space exploration tool
- **Use Case**: Explore configuration parameters
- **Build**: `make` in other/ directory

#### Configuration Files

- **`config_*.cfg`**: Example configuration files
- **`MyTimer.h`**: Timing utility header

## File Dependencies Summary

```
NoximMain.cpp
├── NoximNoC.h
│   ├── NoximTile.h
│   │   ├── NoximRouter.h
│   │   │   ├── NoximBuffer.h
│   │   │   ├── NoximVCState.h
│   │   │   ├── NoximStats.h
│   │   │   └── Routing Tables
│   │   └── NoximProcessingElement.h
│   │       ├── NoximBuffer.h
│   │       └── NoximVCState.h
│   ├── NoximVLink.h
│   └── Routing/Traffic Tables
├── NoximGlobalStats.h
├── NoximPower.h (ORION integration)
├── NoximThermal_IF.h (Hotspot integration)
└── NoximCmdLineParser.h
```

## Quick Reference for Common Tasks

| Task                       | Primary File                 | Secondary Files                                    |
| -------------------------- | ---------------------------- | -------------------------------------------------- |
| Add new routing algorithm  | `NoximRouter.cpp`            | `NoximParameters.h`, `NoximCmdLineParser.cpp`      |
| Change network dimensions  | `NoximParameters.h`          | `NoximNoC.h`, `NoximCmdLineParser.cpp`             |
| Add traffic pattern        | `NoximProcessingElement.cpp` | `NoximParameters.h`, `NoximGlobalTrafficTable.cpp` |
| Modify buffer behavior     | `NoximBuffer.cpp`            | `NoximRouter.cpp`                                  |
| Add new statistics         | `NoximStats.cpp`             | `NoximGlobalStats.cpp`                             |
| Change power model         | `NoximPower.cpp`             | `ORION_*.c` files                                  |
| Adjust thermal model       | `NoximThermal_IF.cpp`        | `Hotspot_*.c` files                                |
| Add command-line parameter | `NoximCmdLineParser.cpp`     | `NoximParameters.h`, `NoximMain.h`                 |
| Change VC behavior         | `NoximVCState.cpp`           | `NoximRouter.cpp`                                  |
| Modify PE behavior         | `NoximProcessingElement.cpp` | `NoximReservationTable.cpp`                        |
