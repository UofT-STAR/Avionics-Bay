# Development, debugging, and testing

## Understand the two programs

`src/test1.cpp` uses SparkFun's ICM-20948 library. It starts serial communication at 115200 baud, initializes I2C, and prints X/Y acceleration when data is ready. Its `AD0_VAL` is `0`; check the sensor's address configuration against the actual breakout. Check the installed library's units before interpreting acceleration values.

`src/main.cpp` uses the MS5611 and ICM20948_WE libraries alongside SD and SPI. It initializes sensors and SD storage, estimates a ground altitude baseline, then runs a state machine with `PRE_LAUNCH`, `ASCENT`, `DESCENT`, and `LAND` states. It also controls deployment outputs. Review and testing of this program require a separately agreed setup; do not switch the onboarding exercise to it casually.

The two IMU libraries expose different APIs. Follow the library used by the source file you are changing rather than copying calls between the two programs.

## Build and inspect

From the repository root in a PlatformIO terminal:

```sh
pio run -e legacy_uno
```

For more compiler detail:

```sh
pio run -e legacy_uno -v
```

If you suspect stale build output, clean and rebuild:

```sh
pio run -e legacy_uno -t clean
pio run -e legacy_uno
```

Read the first meaningful compiler error before the later errors it causes. Check the file being compiled, selected environment, and dependency versions. The configuration contains a mix of version constraints and an unversioned `SD` dependency, so do not assume every fresh installation resolves identically.

## Diagnose a bench problem

| Symptom | First checks |
| --- | --- |
| `pio` is not recognized | Follow the [PowerShell PATH instructions](../README.md#if-powershell-cannot-find-pio), or open a PlatformIO terminal in VS Code. |
| Library or platform installation fails | Read the download error and check network access; retain the error output. |
| Upload fails or port cannot be opened | Confirm board and bootloader, detected port, data cable, and that no other monitor owns the port. |
| Serial output is unreadable | Confirm monitor speed is 115200 baud. |
| Sensor experiment prints nothing | Confirm the correct program was uploaded, then inspect sensor initialization, power, I2C wiring, address configuration, and data-ready status with a teammate. |
| Edits appear to have no effect | Check `build_src_filter`: only `test1.cpp` currently builds. |

Add small, clearly labeled serial diagnostics around the failing operation. Avoid printing so much that it changes the timing you are trying to inspect. Record enough context to reproduce the issue before changing several things at once.

No hardware debugger configuration is checked in. Serial diagnostics are the documented starting point; breakpoint debugging needs confirmation of board, probe, wiring, and tooling support.

## Testing and evidence

There is currently no checked-in automated test suite or CI workflow. A successful compile verifies only the selected program's build, not sensor correctness, timing, or flight behavior.

Use these levels as a proposed testing progression:

1. **Build:** compile the affected program and report the environment and result.
2. **Sensor-only bench check:** on an approved setup, verify initialization and plausible readings, including a controlled orientation change. Save representative serial output.
3. **Logic tests:** as testable modules are introduced, use synthetic readings to check state transitions, counters, invalid data, and boundary cases without actuating hardware. This infrastructure still needs to be built.
4. **Integration review:** agree on a hardware test procedure and acceptance criteria with the team before testing the larger avionics program.

For every test, record source revision, environment, hardware/setup if applicable, input or action, expected result, actual result, and pass/fail. If a check was not run, say so.

## Source details that need review

These observations concern the currently excluded `main.cpp` and are useful review topics, not validated fixes:

- **Logging units:** altitude is multiplied by 100 before conversion to `int16_t`, while the parameter name and comment say decimeters. The scale and representable range need agreement before using logs for analysis.
- **Initialization failures:** several failure paths print an error and continue because their halt statements are commented out. Define the intended behavior when a sensor or SD initialization fails.
- **Counters:** the ascent and landing checks do not reset their counters on every nonmatching sample. Clarify whether the intended requirement is consecutive readings or accumulated readings.
- **Timing:** blocking delays and SD operations affect sampling and logging cadence; comments describing exact rates should not be treated as measurements.

Keep changes to these behaviors separate from onboarding documentation and support them with reviewed requirements and reproducible tests.
