# PAT-Noxim Installation Guide

## Overview

This guide provides comprehensive instructions for installing PAT-Noxim (Power & Thermal Cycle-Accurate NoC Simulator) on your system.

## Prerequisites

Before installing PAT-Noxim, ensure you have the following components installed:

### Required Software

- **SystemC 2.2.0**: The core simulation framework
  - Download: https://github.com/systemc/systemc-2.2.0
  - Required for cycle-accurate simulation
- **G++ Compiler**: GNU C++ compiler (version 4.x or higher)

  - On Ubuntu/Debian: `sudo apt-get install g++`
  - On macOS: `xcode-select --install`
  - On Windows: Use MinGW or MSVC

- **Make**: Build automation tool

  - On Ubuntu/Debian: `sudo apt-get install build-essential`
  - On macOS: Included with Xcode
  - On Windows: Use MinGW or nmake

- **tcsh Shell**: Required for SystemC compilation
  - On Ubuntu/Debian: `sudo apt-get install tcsh`
  - On macOS: `brew install tcsh`
  - On Windows: Install through WSL or Cygwin

## Step-by-Step Installation

### Step 1: Install SystemC 2.2.0

This is the most critical dependency for PAT-Noxim.

```bash
# Download SystemC 2.2.0
git clone https://github.com/systemc/systemc-2.2.0.git
cd systemc-2.2.0

# Create temporary build directory
mkdir objdir
cd objdir

# Install tcsh if not already installed
sudo apt-get install tcsh

# Start tcsh shell
tcsh

# Set environment for compilation
setenv CXX g++

# Configure SystemC
../configure

# Compile (this may take 10-15 minutes)
gmake

# Install SystemC (may require sudo)
gmake install

# Exit tcsh and return to project directory
exit
cd ../..

# Clean up build directory
rm -rf objdir
```

**Important Note**: After installation, ensure the directory `lib-linux` exists. If it's named `lib-linux64`, rename it:

```bash
mv lib-linux64 lib-linux
```

### Step 2: Set Environment Variables

After SystemC installation, set environment variables for compilation:

```bash
# Add to your ~/.bashrc or ~/.profile
export SYSTEMC_HOME=/usr/local/systemc-2.2.0  # Or wherever you installed it
export LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux:$LD_LIBRARY_PATH
export CPLUS_INCLUDE_PATH=$SYSTEMC_HOME/include:$CPLUS_INCLUDE_PATH
```

Then reload your environment:

```bash
source ~/.bashrc
```

### Step 3: Install PAT-Noxim

```bash
# Navigate to PAT-Noxim project directory
cd PAT-Noxim/bin

# Compile the simulator
make

# Install the compiled binary (optional)
make install
```

### Step 4: Verify Installation

Test if PAT-Noxim compiled successfully:

```bash
# Navigate to bin directory
cd PAT-Noxim/bin

# Run with help option
./noxim -h
```

You should see a list of available command-line options if the installation was successful.

## Build System Details

### Makefile Components

The build system consists of three main files in `PAT-Noxim/bin/`:

- **Makefile**: Main build configuration

  - Defines source files
  - Sets compilation flags
  - Links ORION power models
  - Integrates Hotspot thermal models

- **Makefile.defs**: Build definitions

  - Sets linker flags
  - Defines library paths
  - Configures SystemC linking

- **Makefile.deps**: Dependency management
  - Tracks header file dependencies
  - Ensures proper rebuild on changes

### Source Organization

The build process compiles from multiple modules:

```
bin/
├── Makefile                    # Main build file
├── Makefile.defs              # Build definitions
└── Makefile.deps              # Dependencies
```

Source files are located in `../src/` and organized as:

- **Noxim Core** (NoximXXX.cpp/h files)
- **ORION Power Model** (ORION\_\*.c/h files)
- **Hotspot Thermal Model** (Hotspot\_\*.c/h files)

## Troubleshooting

### Common Issues and Solutions

#### 1. "SystemC library not found"

```bash
# Solution: Set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/systemc-2.2.0/lib-linux:$LD_LIBRARY_PATH
```

#### 2. "lib-linux64 not compatible"

```bash
# Solution: Rename the directory
cd /usr/local/systemc-2.2.0/
mv lib-linux64 lib-linux
```

#### 3. "g++: command not found"

```bash
# Solution: Install g++
sudo apt-get install g++
# Or on macOS
xcode-select --install
```

#### 4. "tcsh: command not found" during SystemC build

```bash
# Solution: Install tcsh
sudo apt-get install tcsh
```

#### 5. Compilation errors with ORION or Hotspot modules

- Ensure all source files in `src/` are present
- Check that `Makefile` references all required source files
- Verify gcc version compatibility (use gcc 4.x or 5.x for best compatibility)

## Platform-Specific Notes

### Linux (Ubuntu/Debian)

- Fully supported
- Follow standard installation steps above
- Use `apt-get` for package management

### macOS

- SystemC may require special configuration
- Install Xcode command line tools first
- May need to adjust `configure` flags in SystemC setup

### Windows

- Use Windows Subsystem for Linux (WSL) for best results
- Alternatively, install MinGW or Cygwin
- MSVC compatibility may require additional configuration

## Next Steps

After successful installation:

1. **Read Configuration Guide**: See `CONFIGURATION.md` to understand simulation parameters
2. **Review Architecture**: See `ARCHITECTURE.md` for system design overview
3. **Check File Guide**: See `FILE_GUIDE.md` for source file descriptions
4. **Run Example**: See `RUNNING_SIMULATIONS.md` for example simulations
5. **Developer Guide**: See `DEVELOPER_GUIDE.md` if you plan to extend the simulator

## Additional Resources

- **SystemC Documentation**: https://systemc.readthedocs.io/
- **ORION Power Model**: Embedded in `src/ORION_*.c/h` files
- **Hotspot Thermal Model**: Embedded in `src/Hotspot_*.c/h` files
- **PAT-Noxim Paper**: A. Norollah et al., "PAT-Noxim: A Precise Power & Thermal Cycle-Accurate NoC Simulator," SOCC 2018

## Getting Help

If you encounter issues:

1. Check this troubleshooting section
2. Review the main README.md
3. Examine compiler error messages carefully
4. Contact: a.norollah.official@gmail.com
