# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

This project uses GNU Make with ChibiOS RTOS for STM32F446 microcontroller.

### Common Commands

Build firmware:
```bash
make
```

Build for specific board (default is dingopdm_v7):
```bash
make BOARD=dingopdm_v7
# OR
make BOARD=dingopdmmax_v1
```

Clean build artifacts:
```bash
make clean
```

Debug build (change optimization from -O2 to -O0):
```bash
make USE_OPT="-O0 -ggdb -fomit-frame-pointer -falign-functions=16 -fsingle-precision-constant"
```

### Board Variants

- `dingopdm_v7` - Main PDM board (default)
- `dingopdmmax_v1` - Extended capacity variant

Build artifacts are placed in `./build/` directory.

## Architecture Overview

### Core Structure

The firmware is built on ChibiOS RTOS with a modular C++20 architecture:

- **Main Loop**: Simple initialization + sleep loop, all functionality in threaded modules
- **Board Abstraction**: Hardware-specific code in `boards/[board]/` directories
- **Configuration Management**: Persistent config stored in FRAM, managed by `config.cpp` and `config_handler.cpp`
- **Communication**: CAN bus + USB for configuration and monitoring

### Key Components

**Power Distribution (`pdm.cpp`, `profet.cpp`)**:
- Infineon Profet smart switch control
- Overcurrent protection and diagnostics
- PWM control for variable loads

**Input Processing**:
- `input.cpp` - Physical digital inputs with debouncing
- `can_input.cpp` - CAN-based virtual inputs
- `virtual_input.cpp` - Logic-based virtual inputs with conditions

**Output Control**:
- `profet.cpp` - Smart switch drivers
- `pwm.cpp` - PWM output generation
- `led.cpp` - Status/diagnostic LEDs

**Automotive Features**:
- `wiper/` - Windshield wiper control logic
- `starter.cpp` - Engine starter management
- `flasher.cpp` - Turn signal/hazard flasher logic

**Monitoring & Safety**:
- `error.cpp` - Error handling and diagnostics
- Temperature monitoring via MCP9808
- Current sensing and protection

**Communication**:
- `can.cpp` - CAN bus communication
- `usb.cpp` - USB configuration interface
- `msg.cpp` - Message protocol handling

### Configuration System

The system uses a versioned configuration structure (`CONFIG_VERSION`) stored in FRAM:
- `config.h` - Configuration structure definitions
- `config_handler.cpp` - Persistent storage management
- `hardware/mb85rc.cpp` - FRAM driver

Configuration changes require incrementing `CONFIG_VERSION` to handle migrations.

### Hardware Abstraction

Board-specific code is isolated in `boards/[board]/`:
- `board.c/h` - ChibiOS board definitions
- `port.cpp/h` - Hardware pin assignments and initialization
- `hw_devices.cpp/h` - Board-specific device drivers
- Custom linker scripts for memory layout

### Thread Architecture

ChibiOS threads handle:
- Periodic input scanning and debouncing
- Output state management and fault detection
- CAN message processing
- USB communication
- Temperature and voltage monitoring
- Keypad processing (if enabled)

### Message Protocol

USB and internal communication uses a structured message system:
- `enums.h` - Command and status enumerations
- `msg.cpp` - Message parsing and response generation
- Commands for reading inputs, controlling outputs, configuration

## Hardware Configuration

The build system automatically selects hardware configuration based on `BOARD` variable. Each board variant has different:
- GPIO assignments
- Memory layout
- Available peripherals
- Device drivers

STM32F446 specific features:
- ARM Cortex-M4 with FPU
- 180MHz operation
- Hardware CAN controller
- Multiple timer units for PWM