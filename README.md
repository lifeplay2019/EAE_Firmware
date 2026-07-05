# EAE Firmware - Section 7.1 Simulation

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

## Interview Explanation

The simplest explanation is:

1. The plant publishes temperature and status frames onto a simulated CAN bus.
2. The firmware reads the latest CAN frames every scan.
3. The state machine decides whether the system is `OFF`, `RUNNING`,
   `WARNING`, or `FAULT`.
4. If cooling is allowed, the PID loop calculates how much pump/fan command is
   needed to pull temperature back toward the target.
5. The firmware sends pump, fan, derate, and state signals back out over the
   simulated CAN bus.
6. Unit tests prove the important behavior before connecting this logic to real
   hardware.
