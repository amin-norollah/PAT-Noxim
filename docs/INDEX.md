# PAT-Noxim Documentation Index

## Quick Navigation

Welcome to PAT-Noxim comprehensive documentation! This file serves as a navigation guide to all available documentation.

## Getting Started

**New to PAT-Noxim?** Start here:

1. **[README.md](../README.md)** - Project overview and citation
2. **[INSTALLATION.md](INSTALLATION.md)** - Set up PAT-Noxim on your system
3. **[RUNNING_SIMULATIONS.md](RUNNING_SIMULATIONS.md)** - Run your first simulation

## Complete Documentation

### User Guides (For Simulation Users)

| Document | Purpose | Best For |
|----------|---------|----------|
| **[INSTALLATION.md](INSTALLATION.md)** | Step-by-step setup guide | First-time users, troubleshooting |
| **[CONFIGURATION.md](CONFIGURATION.md)** | Parameter reference | Understanding simulation options |
| **[RUNNING_SIMULATIONS.md](RUNNING_SIMULATIONS.md)** | Simulation examples & scripts | Running and analyzing simulations |

### Reference Guides (For Understanding the System)

| Document | Purpose | Best For |
|----------|---------|----------|
| **[ARCHITECTURE.md](ARCHITECTURE.md)** | System design and components | Understanding how PAT-Noxim works |
| **[FILE_GUIDE.md](FILE_GUIDE.md)** | Description of every source file | Navigating the codebase |
| **[MODELLING.md](MODELLING.md)** | Power and thermal models | Understanding ORION and Hotspot |

### Developer Guides (For Code Modification)

| Document | Purpose | Best For |
|----------|---------|----------|
| **[DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)** | Extending and modifying PAT-Noxim | Adding features, debugging code |

## Documentation by Task

### I want to...

**...install PAT-Noxim**
→ See [INSTALLATION.md](INSTALLATION.md)
- System requirements
- Step-by-step installation
- Troubleshooting common issues

**...run a simulation**
→ See [RUNNING_SIMULATIONS.md](RUNNING_SIMULATIONS.md)
- Basic examples
- Advanced scenarios
- Batch scripting
- Result interpretation

**...configure parameters**
→ See [CONFIGURATION.md](CONFIGURATION.md)
- Default parameters reference
- Command-line options
- Configuration examples
- Performance tuning tips

**...understand the architecture**
→ See [ARCHITECTURE.md](ARCHITECTURE.md)
- System components overview
- Data flow
- Module interactions
- Design patterns

**...find a specific source file**
→ See [FILE_GUIDE.md](FILE_GUIDE.md)
- File-by-file descriptions
- Usage examples
- Dependencies
- When to modify each file

**...understand power/thermal modeling**
→ See [MODELLING.md](MODELLING.md)
- ORION power model details
- Hotspot thermal model details
- Power-thermal coupling
- Advanced topics

**...modify or extend PAT-Noxim**
→ See [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)
- Development setup
- Common modification tasks
- Debugging techniques
- Testing procedures
- Performance optimization

## File Structure

```
PAT-Noxim/
├── README.md                    # Main project readme
├── docs/                        # Documentation folder
│   ├── INDEX.md                 # This file
│   ├── INSTALLATION.md          # Setup and installation
│   ├── ARCHITECTURE.md          # System architecture
│   ├── FILE_GUIDE.md            # Source file descriptions
│   ├── CONFIGURATION.md         # Configuration parameters
│   ├── RUNNING_SIMULATIONS.md   # Simulation examples
│   ├── DEVELOPER_GUIDE.md       # Developer documentation
│   └── MODELLING.md             # Power/thermal modeling
├── PAT_Noxim/
│   ├── bin/                     # Build system
│   ├── src/                     # Source code
│   └── other/                   # Utility tools
└── pat-noxim.png               # Project logo
```

## Quick Reference

### Installation Quick Start
```bash
# 1. Install SystemC 2.2.0
# 2. Set environment variables
export SYSTEMC_HOME=/usr/local/systemc-2.2.0
export LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux:$LD_LIBRARY_PATH

# 3. Build PAT-Noxim
cd PAT-Noxim/bin
make

# 4. Run simulator
./noxim -dim_x 4 -dim_y 4 -simtime 10000
```

### Common Commands

```bash
# Get help
./noxim -h

# Simple 4×4 network
./noxim -dim_x 4 -dim_y 4 -simtime 10000

# 8×8 with hotspot traffic
./noxim -dim_x 8 -dim_y 8 -traffic TRAFFIC_HOTSPOT \
        -injection_rate 0.1 -simtime 50000

# 3D with thermal modeling
./noxim -dim_x 4 -dim_y 4 -dim_z 4 -cal_temp 1 \
        -throttling THROT_DYNAMIC -simtime 100000

# Power analysis at 22nm
./noxim -dim_x 8 -dim_y 8 -tech 22 -voltage 0.8 \
        -frequency 2e9 -simtime 50000
```

### Key Parameters

| Parameter | Purpose | Typical Range |
|-----------|---------|---------------|
| `-dim_x`, `-dim_y`, `-dim_z` | Network dimensions | 4-16 |
| `-num_vc` | Virtual channels | 1-4 |
| `-injection_rate` | Traffic load | 0.01-0.3 |
| `-routing` | Routing algorithm | XYZ, WEST_FIRST, ODD_EVEN |
| `-traffic` | Traffic pattern | RANDOM, HOTSPOT, BITREVERSE |
| `-simtime` | Simulation duration | 10K-1M cycles |
| `-tech` | Technology node | 22, 32, 45, 65, 90 nm |
| `-cal_temp` | Thermal modeling | 0 (off) or 1 (on) |

## Understanding the Docs

### Complexity Levels

**Beginner** (Start here):
- README.md
- INSTALLATION.md
- RUNNING_SIMULATIONS.md (first examples)

**Intermediate** (After basic usage):
- CONFIGURATION.md
- ARCHITECTURE.md
- RUNNING_SIMULATIONS.md (advanced examples)

**Advanced** (For development):
- FILE_GUIDE.md
- DEVELOPER_GUIDE.md
- MODELLING.md

### Time to Read

- INSTALLATION.md: 15 minutes
- RUNNING_SIMULATIONS.md: 30 minutes
- CONFIGURATION.md: 30 minutes
- ARCHITECTURE.md: 45 minutes
- FILE_GUIDE.md: 1 hour
- DEVELOPER_GUIDE.md: 1-2 hours
- MODELLING.md: 1 hour

**Total:** ~5-6 hours for complete understanding

## Common Questions

**Q: Where do I start?**
A: Read INSTALLATION.md first, then RUNNING_SIMULATIONS.md with basic examples.

**Q: How do I configure parameters?**
A: See CONFIGURATION.md for complete reference of all options.

**Q: How does the simulator work?**
A: See ARCHITECTURE.md for system overview, then FILE_GUIDE.md for component details.

**Q: How do I add a new feature?**
A: See DEVELOPER_GUIDE.md for modification instructions.

**Q: How are power and temperature calculated?**
A: See MODELLING.md for detailed explanation of ORION and Hotspot integration.

**Q: Why are my results different from expected?**
A: Check CONFIGURATION.md for parameter settings and RUNNING_SIMULATIONS.md for troubleshooting.

**Q: Can I modify the source code?**
A: Yes! See DEVELOPER_GUIDE.md for guidelines on modifying PAT-Noxim.

## External Resources

### Original Papers and Projects
- **Noxim Original**: https://github.com/davidepatti/noxim
- **PAT-Noxim Paper**: Norollah et al., "PAT-Noxim: A Precise Power & Thermal Cycle-Accurate NoC Simulator," SOCC 2018
- **ORION**: https://www.ece.cmu.edu/~delta/tools/orion/
- **Hotspot**: https://www.ece.cmu.edu/~pmchen/tools/hotspot/

### Technical Resources
- **SystemC**: https://systemc.readthedocs.io/
- **C++ Guidelines**: https://github.com/isocpp/CppCoreGuidelines
- **Network-on-Chip Design**: Various IEEE/ACM papers on NoC architectures

## Getting Help

### Troubleshooting Steps

1. **Installation issues**: See INSTALLATION.md troubleshooting section
2. **Simulation issues**: See RUNNING_SIMULATIONS.md troubleshooting section
3. **Configuration issues**: See CONFIGURATION.md parameter reference
4. **Code modification issues**: See DEVELOPER_GUIDE.md debugging section

### Contact

- **Email**: a.norollah.official@gmail.com
- **GitHub Issues**: If available in repository
- **Review documentation**: Most issues have answers in provided docs

## Documentation Maintenance

This documentation is maintained alongside the code. If you find:
- **Outdated information**: Please report it
- **Missing sections**: Let us know what you need
- **Unclear explanations**: Suggestions are welcome

## Version Information

- **PAT-Noxim Version**: Latest (2018+)
- **SystemC Version**: 2.2.0
- **ORION Version**: 3.0
- **Hotspot Version**: 6.0
- **Documentation Version**: 2024 (Updated)

## Summary

PAT-Noxim is a comprehensive NoC simulator with detailed documentation for:

✓ **Installation**: Complete setup guide
✓ **Usage**: Examples from basic to advanced
✓ **Configuration**: Reference for all parameters
✓ **Architecture**: System design and components
✓ **Development**: Modification and extension guide
✓ **Modeling**: Power and thermal details

**Start with** [INSTALLATION.md](INSTALLATION.md) → [RUNNING_SIMULATIONS.md](RUNNING_SIMULATIONS.md)

**Then explore** documentation based on your needs.

Happy simulating!

