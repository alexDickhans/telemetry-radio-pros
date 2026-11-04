# libTelemetryRadio

PROS library that samples measurements and log lines on a VEX V5 Brain and hands the same payload to every registered transmitter. This repository is the library source and a sample V5 project. The installable template is [`libTelemetryRadio@0.0.1.zip`](libTelemetryRadio@0.0.1.zip).

Transmitters in this library are:

- `PT::USB`, which writes the payload to the USB console read by `pros terminal`
- `PT::TelemetryRadio`, which writes the payload to a V5 smart port at 115200 baud

## Requirements

- PROS kernel 3.8.0 for the V5 (`project.pros` records that kernel and target)
- PROS CLI, to build this project and to apply the template

OkapiLib 4.8.0 is applied in this repository. The telemetry sources use `pros::Serial`, `pros::Task`, `pros::Mutex`, and `pros::millis`. A project that only applies the zip can leave OkapiLib uninstalled.

## Install

From the PROS project that should send telemetry:

```bash
pros conduct apply /path/to/libTelemetryRadio@0.0.1.zip
```

`pros c apply` is the same command. The template installs:

- headers under `include/telemetryRadio/`
- `firmware/libTelemetryRadio.a`

The PROS Makefile links every archive in `firmware/`. Add the library header where the program can see it, for example in `include/main.h`:

```cpp
#include "telemetryRadio/telemetryManager.hpp"
```

Public types live in namespace `PT`.

## Use

`src/main.cpp` is a sample robot program. The library archive excludes `src/main.*`, so applying the template does not replace a project's `main.cpp`. The sample sets the logger to `warn`, registers `pros::micros`, four controller axes, and one placeholder measurement, then sends them to USB and to a radio on smart port 20 every 20 ms. `opcontrol` calls `logger->info("hello")`; at `warn`, that line is not stored. The same function also runs a two-motor tank drive on ports 1 and 2.

`PT::TelemetryManager` and `PT::Logger` are singletons. `FunctionMeasurement` stores a `T (*)()` function pointer, so the callable cannot capture local variables. A file-scope object, as in the sample, is visible to that pointer.

```cpp
pros::Controller controller(pros::E_CONTROLLER_MASTER);

void initialize() {
    PT::Logger* logger = PT::Logger::getInstance();
    logger->setLogLevel(PT::LogLevel::info);

    PT::TelemetryManager* telemetry = PT::TelemetryManager::getInstance();
    telemetry->addTransmitter(
        std::make_shared<PT::USB>(new PT::PassThroughEncoding()));
    telemetry->addTransmitter(
        std::make_shared<PT::TelemetryRadio>(20, new PT::PassThroughEncoding()));
    telemetry->addMeasurementSource(
        std::make_shared<PT::FunctionMeasurement<int>>(
            "ControllerLeftY", "Value",
            []() -> int { return controller.get_analog(ANALOG_LEFT_Y); }));
    telemetry->addMeasurementSource(
        std::make_shared<PT::FunctionMeasurement<uint64_t>>(
            "System", "Micros", pros::micros));

    telemetry->setUpdateTime(20);
    telemetry->enableUpdateScheduler();
}

void opcontrol() {
    PT::Logger::getInstance()->info("opcontrol started");
    while (true) {
        pros::delay(20);
    }
}
```

The `TelemetryRadio` port is the V5 smart port in the range 1–21.

## Payload

`TelemetryManager::update()` updates every measurement, builds one string, and passes that string to every transmitter.

A measurement is included only when it is marked updated. `FunctionMeasurement::update()` calls the function, converts the result with `std::to_string`, and marks the measurement updated when that text differs from the previous sample. `to_string()` clears the mark, so an unchanged value is left out of later payloads.

The measurement line is produced by `Measurement::formatSingle`:

```text
{group} {name}={value} \n
```

The space before the newline is part of the format. A controller sample looks like:

```text
ControllerLeftY Value=-12 \n
```

A base `Measurement` starts marked updated and its `to_string()` writes `NULL`, so it contributes one line, `{name} {name}=NULL \n`, and then stays quiet.

Logger lines are appended after the measurements. `Logger` stores a line when its level number is greater than or equal to the call's level number:

| Call | Number |
| --- | --- |
| `debug` | 4 |
| `info` | 3 |
| `warn` | 2 |
| `error` | 1 |
| `off` | 0 |

The default level is `warn`, so `warn` and `error` are stored and `info` and `debug` are dropped. `off` stores nothing. A stored line looks like:

```text
#INFO,12345: message\n
```

`12345` is `pros::millis()` at the logging call. The manager sends the buffered lines on the next `update()` and then clears them.

`update()` still calls every transmitter when the string is empty. With `PassThroughEncoding` that write is zero bytes. With `COBSEncoding`, an empty input still becomes a delimited packet.

`enableUpdateScheduler()` starts a `pros::Task` that calls `update()` and then delays. The delay aims for one loop per `updateTime` milliseconds. The default `updateTime` is 10; the sample sets 20. If `update()` has already used the whole period, the task waits another full period. `disableUpdateScheduler()` returns `taskRunning` and leaves that task running. Nothing in the library assigns `taskRunning`, so the returned bool is indeterminate. A second `update()` call while the task is running samples and sends again.

## Encoders

Pass a heap-allocated encoder into each transmitter.

- `PassThroughEncoding` forwards the bytes unchanged. `USB` uses this when no encoder is passed.
- `COBSEncoding` applies Consistent Overhead Byte Stuffing and appends a `0x00` delimiter. `decode()` reads one delimited packet.
- Base `Encoding::encode` returns an empty buffer. `TelemetryRadio` uses that base type when no encoder is passed, so the radio then writes no payload bytes.

`USB` prints each output byte with `printf("%c", ...)` and deletes its encoder in the destructor. `TelemetryRadio` writes the encoded bytes with `pros::Serial` at `TELEMETRY_RADIO_BAUDRATE` (115200). Each radio transmit also prints the encoded size, the `pros::Serial::write` result, and `errno` as three decimal numbers with no separator, then a newline. The radio destructor does not delete its encoder.

`include/telemetryRadio/measurementTypes/variableMeasurement.hpp` does not compile as a `Measurement`: `VariableMeasurement` does not inherit `Measurement`, and it calls helpers it does not define. `functionGroup` is an empty class. Register values with `FunctionMeasurement` or a `Measurement` subclass.

## Build

`make`, or `pros make`, builds the hot package `bin/hot.package.bin` (`USE_PACKAGE` is 1). `bin/` is gitignored.

`make template` archives `src/telemetryRadio/` into `bin/libTelemetryRadio.a` (`src/main.*` stays out of that archive), then runs `pros c create-template` and writes `libTelemetryRadio@0.0.1.zip`. `LIBNAME` and `VERSION` in the `Makefile` set that filename. The target passes `include/**/*.h`, `include/**/*.hpp`, and the new archive. The `template/` staging directory is gitignored. The committed zip contains the telemetry headers named in its `template.pros` and `firmware/libTelemetryRadio.a`. `functionGroup.hpp` is in the source tree and is absent from that zip. Commit a regenerated zip after the library sources change.

## Layout

| Path | Role |
| --- | --- |
| `include/telemetryRadio/`, `src/telemetryRadio/` | Library sources |
| `src/main.cpp` | Sample program, excluded from the static library |
| `libTelemetryRadio@0.0.1.zip` | Template to apply to other PROS projects |
| `include/pros/`, `include/display/`, `common.mk`, `firmware/libpros.a`, `firmware/libc.a`, `firmware/libm.a`, `firmware/*.ld` | PROS kernel 3.8.0 headers, LVGL, archives, and linker scripts shipped with the project |
| `include/okapi/`, `firmware/okapilib.a`, `firmware/squiggles.mk` | OkapiLib 4.8.0 files shipped with the project |

## Vendored licenses

PROS kernel sources and OkapiLib headers in this tree are under the Mozilla Public License 2.0. Their file headers point at that license. `include/display/` is LVGL and keeps the MIT text in `include/display/licence.txt`. The telemetry sources under `include/telemetryRadio/` and `src/telemetryRadio/` do not add a separate license.
