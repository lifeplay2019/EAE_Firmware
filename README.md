# EAE Firmware - Section 7.1 Simulation

## Completion Note

This repository is not a complete implementation of every optional Section 7.1
firmware requirement. Due to the limited submission timeline, I focused on a
clear, runnable firmware-style simulation that demonstrates the main control
concepts: CAN message simulation, PID-based cooling demand, state-machine
handling, command-line runtime parameters, CMake project structure, and unit
tests.

I did not complete a full Linux-based firmware build environment. My available
Linux machine is a work machine and was not appropriate to use for this
submission. I also considered MSYS2 as a Unix-like development and build
environment on Windows, but it conflicted with one of my CUDA kernel
development environments. For that reason, this project should be reviewed as a
Windows/MSVC-validated firmware simulation framework rather than a fully
finished cross-platform firmware package.

This folder contains a C++17 firmware-style simulation for the optional
Section 7.1 Firmware task in the EAE Electrical and Controls Challenge.

It demonstrates:

- simulated CAN receive and transmit traffic;
- a PID cooling loop;
- a cooling controller state machine;
- command-line setpoints and simulation parameters;
- CMake-based build configuration;
- GoogleTest unit tests without shipping third-party dependency source.

The project is intentionally small enough to explain in an interview, but it is
organized like a real embedded software module rather than one large script.

## System Model

The simulated controller receives these CAN signals:

- coolant temperature;
- coolant level status;
- ignition/run request;
- pump fault status.

It sends these CAN outputs:

- pump command percentage;
- fan command percentage;
- derate request;
- controller state code.

The PID loop compares the measured coolant temperature against the target
temperature. When temperature rises above the target, the PID output increases
cooling demand. The state machine decides whether that demand is allowed:

- `OFF`: ignition is off;
- `RUNNING`: system is enabled and healthy;
- `WARNING`: temperature is high but still controllable;
- `FAULT`: unsafe condition, such as low coolant, invalid sensor, pump fault,
  or critical over-temperature.

## Files

- `CMakeLists.txt` - CMake project and GoogleTest wiring.
- `include/eae_firmware` - public module headers.
- `src` - implementation and demo executable.
- `tests` - GoogleTest unit tests.
- `scripts/run_demo.ps1` - Windows PowerShell demo launcher.
- `scripts/run_demo.sh` - Linux/MSYS2 shell demo launcher.

## Run

```powershell
.\build\Release\eae_firmware.exe --target-temp 65 --initial-temp 42 --steps 90 --dt 1.0
```

With single-config generators such as Ninja or MinGW Makefiles:

```powershell
.\build\eae_firmware.exe --target-temp 65 --initial-temp 42 --steps 90 --dt 1.0
```

You can also use:

```powershell
.\scripts\run_demo.ps1
```

On Linux or MSYS2:

```bash
./scripts/run_demo.sh
```

## Useful Runtime Parameters

- `--target-temp <C>`: coolant temperature target used by the PID loop.
- `--initial-temp <C>`: starting coolant temperature.
- `--steps <N>`: number of simulated controller scans.
- `--dt <seconds>`: controller scan period.
- `--heat-load <kW>`: inverter/DC-DC heat load.
- `--coolant-ok <0|1>`: start with good or low coolant.
- `--ignition <0|1>`: start with ignition off or on.
- `--pump-fault <0|1>`: start with a simulated pump fault.

Example fault run:

```powershell
.\build\Release\eae_firmware.exe --target-temp 65 --initial-temp 78 --pump-fault 1
```

## Test

```powershell
ctest --test-dir build --output-on-failure -C Release
```

The tests cover:

- CAN receive/transmit mailbox behavior;
- PID output direction, clamping, and reset behavior;
- state-machine transitions;
- integrated firmware command behavior.
