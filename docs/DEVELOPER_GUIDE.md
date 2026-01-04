# PAT-Noxim Developer Guide

## For Future Developers and Contributors

This guide helps developers understand how to extend and modify PAT-Noxim.

## Development Environment Setup

### Prerequisites
- C++ knowledge (SystemC framework)
- Understanding of NoC architecture
- Familiarity with makefile-based builds
- Git for version control (optional)

### Development Setup

```bash
# 1. Install dependencies
sudo apt-get install build-essential git

# 2. Install SystemC (see INSTALLATION.md)

# 3. Clone/navigate to project
cd PAT-Noxim

# 4. Build in debug mode
cd bin
make clean
make DEBUG="-g -O0"  # Disable optimization for debugging

# 5. Run with gdb for debugging
gdb ./noxim
(gdb) run -dim_x 4 -dim_y 4
```

## Code Organization

### Directory Structure
```
PAT_Noxim/
├── bin/          # Build system and compiled binary
├── src/          # All source code
│   ├── Noxim*.cpp/h      # Simulator core
│   ├── ORION_*.c/h       # Power model
│   └── Hotspot_*.c/h     # Thermal model
└── other/        # Optional tools
    ├── apsra2noxim.cpp
    ├── mapping2cg.cpp
    └── noxim_explorer.cpp
```

### Code Style
- **Naming**: CamelCase for classes, snake_case for functions
- **Comments**: English, docstring for public methods
- **Indentation**: 4 spaces (not tabs)
- **Line Length**: Keep under 100 characters when possible

## Common Development Tasks

### Task 1: Add a New Routing Algorithm

**Files to modify**:
1. `src/NoximRouter.cpp` - Implement routing logic
2. `src/NoximParameters.h` - Add routing constant
3. `src/NoximCmdLineParser.cpp` - Add command-line option

**Step-by-step**:

```cpp
// Step 1: Define routing type in NoximParameters.h
#define ROUTING_MY_CUSTOM 99

// Step 2: Implement routing in NoximRouter::routingComputation()
void NoximRouter::routingComputation() {
    switch (NoximGlobalParams::routing_algorithm) {
        // ... existing cases ...
        
        case ROUTING_MY_CUSTOM:
            // Your custom routing logic
            for (int i = 0; i < DIRECTIONS; i++) {
                // Determine if port i is valid output
                route[i] = isValidOutput(i, flit.dst_id);
            }
            break;
    }
}

// Step 3: Add command-line parsing in NoximCmdLineParser.cpp
if (argv[i] == string("-routing")) {
    i++;
    if (argv[i] == string("ROUTING_MY_CUSTOM"))
        NoximGlobalParams::routing_algorithm = ROUTING_MY_CUSTOM;
}
```

### Task 2: Add Custom Traffic Pattern

**Files to modify**:
1. `src/NoximProcessingElement.cpp` - Add pattern generation
2. `src/NoximParameters.h` - Add traffic type constant
3. `src/NoximCmdLineParser.cpp` - Add command-line option

**Example: Diagonal traffic pattern**

```cpp
// In NoximParameters.h
#define TRAFFIC_DIAGONAL 99

// In NoximProcessingElement.cpp
int NoximProcessingElement::selectRandomDestination() {
    int selected = false;
    NoximCoord dest;
    
    switch(NoximGlobalParams::traffic_distribution) {
        // ... existing cases ...
        
        case TRAFFIC_DIAGONAL: {
            // Create diagonal communication pattern
            int x = local_id % NoximGlobalParams::mesh_dim_x;
            int y = (local_id / NoximGlobalParams::mesh_dim_x) 
                    % NoximGlobalParams::mesh_dim_y;
            int z = local_id / (NoximGlobalParams::mesh_dim_x * 
                                NoximGlobalParams::mesh_dim_y);
            
            // Diagonal offset
            int new_x = (x + 1) % NoximGlobalParams::mesh_dim_x;
            int new_y = (y + 1) % NoximGlobalParams::mesh_dim_y;
            int new_z = z;  // Same layer
            
            selected = new_x * NoximGlobalParams::mesh_dim_y * 
                      NoximGlobalParams::mesh_dim_z +
                      new_y * NoximGlobalParams::mesh_dim_z + new_z;
            break;
        }
    }
    return selected;
}
```

### Task 3: Modify Buffer Architecture

**Files to modify**:
1. `src/NoximBuffer.cpp/h` - Buffer implementation
2. `src/NoximRouter.cpp` - Buffer usage

**Example: Add buffer statistics**

```cpp
// In NoximBuffer.h
class NoximBuffer {
private:
    // ... existing members ...
    int max_occupancy;
    long total_occupancy;
    int access_count;

public:
    double getAverageOccupancy() {
        return (double)total_occupancy / access_count;
    }
};

// In NoximBuffer.cpp implementation
void NoximBuffer::write(const NoximFlit& flit) {
    queue.push(flit);
    int current = queue.size();
    if (current > max_occupancy) max_occupancy = current;
    total_occupancy += current;
    access_count++;
}
```

### Task 4: Add New Statistics

**Files to modify**:
1. `src/NoximStats.cpp/h` - Per-tile statistics
2. `src/NoximGlobalStats.cpp/h` - Global aggregation
3. `src/NoximRouter.cpp` or `src/NoximProcessingElement.cpp` - Data collection

**Example: Track hop count distribution**

```cpp
// In NoximStats.h
class NoximStats {
    // ... existing members ...
    vector<int> hop_count_histogram;
    
public:
    void recordHopCount(int hops) {
        if (hops >= hop_count_histogram.size())
            hop_count_histogram.resize(hops + 1, 0);
        hop_count_histogram[hops]++;
    }
};

// In NoximRouter.cpp, when flit arrives
stats.recordHopCount(flit.hops);
```

### Task 5: Extend Power Model

**Files to modify**:
1. `src/NoximPower.cpp/h` - Power calculation
2. ORION model files as needed

**Example: Add cache-based power**

```cpp
// In NoximPower.h
class NoximPower {
    // ... existing members ...
    double cache_power;
    
public:
    double getCachePower() { return cache_power; }
};

// In NoximPower.cpp
void NoximPower::updatePower(...) {
    // ... existing power calculations ...
    
    // Add cache power calculation
    cache_power = calculateCachePower();
}
```

## Debugging Guide

### Enable Verbose Output

```cpp
// In NoximRouter.cpp
if (NoximGlobalParams::verbose_mode == VERBOSE_DETAILED) {
    cout << "Router " << local_id << " at cycle " 
         << sc_time_stamp() << ": Packet " 
         << flit.sequence_number << " routed to port " 
         << port << endl;
}
```

### Add Debug Files

```cpp
// Create debug output file
ofstream debug_log("debug.txt");

// Log events
debug_log << "Cycle " << sc_time_stamp() 
         << ": Event description" << endl;
```

### Use GDB Debugger

```bash
# Start debugger
gdb ./noxim

# Set breakpoint
(gdb) break NoximRouter::routingComputation

# Run simulation
(gdb) run -dim_x 4 -dim_y 4 -simtime 1000

# When breakpoint hits
(gdb) print flit.dst_id
(gdb) print route[0]
(gdb) step
```

### Add Assertions

```cpp
// Verify assumptions
assert(buffer_depth > 0);
assert(flit.packet_id < max_packets);

// With message
assert(num_vc > 0 && "Virtual channels must be > 0");
```

## Performance Optimization

### Profiling

```bash
# Generate profile information
make DEBUG="-pg"
./noxim -dim_x 4 -dim_y 4
gprof ./noxim gmon.out > profile.txt

# Analyze bottlenecks
less profile.txt
```

### Common Bottlenecks

1. **Buffer operations**: Too many write/read calls
   - Solution: Batch operations where possible

2. **Routing computation**: Complex route calculations
   - Solution: Pre-compute routes for deterministic routing

3. **Statistics collection**: Per-packet tracking
   - Solution: Only track essential statistics

4. **Signal updates**: Too many SystemC signals
   - Solution: Use efficient signal binding

### Optimization Techniques

```cpp
// Before: Inefficient per-packet stats
for (int i = 0; i < num_packets; i++) {
    stats.recordLatency(packets[i].latency);  // Slow
}

// After: Batch stats recording
double total_latency = 0;
for (int i = 0; i < num_packets; i++) {
    total_latency += packets[i].latency;
}
stats.recordLatencies(total_latency, num_packets);  // Fast
```

## Adding New Features

### Feature Checklist

When adding a new feature:

- [ ] Define parameters in `NoximParameters.h`
- [ ] Add command-line parsing in `NoximCmdLineParser.cpp`
- [ ] Implement core logic in appropriate module
- [ ] Add statistics collection in `NoximStats.cpp`
- [ ] Update documentation in `docs/`
- [ ] Test with small network
- [ ] Test with large network
- [ ] Verify no performance regression
- [ ] Add comments explaining the feature

### Version Control Workflow

```bash
# Create feature branch
git checkout -b feature/my-new-feature

# Make changes
git add src/NoximXXX.cpp
git commit -m "Add my new feature"

# Merge back
git checkout main
git merge feature/my-new-feature
```

## Testing Your Changes

### Unit Testing Approach

```cpp
// Create test program
int testNewFeature() {
    // Test 1: Basic functionality
    NoximRouter router;
    assert(router.isInitialized());
    
    // Test 2: Edge cases
    assert(router.handleEmptyBuffer());
    
    // Test 3: Large data
    for (int i = 0; i < 10000; i++) {
        assert(router.processPacket(packets[i]));
    }
    
    return 0;  // Success
}
```

### Integration Testing

```bash
# Test 1: Small network
./noxim -dim_x 4 -dim_y 4 -simtime 10000

# Test 2: New feature enabled
./noxim -dim_x 4 -dim_y 4 -new_feature 1 -simtime 10000

# Test 3: Compare outputs
diff results_before.txt results_after.txt
```

### Regression Testing

Create test suite script:

```bash
#!/bin/bash

TESTS=(
    "4:4:1:ROUTING_XYZ"
    "8:8:1:ROUTING_WEST_FIRST"
    "4:4:2:ROUTING_ODD_EVEN_3D"
)

for test in "${TESTS[@]}"; do
    IFS=':' read -r X Y Z ROUTING <<< "$test"
    ./noxim -dim_x $X -dim_y $Y -dim_z $Z \
            -routing $ROUTING -simtime 50000
    # Check exit code
    if [ $? -ne 0 ]; then
        echo "FAILED: $test"
        exit 1
    fi
done

echo "All tests passed"
```

## Building and Compiling

### Build Variants

```bash
# Standard release build
cd bin
make

# Debug build (with symbols)
make DEBUG="-g -O0"

# Optimized build
make OPT="-O3"

# Clean build
make clean
make

# Parallel build (faster on multi-core)
make -j4
```

### Makefile Structure

Understanding `Makefile`:

```makefile
TARGET_ARCH = linux          # Target architecture
CC = g++                     # Compiler
OPT = -O3                    # Optimization level
VPATH = ../src:.            # Source paths
MODULE = noxim              # Output filename
SRCS = *.cpp                # C++ source files
SRCS_C = *.c                # C source files
```

## Documentation

### Commenting Code

```cpp
// Bad: No comment
route[0] = checkNorth();

// Good: Clear comment
// Check if north neighbor is valid output port for this destination
route[0] = checkNorth(flit.dst_id);

// Better: Full documentation
/**
 * Computes routing decisions for an incoming flit.
 * @param flit - The incoming flit to route
 * @return Boolean array indicating valid output ports
 * @note Must be called once per flit per cycle
 */
void routingComputation(const NoximFlit& flit);
```

### Updating Documentation

When adding features, update:
- `docs/FILE_GUIDE.md` - Describe modified files
- `docs/ARCHITECTURE.md` - Explain design changes
- `docs/CONFIGURATION.md` - Document new parameters
- Code comments - Inline documentation

## Memory Management

### Best Practices

```cpp
// Avoid dynamic allocation in hot paths
// Bad: Allocates memory every cycle
vector<int>* routes = new vector<int>();

// Good: Pre-allocated buffers
vector<int> routes;  // Allocated once, reused

// Bad: Unbounded memory growth
queue<Packet> packet_queue;  // Could grow indefinitely

// Good: Bounded memory
queue<Packet> packet_queue;  // With size checks
assert(packet_queue.size() < MAX_QUEUE_SIZE);
```

### Performance Tips

```cpp
// Use references to avoid copies
void processPacket(const NoximPacket& pkt);  // Good
void processPacket(NoximPacket pkt);         // Bad

// Pre-allocate vectors
vector<int> route;
route.reserve(DIRECTIONS);

// Use const where possible
const NoximFlit& readFlit() const;
```

## Continuous Integration

### Automated Testing

Create `.github/workflows/build.yml` for GitHub Actions:

```yaml
name: Build and Test

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: sudo apt-get install g++ make
      - name: Build
        run: cd bin && make
      - name: Run tests
        run: ./bin/noxim -dim_x 4 -dim_y 4 -simtime 1000
```

## Getting Help

### Debugging Checklist

If something doesn't work:

1. **Check compilation errors**: Fix syntax first
2. **Verify parameters**: Print configuration at start
3. **Check log files**: Look for error messages
4. **Enable verbose mode**: `-verbose VERBOSE_DETAILED`
5. **Use debugger**: `gdb ./noxim`
6. **Compare with baseline**: Test unmodified code first
7. **Check bounds**: Ensure array access is valid
8. **Memory check**: Use `valgrind` for memory leaks

### Common Issues

**Issue**: Undefined reference to symbol
```
Solution: Check include files and Makefile SRCS
make clean
make
```

**Issue**: Simulation hangs
```
Solution: Check for deadlock in routing
Enable verbose output to see packet progression
Reduce simtime to find where it hangs
```

**Issue**: Incorrect results
```
Solution: Add debug output
Verify algorithm implementation against reference
Check boundary conditions
```

## Contributing Guidelines

1. **Code Quality**: Follow existing style
2. **Testing**: Test thoroughly before submission
3. **Documentation**: Document your changes
4. **Comments**: Clear, concise comments
5. **No Breaking Changes**: Ensure backward compatibility
6. **Performance**: Benchmark changes

## References

- **SystemC**: https://systemc.readthedocs.io/
- **C++ Best Practices**: https://github.com/isocpp/CppCoreGuidelines
- **Noxim Original**: https://github.com/davidepatti/noxim

## Contact & Support

For questions or issues:
- Email: a.norollah.official@gmail.com
- Review existing documentation
- Check GitHub issues if available


