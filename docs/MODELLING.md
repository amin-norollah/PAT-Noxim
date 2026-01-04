# PAT-Noxim Power and Thermal Modeling Guide

## Overview

PAT-Noxim integrates two sophisticated models for accurate hardware simulation:
1. **ORION 3.0**: Power consumption and area estimation
2. **Hotspot 6.0**: Thermal simulation and temperature modeling

This guide explains both models and how they interact.

## Part 1: ORION 3.0 Power Model

### What is ORION?

ORION (On-Chip Router Area/Power Modeling) is a comprehensive power modeling framework that:
- Calculates power dissipation in NoC routers
- Estimates area of router components
- Supports multiple technology nodes (22nm to 90nm)
- Accounts for both dynamic and static (leakage) power

### Power Components Modeled

ORION breaks down router power into several components:

#### 1. Buffer Power

**Input Buffers** (read/write operations):
```
Input Buffer Power = Reads × Power_per_read + Writes × Power_per_write
```
- One input buffer per direction
- Separate power tracking for read and write
- Varies by buffer SRAM model (large/medium/small)

**Output Buffers**:
```
Output Buffer Power = Reads × Power_per_read + Writes × Power_per_write
```

**Central Buffer** (if used):
- Shared buffer for all channels
- Additional read/write power

Files handling: `ORION_array_l.c/h`, `ORION_array_m.c/h`, `ORION_cam.c/h`

#### 2. Crossbar Power

**Crossbar Switching**:
```
Crossbar Power = Number_of_transfers × Power_per_transfer
```
- Depends on crossbar width (number of output ports)
- Size increases with number of input/output ports
- Larger crossbars consume more power per switch

File handling: `ORION_crossbar.c/h`

#### 3. Arbitration Power

**Switch Allocation** (arbitration logic):
```
SA Power = Requests × Power_per_arbitration
```
- Combinational logic power for arbitration
- Increases with number of ports
- Scales with complexity of allocation algorithm

**Virtual Channel Allocation**:
```
VA Power = Requests × Power_per_VC_allocation
```
- Allocates virtual channels to new packets
- Separate arbitration per VC

Files handling: `ORION_arbiter.c/h`

#### 4. Link Power

**Interconnect Links** (routers to routers):
```
Link Power = Link_length × Bit_rate × Power_per_unit_length
```
- Power for transmitting flits between routers
- Depends on distance (Manhattan distance)
- Scales with link width and frequency

File handling: `ORION_link.c/h`

#### 5. Clock Power

**Clock Distribution**:
```
Clock Power = Proportional to frequency × load
```
- Power in clock tree
- Typically 20-30% of dynamic power

File handling: `ORION_clock.c/h`

### Dynamic vs. Static Power

**Dynamic Power**:
```
P_dynamic = Activity × Capacitance × Voltage² × Frequency
```
- Depends on switching activity
- Varies with traffic load
- Scales with Vdd²
- Most significant in high-frequency operation

**Static (Leakage) Power**:
```
P_static = Leakage_current × Voltage
```
- Depends on temperature
- Temperature effect inversion (TEI) calculated
- More significant in advanced technology nodes
- Less active at lower frequencies

### Technology Scaling

ORION supports technology-specific parameters for:

| Technology | Vdd (V) | Key Feature |
|------------|---------|------------|
| 90nm | 1.2-1.5 | Baseline, higher leakage |
| 65nm | 1.0-1.3 | Transitional |
| 45nm | 0.8-1.2 | Modern process |
| 32nm | 0.8-1.0 | Advanced |
| 22nm | 0.7-0.9 | Leading edge |

**How it works**:
```cpp
#define PARM_TECH_POINT 22  // Select technology

// ORION automatically loads:
// - ORION_technology_v2_NEW.h (for 22nm)
// - ORION_parameter.h (tech-specific parameters)
// - Transistor capacitance values
// - Leakage models
```

### Area Modeling

ORION also estimates physical area:

**Component Areas**:
- **Buffer Area**: Proportional to depth × width
- **Crossbar Area**: ~O(ports²)
- **Arbiter Area**: ~O(log(ports))
- **Clock Distribution**: Proportional to router area

```cpp
// In NoximPower.cpp
void NoximPower::calculateArea() {
    AR_buffer = calculateBufferArea();      // Input/output buffers
    AR_crossbar = calculateCrossbarArea();  // Crossbar
    AR_vcallocator = calculateVCArea();     // VC allocator
    AR_swallocator = calculateSAArea();     // Switch allocator
    AR_total = AR_buffer + AR_crossbar + 
               AR_vcallocator + AR_swallocator;
}
```

**Use**: Area is fed to Hotspot for temperature calculations

### Power Calculation Flow

```
Start of Cycle
    │
    ├─► Count buffer reads/writes
    │   └─► Update buffer power
    │
    ├─► Count crossbar transfers
    │   └─► Update crossbar power
    │
    ├─► Count arbitration requests
    │   ├─► Update SA power
    │   └─► Update VA power
    │
    ├─► Count link transmissions
    │   └─► Update link power
    │
    ├─► Accumulate dynamic power
    │   └─► P_dynamic = all components
    │
    └─► Calculate leakage power
        └─► P_leakage = f(Temperature, Tech)
            └─► Update total power
```

### Integrating ORION into PAT-Noxim

**Main Interface**: `NoximPower.cpp/h`

```cpp
class NoximPower {
    // ORION structures
    orion_router_info_t router_info;
    double total_power;
    
    // Called per cycle
    void updatePower() {
        // Count activity
        // Call ORION functions
        // Update power values
    }
};
```

**Activity Counting**:

In `NoximRouter.cpp`:
```cpp
// Count buffer reads/writes
NoximStats::power.addBufferRead();
NoximStats::power.addBufferWrite();

// Count crossbar transfers
NoximStats::power.addCrossbarTransfer();

// Count arbitration
NoximStats::power.addArbitration();
```

### Example: Calculating Router Power

For a packet processed through the router:

1. **Flit arrives** → Input buffer write
   - Power += Input_buffer_write_power

2. **Route computed** → Arbitration
   - Power += Arbitration_power

3. **Flit in crossbar** → Crossbar transfer
   - Power += Crossbar_power

4. **Flit to output** → Output buffer write
   - Power += Output_buffer_write_power

5. **Link transmission** → Interrouter link
   - Power += Link_power

**Total per flit**: ~0.5-2 mW (depending on technology/frequency)

### Power Reduction Techniques

Based on ORION model:

| Technique | Effect | Implementation |
|-----------|--------|-----------------|
| Reduce frequency | P ∝ f | Lower Freq parameter |
| Reduce voltage | P ∝ V² | Lower Vdd parameter |
| Reduce buffer depth | Area & power ↓ | `-buffer_depth 4` |
| Reduce VCs | Arbiter complexity ↓ | `-num_vc 1` |
| Optimize routing | Activity ↓ | Use better algorithm |
| Thermal throttling | Injection rate ↓ | Enable throttling |

## Part 2: Hotspot 6.0 Thermal Model

### What is Hotspot?

Hotspot is a fast, accurate thermal simulator that:
- Models heat dissipation in multi-core chips
- Computes steady-state and transient temperatures
- Supports hierarchical thermal modeling
- Integrates with power traces

### Thermal Basics

**Heat Transfer Mechanisms**:

1. **Conduction**: Heat flow through material
   ```
   Q = k × A × ΔT / d
   where: k = conductivity, A = area, d = thickness
   ```

2. **Convection**: Heat dissipation to environment
   ```
   Q = h × A × (T - T_ambient)
   where: h = convection coefficient
   ```

3. **Radiation**: Infrared emission (minor at chip temperatures)
   ```
   Q = σ × A × (T⁴ - T_ambient⁴)
   ```

**RC Network Analogy**:
- Heat flow ≈ Electrical current
- Temperature difference ≈ Voltage
- Thermal resistance R ≈ Electrical resistance
- Thermal capacitance C ≈ Electrical capacitance

### Hotspot Temperature Calculation

**Thermal Model Equation**:
```
C × dT/dt = P - G × (T - T_ambient)

where:
  C = thermal capacitance
  P = power dissipation
  G = thermal conductance (1/R)
  T = temperature
  T_ambient = ambient temperature (300K typically)
```

**Solving**:
- **Steady-state**: dT/dt = 0
  - T = T_ambient + P × R (simple proportionality)
  
- **Transient**: Solve differential equation
  - Temperature gradually stabilizes
  - Typically reaches steady-state in milliseconds

### Hotspot in PAT-Noxim

**Integration**: `NoximThermal_IF.cpp/h`

```cpp
// Each cycle:
1. Get power from routers (ORION)
2. Feed power to Hotspot
3. Hotspot computes temperature grid
4. Get temperature distribution
5. Feed back to routers for TEI leakage calculation
6. Update throttling decisions based on temperature
```

### Temperature Distribution

**Spatial Temperature Map**:

```
Hotspot creates a 2D/3D grid:
  
Layer 1 (Z=1):                Layer 2 (Z=0):
┌──────┬──────┬──────┐       ┌──────┬──────┬──────┐
│70°C  │75°C  │80°C  │       │55°C  │60°C  │65°C  │
├──────┼──────┼──────┤       ├──────┼──────┼──────┤
│72°C  │85°C  │78°C  │       │58°C  │68°C  │62°C  │
├──────┼──────┼──────┤       ├──────┼──────┼──────┤
│68°C  │80°C  │82°C  │       │54°C  │64°C  │66°C  │
└──────┴──────┴──────┘       └──────┴──────┴──────┘
```

**Hot spot**:
- Center shows 85°C (high power)
- Corners cooler due to edge heat dissipation
- Lower layer cooler (distance from heat source)

### Temperature Feedback Loop

```
Power Dissipation
    │
    └─► ORION calculates dynamic power
        │
        └─► Hotspot calculates temperature
            │
            └─► High temperature increases leakage
                │
                └─► Leakage increases power
                    │
                    └─► Back to temperature increase
                        (Thermal runaway risk)
```

**Solution**: Throttling
```
High Temperature Detected
    │
    └─► Reduce injection rate
        │
        └─► Fewer packets injected
            │
            └─► Lower power dissipation
                │
                └─► Temperature decreases
```

### Temperature-Dependent Leakage

**TEI (Temperature Effect Inversion)**:

```
I_leak = I_leak_0 × (T/T_0)^n × exp(q×Vth_coeff×(T-T_0)/(k×T×T_0))
```

Where:
- I_leak_0: Leakage at reference temperature
- T: Current temperature
- Vth_coeff: Threshold voltage coefficient
- T_0: Reference temperature (300K)

**In PAT-Noxim**:
```cpp
// In NoximPower.cpp
double leakage_temp_factor = calculateTEI(current_temperature);
leakage_power = base_leakage * leakage_temp_factor;
```

**Effect**:
- 30°C: Baseline leakage
- 50°C: ~1.5× leakage
- 80°C: ~3× leakage
- 100°C: ~6× leakage (potential runaway)

### Hotspot Configuration

Key parameters in thermal model:

```cpp
// Package size
#define PACKAGE_WIDTH 10.0   // mm
#define PACKAGE_HEIGHT 10.0  // mm

// Material properties
#define THERMAL_CONDUCTIVITY_DIE 100   // W/m/K
#define THERMAL_CONDUCTIVITY_INTERFACE 4.0
#define HEAT_SINK_COEFFICIENT 10.0     // Convection coeff

// Ambient
#define AMBIENT_TEMPERATURE 300   // Kelvin (27°C)
```

### Temperature Monitoring

**Output Tracking**:

```bash
# Temperature logged every N cycles
# Default: Every 100,000 cycles

# Output file: temp_log_*.txt
# Format: Cycle, Peak_Temp, Average_Temp, X, Y, Z
```

**Analysis**:
```bash
# Find peak temperature
grep -o "[0-9]*\.[0-9]*" temp_log_*.txt | sort -n | tail -1

# Identify hotspots
grep "Peak_Temp" temp_log_*.txt | head -20
```

### Thermal Management Techniques

#### 1. Throttling

**Dynamic Throttling**:
```
if (Temperature > TEMP_THRESHOLD) {
    injection_rate *= THROTTLING_FACTOR;  // Reduce traffic
    // Fewer packets → lower power → lower temperature
}
```

**In PAT-Noxim**:
```cpp
#define DEFAULT_THROTTLING_TYPE THROT_DYNAMIC
#define TEMP_THRESHOLD 80.0  // Celsius
```

#### 2. Vertical Heat Spreading

**3D Network Advantage**:
- Heat spreads across multiple layers
- Temperature more uniform than 2D
- Better heat dissipation

```
2D Network (Hot spot):   3D Network (Heat spreads):
       85°C                      65°C
      ┌────┐                   ┌────┐
      │Hot │                   │Warm│ Layer 1
      └────┘                   └────┘
       65°C                      50°C
      ┌────┐                   ┌────┐
      │Cool│                   │Cool│ Layer 2
      └────┘                   └────┘
```

#### 3. Routing-Aware Thermal Management

**Thermal-Aware Routing**:
- Avoid routing through hottest regions
- Distribute traffic across cooler paths
- Beltway routing for lateral heat dissipation

#### 4. Heterogeneous Voltage/Frequency

**DVFS Integration**:
```
High Temp → Lower frequency (and voltage)
           → Power ∝ f × V²
           → Significant power reduction
           → Temperature decreases
```

## Part 3: Power-Thermal Interaction

### Coupled Simulation Flow

**Per Cycle**:

```
1. Process packets through routers
   ├─► Count activity (buffers, crossbar, arbitration)
   └─► Activity metric

2. Calculate power (ORION)
   ├─► Dynamic power from activity
   ├─► Leakage power from previous temperature
   └─► Total power

3. Update temperature (Hotspot)
   ├─► Heat equation solver
   ├─► Incorporate dissipation
   ├─► Compute spatial distribution
   └─► New temperature grid

4. Temperature feedback (Thermal-aware)
   ├─► If T > threshold: activate throttling
   ├─► Update leakage power for next cycle
   └─► May trigger dynamic voltage/frequency scaling

5. Collect statistics
   ├─► Power trace
   ├─► Temperature trace
   └─► Performance metrics
```

### Key Files for Power-Thermal Integration

| File | Role |
|------|------|
| `NoximPower.cpp/h` | Power calculation interface |
| `NoximThermal_IF.cpp/h` | Hotspot interface |
| `NoximRouter.cpp` | Activity counting, throttling |
| `NoximStats.cpp/h` | Statistics collection |
| `ORION_*.c/h` | Power models |
| `Hotspot_*.c/h` | Thermal models |

### Example: Temperature Rise in High-Load Simulation

**Scenario**: 8×8 mesh, 20% injection rate, 22nm tech

**Timeline**:

| Cycle | Load | Power | Temp | Notes |
|-------|------|-------|------|-------|
| 0 | 0% | 0W | 27°C | Initial |
| 1K | 5% | 5W | 35°C | Warm-up |
| 10K | 20% | 15W | 55°C | Steady-state approaching |
| 50K | 20% | 15W | 72°C | Near equilibrium |
| 100K | 20% | 15W | 78°C | Equilibrium reached |
| 110K | 20% | 8W | 65°C | Throttling activated |
| 120K | 20% | 10W | 70°C | New equilibrium with throttling |

**Key insights**:
- Transient rise (0-50K cycles)
- Throttling prevents thermal runaway
- Final temperature depends on power dissipation

## Practical Modeling Examples

### Example 1: Power Comparison Across Technologies

**Script**:
```bash
#!/bin/bash

for tech in 90 65 45 32 22; do
    echo "Tech: ${tech}nm"
    
    ./noxim -dim_x 8 -dim_y 8 -tech $tech \
            -voltage $(calc_vdd $tech) \
            -frequency $(calc_freq $tech) \
            -injection_rate 0.1 -simtime 50000 \
            > results_${tech}nm.txt
    
    POWER=$(grep "Total Power" results_${tech}nm.txt | \
            awk '{print $NF}')
    echo "  Power: $POWER W"
done
```

**Expected Results**:
- 90nm: ~20W (baseline)
- 65nm: ~12W (improved efficiency)
- 45nm: ~8W (more efficient)
- 32nm: ~6W (power scaling benefit)
- 22nm: ~5W (improved process)

### Example 2: Throttling Effectiveness

**Scenario**: Monitor temperature with/without throttling

```bash
# Without throttling
./noxim -dim_x 8 -dim_y 8 -throttling THROT_NORMAL \
        -injection_rate 0.3 -simtime 100000 \
        > without_throttle.txt

# With dynamic throttling
./noxim -dim_x 8 -dim_y 8 -throttling THROT_DYNAMIC \
        -injection_rate 0.3 -simtime 100000 \
        > with_throttle.txt

# Compare temperatures
echo "Without throttle peak:"
grep "Peak" without_throttle.txt | tail -1

echo "With throttle peak:"
grep "Peak" with_throttle.txt | tail -1
```

### Example 3: 2D vs 3D Thermal Distribution

```bash
# 2D Network (4×4 single layer)
./noxim -dim_x 4 -dim_y 4 -dim_z 1 \
        -traffic TRAFFIC_HOTSPOT -injection_rate 0.2 \
        -cal_temp 1 -simtime 100000 > temp_2d.txt

# 3D Network (4×4×4)
./noxim -dim_x 4 -dim_y 4 -dim_z 4 \
        -traffic TRAFFIC_HOTSPOT -injection_rate 0.2 \
        -cal_temp 1 -simtime 100000 > temp_3d.txt

# Compare maximum temperatures
echo "2D max temp:"
grep -o "[0-9]*\.[0-9]*" temp_2d.txt | sort -n | tail -1

echo "3D max temp:"
grep -o "[0-9]*\.[0-9]*" temp_3d.txt | sort -n | tail -1
```

## Advanced Topics

### Extending Power Model

Add custom power components:

```cpp
// In NoximPower.h
class NoximPower {
    double custom_component_power;
    
    void calculateCustomPower() {
        // Your custom calculation
        custom_component_power = ...;
    }
};
```

### Extending Thermal Model

Integrate alternative thermal model:

```cpp
// In NoximThermal_IF.cpp
// Add interface to different thermal engine
// Maintain same output format for compatibility
```

### Multi-Objective Optimization

Power and thermal trade-offs:

```
Objective 1: Minimize power consumption
Objective 2: Minimize peak temperature
Objective 3: Maximize throughput

Constraints:
- T_peak < 90°C
- P_avg < 20W
- Latency < 50 cycles

Search space:
- Frequency (0.5-3.0 GHz)
- Voltage (0.7-1.2V)
- Injection rate (0.01-0.3)
- Routing algorithm
- VC count
```

## References

- ORION: https://www.ece.cmu.edu/~delta/tools/orion/
- Hotspot: https://www.ece.cmu.edu/~pmchen/tools/hotspot/
- PAT-Noxim Paper: Norollah et al., SOCC 2018
- Thermal Modeling Concepts: https://en.wikipedia.org/wiki/Thermal_engineering

## Summary

- **ORION** provides accurate power estimates across technology nodes
- **Hotspot** models realistic temperature distribution
- **Coupling** creates feedback loop for thermal-aware behavior
- **Throttling** prevents thermal runaway
- **Analysis** enables power-thermal co-optimization

