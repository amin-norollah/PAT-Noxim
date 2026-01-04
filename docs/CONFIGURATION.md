# PAT-Noxim Configuration Guide

## Overview

PAT-Noxim provides extensive configuration options to customize simulations. Configuration can be set through:

1. **Default Parameters** in `NoximParameters.h` (compile-time)
2. **Command-line Arguments** (runtime override)

## Default Parameters (NoximParameters.h)

### Network Topology

```cpp
// Mesh dimensions (X, Y, Z)
#define DEFAULT_MESH_DIM_X 4
#define DEFAULT_MESH_DIM_Y 4
#define DEFAULT_MESH_DIM_Z 2

// Vertical link configuration
#define DEFAULT_VERTICAL_LINK VERTICAL_MESH
// Options: VERTICAL_MESH, VERTICAL_BYPASS, VERTICAL_THROTTLE
```

**Effect**: Defines network size. Default is 4×4×2 = 32 tiles
**Range**: Tested up to 16×16×4 (memory/time limited by SystemC)
**Performance Note**: Simulation time scales roughly O(dimension³)

### Virtual Channels

```cpp
#define DEFAULT_NUM_VC 1
```

**Range**: 1-8 channels typically
**Effect**: Each VC has separate state, allocation, and buffering
**Benefit**: Deadlock prevention, reduced head-of-line blocking
**Cost**: Increased area, power, and simulation complexity

### Buffer Configuration

```cpp
#define DEFAULT_BUFFER_DEPTH 8
#define DEFAULT_MAX_PACKET_SIZE 8
#define DEFAULT_MIN_PACKET_SIZE 8
```

**Buffer Depth**: Number of flits per input buffer

- Typical: 4-8 flits
- Larger = more buffering, higher latency, more power
- Smaller = tighter deadlines, potential packet loss

**Packet Size**:

- Max: Maximum packet size (in flits)
- Min: Minimum packet size
- Typical range: 4-16 flits
- Variable sizes: Set Min ≠ Max

### Routing Algorithm

```cpp
#define DEFAULT_ROUTING_ALGORITHM ROUTING_XYZ
```

**Supported Algorithms**:

- `ROUTING_XYZ`: 3D dimensional order routing (default, minimal, deadlock-free)
- `ROUTING_WEST_FIRST`: West-first with odd-even turns
- `ROUTING_ODD_EVEN_3D`: Odd-even routing (more adaptive)
- `ROUTING_FULLY_ADAPTIVE`: Any path adaptive (requires careful VC allocation)

**Trade-offs**:

- **XYZ**: Minimal path, low latency, but prone to congestion
- **West-First**: Better load balancing, similar latency
- **Odd-Even**: Best load balancing, potentially higher latency
- **Fully Adaptive**: Best performance, but requires careful design

**Custom Routing**:

```cpp
#define DEFAULT_ROUTING_TABLE_FILENAME ""
```

Specify file with custom routes (format: src dst port)

### Routing Computation

```cpp
#define DEFAULT_ROUTING_COMPUTATION 0
// 0: Normal routing
// 1: Pre-routing (speculative RC)
```

- **Normal**: Route computed per packet
- **Pre-routing**: Route pre-computed with speculation (faster but higher power)

### Switch Allocation

```cpp
#define DEFAULT_SWITCH_ALLOCATION 0
// 0: Normal SA
// 1: Speculation SA
```

- **Normal**: Standard switch allocation
- **Speculation**: Speculative SA (reduces latency, increases power)

### Arbitration Router Architecture

```cpp
#define DEFAULT_A_ROUTER 1
#define DEFAULT_A_WITH_CREDIT 1
```

- **A_ROUTER**: Advanced router with pipelined architecture
- **A_WITH_CREDIT**: Credit-based flow control (required for multi-VC)

### Traffic Configuration

```cpp
#define DEFAULT_TRAFFIC_DISTRIBUTION TRAFFIC_RANDOM
// Options: TRAFFIC_RANDOM, TRAFFIC_BITREVERSE, TRAFFIC_TRANSPOSE,
//          TRAFFIC_HOTSPOT, TRAFFIC_TABLE_BASED

#define DEFAULT_TRAFFIC_TABLE_FILENAME "test_traffic.txt"
#define DEFAULT_PACKET_INJECTION_RATE 0.01  // 1% injection
#define DEFAULT_PROBABILITY_OF_RETRANSMISSION 0.01  // 1% retransmit
```

**Traffic Patterns**:

| Pattern             | Description                 | Use Case                 |
| ------------------- | --------------------------- | ------------------------ |
| TRAFFIC_RANDOM      | Uniform random destinations | Baseline performance     |
| TRAFFIC_BITREVERSE  | Bit-reversal permutation    | Worst-case broadcast     |
| TRAFFIC_TRANSPOSE   | Matrix transpose pattern    | Stencil computations     |
| TRAFFIC_HOTSPOT     | Concentrated to few nodes   | Memory/cache simulation  |
| TRAFFIC_TABLE_BASED | Custom trace from file      | Real workload simulation |

**Injection Rate**:

- Range: 0.0 to 1.0 (0% to 100%)
- Typical: 0.01 to 0.3
- Effect: Determines network load
- Performance impact: Higher rate = higher congestion/latency

### Selection Strategy

```cpp
#define DEFAULT_SELECTION_STRATEGY SEL_RANDOM
```

**Options**: How to select among eligible output ports

- `SEL_RANDOM`: Random selection
- `SEL_FIRST_AVAILABLE`: First available port

### Simulation Control

```cpp
#define DEFAULT_SIMULATION_TIME 10000
#define DEFAULT_STATS_WARM_UP_TIME 1
#define DEFAULT_RESET_TIME 1
```

**Simulation Time**: Total cycles to simulate (e.g., 10,000 cycles)

- Longer = more accurate statistics, slower simulation
- Typical: 1,000 to 1,000,000 cycles

**Warm-up Time**: Initial cycles ignored in statistics

- Allows transient behavior to stabilize
- Typical: 1-10% of simulation time
- Effect: Excludes startup noise from statistics

**Reset Time**: Initial clock cycles before statistics collection

- Usually same as warm-up time

### Tracing and Debugging

```cpp
#define DEFAULT_VERBOSE_MODE VERBOSE_OFF
// Options: VERBOSE_OFF, VERBOSE_DETAILED, VERBOSE_VERY_DETAILED

#define DEFAULT_TRACE_MODE false
#define DEFAULT_TRACE_FILENAME ""
#define DEFAULT_DETAILED false
```

**Verbose Modes**:

- `VERBOSE_OFF`: Minimal output
- `VERBOSE_DETAILED`: Per-packet event logging
- `VERBOSE_VERY_DETAILED`: Per-flit, per-port logging

**Note**: Increased verbosity significantly slows simulation

### Power and Thermal Model Parameters

#### Technology Node

```cpp
#define PARM_TECH_POINT 22
// Supported: 22, 32, 45, 65, 90 nm
```

**Determines**:

- Transistor characteristics
- Power scaling factors
- Leakage current models
- Available technology files

#### Voltage and Frequency

```cpp
#define PARM_Vdd 1.0          // Supply voltage (V)
#define PARM_Freq 1e9         // Clock frequency (Hz)
```

**Vdd**: Supply voltage

- 22 nm typical: 0.7-1.0 V
- 90 nm typical: 1.2-1.5 V
- Effect: Power scales with Vdd²

**Frequency**: Clock frequency

- 22 nm typical: 2.0-3.0 GHz
- 90 nm typical: 1.0-1.5 GHz
- Effect: Power scales with frequency, affects cycle time

#### Transistor Type

```cpp
#define PARM_TRANSISTOR_TYPE NVT
// Options: HVT (High Vth), NVT (Normal Vth), LVT (Low Vth)
```

**Vth Variant Effects**:

- **HVT**: Lower leakage, higher delay, lower performance
- **NVT**: Balanced leakage/performance (typical)
- **LVT**: Higher leakage, lower delay, higher performance

#### Core Model

```cpp
#define CORE_MODEL 2
// Options: ARM_A9=0, NIAGARA1=1, NIAGARA2=2, ALPHA=3
```

**Effect**: Sets PE area and power characteristics for specific processors

### Thermal Configuration

```cpp
#define DEFAULT_CAL_TEMP true
#define DEFAULT_THROTTLING_TYPE THROT_NORMAL
// Options: THROT_NORMAL, THROT_DYNAMIC

#define DEFAULT_THROTTLING_RATIO 0
// 0.0 to 1.0: Throttling intensity
```

**Thermal Management**:

- **CAL_TEMP**: Enable/disable thermal calculation
- **THROTTLING_TYPE**: Throttling strategy
  - `THROT_NORMAL`: Baseline throttling
  - `THROT_DYNAMIC`: Dynamic throttling based on temperature
- **THROTTLING_RATIO**: Injection rate reduction when throttled

#### Temperature Thresholds

```cpp
#define TEMP_THRESHOLD 80.0  // Celsius (define in code)
```

**Effect**: Temperature triggering throttling activation

#### Thermal Feedback

```cpp
#define DEFAULT_DW_LAYER_SEL DW_BL
#define DEFAULT_DOWN_LEVEL 3
```

**Effect**: Thermal model configuration for vertical layer consideration

### Other Parameters

```cpp
#define DEFAULT_VERBOSE_MODE VERBOSE_OFF
#define DEFAULT_DYAD_THRESHOLD 0.6
#define DEFAULT_CASCADE_NODE false
#define DEFAULT_BELTWAY false
#define DEFAULT_MBELTWAY false
#define DEFAULT_SBELTWAY false
#define DEFAULT_MCASCADE false
#define DEFAULT_MESSAGE_LEVEL true
```

**Advanced Features**:

- **DYAD_THRESHOLD**: Power/thermal correlation threshold
- **CASCADE_NODE**: Multi-layer cascade support
- **BELTWAY**: Beltway routing for thermal management
- **MESSAGE_LEVEL**: Message-level statistics

## Command-Line Arguments

Override default parameters at runtime:

```bash
./noxim -option1 value1 -option2 value2 ...
```

### Common Command-Line Options

#### Network Configuration

```bash
-dim_x 8              # Change X dimension to 8
-dim_y 8              # Change Y dimension to 8
-dim_z 2              # Change Z dimension to 2
-buffer_depth 16      # Change buffer depth to 16
-routing ROUTING_WEST_FIRST  # Use West-First routing
-num_vc 4             # Use 4 virtual channels
```

#### Traffic Configuration

```bash
-traffic TRAFFIC_HOTSPOT    # Use hotspot traffic
-injection_rate 0.05        # 5% injection rate
-packet_size 8              # Fixed 8-flit packets
-min_packet_size 4          # Minimum 4 flits
-max_packet_size 12         # Maximum 12 flits
-traffic_table file.txt     # Load custom traffic table
```

#### Simulation Control

```bash
-simtime 100000       # Simulate 100K cycles
-warm_up_time 1000    # 1000 cycle warm-up
-seed 12345           # Random seed
-verbose VERBOSE_DETAILED  # Detailed output
```

#### Power and Thermal

```bash
-tech 45              # Use 45 nm technology
-voltage 0.8          # Set voltage to 0.8V
-frequency 2e9        # Set frequency to 2 GHz
-cal_temp 1           # Enable thermal calculation
-throttling THROT_DYNAMIC  # Enable dynamic throttling
```

### Example Command Lines

**Small Network, High Load**

```bash
./noxim -dim_x 4 -dim_y 4 -dim_z 1 -num_vc 2 -injection_rate 0.2 -simtime 50000
```

**Large Network, Realistic Traffic**

```bash
./noxim -dim_x 16 -dim_y 16 -dim_z 2 -traffic TRAFFIC_HOTSPOT \
        -injection_rate 0.05 -num_vc 4 -simtime 100000 -warm_up_time 5000
```

**Power and Thermal Focused**

```bash
./noxim -dim_x 8 -dim_y 8 -tech 22 -voltage 0.8 -frequency 2e9 \
        -cal_temp 1 -throttling THROT_DYNAMIC -injection_rate 0.15
```

**Custom Routing and Traffic**

```bash
./noxim -routing ROUTING_FULLY_ADAPTIVE -traffic TRAFFIC_TABLE_BASED \
        -traffic_table custom_trace.txt -dim_x 4 -dim_y 4
```

## Configuration File Format

### Traffic Table Format (`test_traffic.txt`)

```
time source destination packet_size
100 0 15 8
200 3 12 8
300 7 4 8
...
```

- **time**: Cycle when traffic injection occurs
- **source**: Source tile ID
- **destination**: Destination tile ID
- **packet_size**: Packet size in flits

### Routing Table Format

```
source destination port
0 1 EAST
0 2 SOUTH
0 3 NORTH
...
```

- **source**: Source tile ID
- **destination**: Destination tile ID
- **port**: Output port (EAST, WEST, NORTH, SOUTH, UP, DOWN, LOCAL)

## Performance Tuning Recommendations

### For Accuracy

```bash
-simtime 500000       # Long simulation
-warm_up_time 10000   # Long warm-up
-num_vc 4             # Multiple VCs
-routing ROUTING_FULLY_ADAPTIVE  # Adaptive routing
```

### For Speed

```bash
-simtime 10000        # Short simulation
-warm_up_time 100     # Short warm-up
-num_vc 1             # Single VC
-routing ROUTING_XYZ  # Simple routing
```

### For Power Accuracy

```bash
-cal_temp 1           # Enable thermal feedback
-tech 22              # Specific technology
-voltage 0.8 -frequency 2e9  # Specific operating point
```

### For Thermal Analysis

```bash
-cal_temp 1           # Enable thermal
-throttling THROT_DYNAMIC  # Dynamic throttling
-injection_rate 0.2   # Higher load for heating
-simtime 100000       # Long simulation for thermal stabilization
```

## Parameter Validation

PAT-Noxim validates parameters:

- **Network size**: Must be ≥ 2×2×1
- **Buffer depth**: Must be ≥ 2
- **Virtual channels**: Must be ≥ 1, ≤ 8
- **Injection rate**: Must be 0.0 to 1.0
- **Simulation time**: Must be > 0
- **Packet size**: Min ≤ Max

## Configuration Examples

### Example 1: Small Test Network

```cpp
#define DEFAULT_MESH_DIM_X 4
#define DEFAULT_MESH_DIM_Y 4
#define DEFAULT_MESH_DIM_Z 1
#define DEFAULT_NUM_VC 1
#define DEFAULT_SIMULATION_TIME 10000
#define DEFAULT_TRAFFIC_DISTRIBUTION TRAFFIC_RANDOM
#define DEFAULT_PACKET_INJECTION_RATE 0.01
```

### Example 2: Large-Scale 3D Network

```cpp
#define DEFAULT_MESH_DIM_X 8
#define DEFAULT_MESH_DIM_Y 8
#define DEFAULT_MESH_DIM_Z 4
#define DEFAULT_NUM_VC 4
#define DEFAULT_BUFFER_DEPTH 16
#define DEFAULT_ROUTING_ALGORITHM ROUTING_FULLY_ADAPTIVE
#define DEFAULT_SIMULATION_TIME 500000
#define DEFAULT_TRAFFIC_DISTRIBUTION TRAFFIC_HOTSPOT
#define DEFAULT_PACKET_INJECTION_RATE 0.1
```

### Example 3: Power and Thermal Focused

```cpp
#define PARM_TECH_POINT 22
#define PARM_Vdd 0.8
#define PARM_Freq 2e9
#define DEFAULT_CAL_TEMP true
#define DEFAULT_THROTTLING_TYPE THROT_DYNAMIC
#define DEFAULT_TRAFFIC_DISTRIBUTION TRAFFIC_TABLE_BASED
#define DEFAULT_SIMULATION_TIME 100000
```

## Advanced Configuration

### Custom Traffic Workload

1. Create `custom_traffic.txt` with your traffic pattern
2. Run: `./noxim -traffic TRAFFIC_TABLE_BASED -traffic_table custom_traffic.txt`

### Technology Comparison

```bash
# 90nm
./noxim -tech 90 -voltage 1.2 -frequency 1.5e9 -out_90nm.txt

# 45nm
./noxim -tech 45 -voltage 1.0 -frequency 2.0e9 -out_45nm.txt

# 22nm
./noxim -tech 22 -voltage 0.8 -frequency 3.0e9 -out_22nm.txt
```

### Performance Comparison

```bash
# Baseline
./noxim -routing ROUTING_XYZ -num_vc 1 > results_baseline.txt

# Improved
./noxim -routing ROUTING_FULLY_ADAPTIVE -num_vc 4 > results_improved.txt
```

## Output Configuration

Output files are generated in current directory:

- `power_router_*`: Router power traces
- `power_mac_*`: Processor power traces
- `power_mem_*`: Memory power traces
- `throughput_*`: Throughput traces
- `stats_*`: Final statistics

Output names include timestamp for automatic differentiation.
