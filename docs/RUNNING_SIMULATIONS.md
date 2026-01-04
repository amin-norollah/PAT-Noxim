# PAT-Noxim Running Simulations Guide

## Quick Start

### Minimum Setup

```bash
cd PAT-Noxim/bin
./noxim
```

This runs with default parameters (4×4×2 mesh, random traffic, 10K cycles).

### View Help

```bash
./noxim -h
```

Shows all available command-line options.

## Basic Simulation Examples

### Example 1: Simple 4×4 Network

**Command**:

```bash
./noxim -dim_x 4 -dim_y 4 -dim_z 1 -simtime 10000
```

**What it does**:

- Creates a 4×4 2D mesh network (16 tiles)
- Runs for 10,000 clock cycles
- Uses default random traffic at 1% injection rate
- Measures latency, throughput, power consumption

**Expected output**:

- Console displays simulation progress
- Final statistics printed to console
- Log files generated with power traces

**Typical duration**: 5-10 seconds

### Example 2: 8×8 Network with Hotspot Traffic

**Command**:

```bash
./noxim -dim_x 8 -dim_y 8 -traffic TRAFFIC_HOTSPOT -injection_rate 0.1 -simtime 50000
```

**What it does**:

- Creates 8×8 2D network (64 tiles)
- Hotspot traffic (concentrated to few nodes, like memory/cache)
- 10% injection rate (higher than default)
- Runs longer (50K cycles)

**Expected output**:

- Shows network performance under concentrated load
- High congestion in hotspot areas
- Temperature peaks in hotspot regions

**Typical duration**: 30-60 seconds

### Example 3: 3D Network with Multiple Virtual Channels

**Command**:

```bash
./noxim -dim_x 4 -dim_y 4 -dim_z 4 -num_vc 4 -buffer_depth 16 -simtime 50000
```

**What it does**:

- Creates 4×4×4 3D mesh (64 tiles)
- 4 virtual channels (better deadlock prevention)
- Larger buffers (16 flits per input)
- Extended simulation for thorough evaluation

**Expected output**:

- Shows 3D network performance
- Lower latency due to multiple VCs
- Higher area/power due to VC overhead

**Typical duration**: 60-120 seconds

### Example 4: Different Routing Algorithms

**Compare routing algorithms**:

```bash
# XYZ routing (default, minimal path)
./noxim -dim_x 8 -dim_y 8 -routing ROUTING_XYZ \
        -injection_rate 0.15 -simtime 50000 > results_xyz.txt

# West-First routing (better load balancing)
./noxim -dim_x 8 -dim_y 8 -routing ROUTING_WEST_FIRST \
        -injection_rate 0.15 -simtime 50000 > results_wf.txt

# Odd-Even routing (even better adaptation)
./noxim -dim_x 8 -dim_y 8 -routing ROUTING_ODD_EVEN_3D \
        -injection_rate 0.15 -simtime 50000 > results_oe.txt

# Fully Adaptive routing (best performance)
./noxim -dim_x 8 -dim_y 8 -routing ROUTING_FULLY_ADAPTIVE \
        -injection_rate 0.15 -simtime 50000 > results_fa.txt
```

Then compare results in output files.

### Example 5: Power and Thermal Analysis

**Command**:

```bash
./noxim -dim_x 4 -dim_y 4 -dim_z 2 \
        -tech 22 -voltage 0.8 -frequency 2e9 \
        -cal_temp 1 -throttling THROT_DYNAMIC \
        -injection_rate 0.2 -simtime 100000
```

**What it does**:

- 4×4×2 3D network
- Advanced technology node (22nm)
- Enables thermal calculation
- Dynamic throttling based on temperature
- Higher injection rate causes higher power/temperature

**Output includes**:

- Power consumption traces
- Temperature distribution over time
- Throttling events
- Impact of thermal feedback

**Typical duration**: 120-300 seconds

## Advanced Scenarios

### Scenario 1: Design Space Exploration

Explore network parameters systematically:

```bash
#!/bin/bash

# Create results directory
mkdir -p results

# Vary injection rate
for rate in 0.01 0.05 0.1 0.15 0.2; do
    echo "Running with injection rate: $rate"
    ./noxim -dim_x 8 -dim_y 8 -injection_rate $rate \
            -simtime 50000 > "results/rate_$rate.txt"
done

# Vary buffer depth
for depth in 4 8 16 32; do
    echo "Running with buffer depth: $depth"
    ./noxim -dim_x 8 -dim_y 8 -buffer_depth $depth \
            -simtime 50000 > "results/buffer_$depth.txt"
done

# Vary number of VCs
for vc in 1 2 4 8; do
    echo "Running with $vc virtual channels"
    ./noxim -dim_x 8 -dim_y 8 -num_vc $vc \
            -simtime 50000 > "results/vc_$vc.txt"
done
```

### Scenario 2: Performance Under Different Traffic Patterns

**Script**:

```bash
#!/bin/bash

patterns=(TRAFFIC_RANDOM TRAFFIC_BITREVERSE TRAFFIC_TRANSPOSE TRAFFIC_HOTSPOT)

for pattern in "${patterns[@]}"; do
    echo "Testing $pattern traffic"
    ./noxim -dim_x 8 -dim_y 8 -traffic $pattern \
            -injection_rate 0.1 -simtime 50000 \
            > "results/traffic_$pattern.txt"
done
```

**Interpretation**:

- Random: Baseline uniform distribution
- BitReverse: Stress test (worst-case broadcast)
- Transpose: Stencil computation pattern
- Hotspot: Memory/cache contention

### Scenario 3: Technology Comparison

Compare performance across technology nodes:

```bash
#!/bin/bash

# Test across technology nodes
for tech in 90 65 45 32 22; do
    echo "Running 22nm simulation"
    ./noxim -dim_x 8 -dim_y 8 -tech $tech \
            -injection_rate 0.1 -simtime 50000 \
            -cal_temp 1 > "results/tech_${tech}nm.txt"
done
```

### Scenario 4: Trace-Based Simulation

Create custom traffic trace:

**Create `custom_trace.txt`**:

```
time source destination packet_size
1000 0 63 8
1001 1 62 8
1002 2 61 8
2000 0 31 8
2001 1 30 8
...
```

**Run simulation**:

```bash
./noxim -dim_x 8 -dim_y 8 \
        -traffic TRAFFIC_TABLE_BASED \
        -traffic_table custom_trace.txt \
        -simtime 10000
```

## Output Interpretation

### Console Output

Typical console output shows:

```
Noxim - the NoC Simulator
...
[Configuration Summary]
Network Dimensions: 4 x 4 x 2
Virtual Channels: 1
Buffer Depth: 8
Routing Algorithm: XYZ
Traffic Pattern: Random
Injection Rate: 0.01
...
[Simulation Progress]
Cycle: 1000...
Cycle: 2000...
...
Cycle: 10000

[Simulation Complete]
Average Latency: 5.23 cycles
Average Throughput: 0.009 flits/cycle
...
```

### Log Files

Generated output files in current directory:

**Power Files**:

- `power_router_*.txt`: Router power traces per cycle
- `power_mac_*.txt`: Processor power traces
- `power_mem_*.txt`: Memory power traces

**Performance Files**:

- `throughput_*.txt`: Network throughput over time
- `stats_*.txt`: Per-node statistics

**Format**:

```
Cycle,TotalPower(W),DynamicPower(W),LeakagePower(W)
0,1.234,0.850,0.384
1,1.245,0.860,0.385
...
```

### Interpreting Results

**Latency**:

- Low (2-5 cycles): Good routing, light load
- Medium (5-15 cycles): Moderate congestion
- High (>20 cycles): Heavy congestion or inefficient routing

**Throughput**:

- Injection rate ≈ achieved throughput: No saturation
- Achieved < injection rate: Network saturated

**Power**:

- Dynamic power dominates at high injection
- Leakage becomes significant at lower frequencies
- Thermal throttling reduces power at cost of throughput

**Temperature**:

- Hotspot traffic shows 10-20°C peaks vs corners
- 3D networks show vertical temperature gradients
- Throttling limits peak temperature

## Batch Simulation Scripts

### Linux/macOS Script

Create `run_batch.sh`:

```bash
#!/bin/bash

OUTPUT_DIR="simulation_results"
mkdir -p $OUTPUT_DIR

echo "=== PAT-Noxim Batch Simulation ==="
echo "Output directory: $OUTPUT_DIR"

# Simulation configurations
CONFIGS=(
    "4:4:1:0.01:ROUTING_XYZ:TRAFFIC_RANDOM"
    "8:8:1:0.05:ROUTING_XYZ:TRAFFIC_HOTSPOT"
    "8:8:2:0.1:ROUTING_WEST_FIRST:TRAFFIC_RANDOM"
    "16:16:1:0.05:ROUTING_ODD_EVEN_3D:TRAFFIC_BITREVERSE"
)

for config in "${CONFIGS[@]}"; do
    IFS=':' read -r DIM_X DIM_Y DIM_Z RATE ROUTING TRAFFIC <<< "$config"

    NAME="${DIM_X}x${DIM_Y}x${DIM_Z}_${RATE}_${ROUTING}"

    echo "Running: $NAME"

    ./noxim \
        -dim_x $DIM_X \
        -dim_y $DIM_Y \
        -dim_z $DIM_Z \
        -injection_rate $RATE \
        -routing $ROUTING \
        -traffic $TRAFFIC \
        -simtime 50000 \
        > "$OUTPUT_DIR/${NAME}.txt"

    echo "  Completed: $NAME"
done

echo "=== All simulations complete ==="
```

Run with:

```bash
bash run_batch.sh
```

### Windows Batch Script

Create `run_batch.bat`:

```batch
@echo off
setlocal enabledelayedexpansion

set OUTPUT_DIR=simulation_results
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

echo === PAT-Noxim Batch Simulation ===

REM Configuration: DIM_X DIM_Y DIM_Z RATE ROUTING TRAFFIC

for %%C in (
    "4:4:1:0.01:ROUTING_XYZ:TRAFFIC_RANDOM"
    "8:8:1:0.05:ROUTING_XYZ:TRAFFIC_HOTSPOT"
) do (
    for /f "tokens=1-6 delims=:" %%A in ("%%C") do (
        set DIM_X=%%A
        set DIM_Y=%%B
        set DIM_Z=%%C
        set RATE=%%D
        set ROUTING=%%E
        set TRAFFIC=%%F

        echo Running: !DIM_X!x!DIM_Y!x!DIM_Z!

        noxim -dim_x !DIM_X! -dim_y !DIM_Y! -dim_z !DIM_Z! ^
              -injection_rate !RATE! -routing !ROUTING! ^
              -traffic !TRAFFIC! -simtime 50000 ^
              > %OUTPUT_DIR%\results_!DIM_X!x!DIM_Y!x!DIM_Z!.txt
    )
)

echo === All simulations complete ===
```

## Performance Monitoring

### During Simulation

Monitor process resources:

**Linux**:

```bash
# In another terminal
watch -n 1 'ps aux | grep noxim'
```

**Windows**:

```bash
tasklist | findstr noxim
```

### After Simulation

Analyze results:

```bash
# Find peak power
grep -o "[0-9]*\.[0-9]*" power_router_*.txt | sort -n | tail -1

# Count iterations
wc -l power_router_*.txt

# Average latency from stats
grep "Average Latency" stats_*.txt
```

## Troubleshooting Simulations

### Issue: Simulation Takes Too Long

**Solutions**:

- Reduce simulation time: `-simtime 10000` (from 50000)
- Reduce network size: `-dim_x 4 -dim_y 4` (from 8 8)
- Reduce virtual channels: `-num_vc 1` (from 4)
- Disable thermal: `-cal_temp 0`

### Issue: Out of Memory

**Solutions**:

- Reduce network size
- Reduce virtual channels
- Reduce buffer depth: `-buffer_depth 4`

### Issue: Unusual Results

**Debug steps**:

1. Enable verbose output: `-verbose VERBOSE_DETAILED`
2. Check configuration printed at start
3. Verify command-line arguments
4. Try with small network first
5. Check log files for errors

### Issue: No Output Files Generated

**Check**:

- Current working directory has write permissions
- Simulation completed successfully
- Look in current directory, not bin/ directory

## Advanced Usage

### Custom Configuration Compilation

Modify `NoximParameters.h` and recompile:

```bash
cd PAT-Noxim/bin
nano ../src/NoximParameters.h  # Edit defaults
make clean
make
./noxim  # Uses new defaults
```

### Linking SystemC Manually

If compilation fails:

```bash
export SYSTEMC_HOME=/usr/local/systemc-2.2.0
export LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux:$LD_LIBRARY_PATH
cd PAT-Noxim/bin
make
```

### Profiling Simulations

Use profiling tools to identify bottlenecks:

```bash
# Linux
gprof ./noxim > profile.txt

# Or with time measurement
time ./noxim -dim_x 4 -dim_y 4 -simtime 10000
```

## Output Analysis Example

Python script to analyze results:

```python
import re
import statistics

def parse_latency_file(filename):
    latencies = []
    with open(filename, 'r') as f:
        for line in f:
            if 'Average Latency' in line:
                match = re.search(r'[\d.]+', line)
                if match:
                    latencies.append(float(match.group()))
    return latencies

def parse_power_file(filename):
    powers = []
    with open(filename, 'r') as f:
        for line in f:
            if line.startswith('Cycle'):
                continue
            parts = line.strip().split(',')
            if len(parts) >= 2:
                powers.append(float(parts[1]))
    return powers

# Analyze results
latencies = parse_latency_file('stats_results.txt')
powers = parse_power_file('power_router_results.txt')

print(f"Average Latency: {statistics.mean(latencies):.2f} cycles")
print(f"Latency Std Dev: {statistics.stdev(latencies):.2f} cycles")
print(f"Average Power: {statistics.mean(powers):.3f} W")
print(f"Peak Power: {max(powers):.3f} W")
```

## Next Steps

After running simulations:

1. Analyze output files and understand results
2. Review ARCHITECTURE.md for system insights
3. Modify parameters based on findings
4. Run comparative studies
5. Review DEVELOPER_GUIDE.md for extensions
