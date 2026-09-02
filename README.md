# Solist-AI ML63Q2537 Project Template

Minimal CMake project template for Solist-AI (ML63Q2537) development.

> New here? Start with **[GETTING_STARTED.md](GETTING_STARTED.md)** — it walks through what the template provides, how to use each piece as-is, and how to modify them for your application.

## Project Structure

```
.
├── cmake/                      # CMake toolchain configuration
├── driver/                     # Hardware driver library
│   ├── inc/                    # Driver headers
│   ├── src/                    # Driver implementations
│   └── lib/                    # ROHM Solist-AI accelerator library (binary)
├── utility/                    # Board support utilities
│   └── board/                  # Board-specific functions
├── src/                        # Application source code
│   └── main.c                  # Application entry point
├── ml63q25x7/                  # MCU-specific files
│   ├── Source/                 # Startup code and linker script
│   └── include/                # CMSIS device headers
├── tests/                      # On-target test framework and suites
├── scripts/                    # hex_to_flash.py (Intel HEX -> OpenOCD TCL)
├── openocd/                    # OpenOCD config with ML63Q2537 flash routines
├── .github/workflows/          # Build CI
├── LICENSE                     # Terms for p-ban.com Corp. authored files
├── LICENSE.apache-2.0          # Apache-2.0 text (startup/system/linker, CMSIS)
├── NOTICE                      # Attribution summary
└── THIRD_PARTY_LICENSES.md     # Per-file license breakdown

```

## Prerequisites

- **ARM GCC Toolchain**: `arm-none-eabi-gcc` (tested with version 10.3 or later)
- **CMake**: 3.16 or higher
- **CMSIS**: ARM Cortex Microcontroller Software Interface Standard

## Setup

### 1. Initialize submodules

CMSIS is tracked as a git submodule under `external/CMSIS`:

```bash
git submodule update --init --recursive
```

## Build

### Basic Build

```bash
# Create build directory
mkdir build && cd build

# Configure project
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build
make -j4
```

### Output Files

After successful build, you will find:
- `solist_ai_template` - ELF executable
- `solist_ai_template.hex` - Intel HEX format (for flashing)
- `solist_ai_template.bin` - Raw binary format

### Build Types

```bash
# Debug build (with symbols, -O0)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (optimized, -Os)
cmake -DCMAKE_BUILD_TYPE=Release ..
```

## Flash Programming

### Using J-Link (recommended)

```bash
scripts/jlink_flash.sh                 # programs the .hex found in build/
scripts/jlink_flash.sh <build_dir>
scripts/jlink_flash.sh --file <image>  # .hex / .elf, or .bin with an address
```

```powershell
powershell -ExecutionPolicy Bypass -File scripts\jlink_flash.ps1
```

`jlink/JLinkDevices.xml` declares the ML63Q2537 for J-Link (the MCU is not in its
built-in database) and binds the flash bank at `0x10000000` to the vendor CMSIS
flash algorithm `jlink/ML63Q25x7.FLM`, taken from ROHM.ML63Q25x7_DFP 0.4.0.
J-Link downloads that algorithm into target RAM and runs it on the Cortex-M0+:
its `Init()` raises the clock to the 48 MHz PLL, and the erase/program loops
drive the flash controller and clear the watchdog while polling FLASHSTA. SWD
only carries the image data, instead of one round trip per programmed 32-bit
word.

Environment overrides: `JLINK_EXE`, `JLINK_SPEED` (kHz, default 4000),
`JLINK_SN`, `JLINK_VTREF` (mV, for probes whose VTref pin is not wired),
`JLINK_NO_RUN=1` (leave the core halted instead of reset-and-run).

### Using OpenOCD

The ML63Q2537 has a custom flash controller that requires the system clock at 48 MHz PLL and a specific accept-flag unlock sequence before any erase/write. All of that is implemented as TCL procs in `openocd/openocd.cfg`, so flashing is a three-step process:

```bash
# 1. Generate a TCL script of flash_write_word calls from the built hex
python3 scripts/hex_to_flash.py build/solist_ai_template.hex flash_from_hex.tcl

# 2. Start openocd in one terminal
openocd -f openocd/openocd.cfg
```

```bash
# 3. From another terminal, drive it via telnet
telnet localhost 4444
> prepare_flash             ;# erase + clock setup (defaults: 0x10000000, 256 KB)
> source flash_from_hex.tcl
```

### Debug adapter

`openocd/openocd.cfg` is configured for a J-Link over SWD (`adapter driver jlink`, `transport select swd`). For a different probe, replace those two lines with the ones your adapter needs; the flash programming procs in that file are adapter-independent.

## Hardware Information

- **MCU**: ML63Q2537 (ROHM/Lapis)
- **Core**: ARM Cortex-M0+ @ 48MHz
- **Flash**: 256KB (starts at 0x10000000)
- **RAM**: 16KB (starts at 0x20000000)

## Solist-AI Accelerator

The AI path is wired up out of the box. `driver/CMakeLists.txt` links one of
ROHM's prebuilt archives from `driver/lib/` into `libdriver.a`, and
`driver/inc/solistAi.h` declares the FFT, on-device-learning (ODL) and OSUAD
entry points — include the header and call them, no extra build steps.

| CMake option | Archive | Models | Inputs | Hidden units |
|---|---|---|---|---|
| `-DSOLIST_AI_LIB_512=ON` (default) | `SolistAi_Library_1_512_64.a` | 1 | 512 | 64 |
| `-DSOLIST_AI_LIB_512=OFF` | `SolistAi_Library_2_256_64.a` | 2 | 256 | 64 |

The selection also sets `ODL_MAX_INST_NUM` / `ODL_MAX_INPUTS` /
`ODL_MAX_UNITS` for every target linking `driver`, so the header's array sizes
match the archive that is actually linked.

The archives are compiled with 32-bit enums, which is why `CPU_FLAGS` carries
`-fno-short-enums`: GCC defaults to `-fshort-enums` on the ARM EABI, and
without the flag `ld` warns that "use of enum values across objects may fail"
as soon as anything calls `fft_Init(uint16_t size, FftWindow window)`.

These archives are **copyright ROHM Co., Ltd.** — see
[`driver/lib/README.md`](driver/lib/README.md).

## Application Entry Point

The template's `src/main.c` is a minimal scaffold: it calls `device_initialize()` (which configures the system clock, starts a 2-second watchdog and brings up UARTF0), prints a greeting at 115200 8N1 on P33/P32, and then loops, clearing the watchdog. Drop your application code into the loop body.

```c
#include <stdint.h>
#include "ML63Q25x7.h"
#include "wdt.h"
#include "device.h"
#include "uart_print.h"

int main(void)
{
  if (device_initialize() != 0) {
    return -1;
  }

  uart_print_puts("Hello world!\r\n");
  uart_print_flush();

  while (1) {
    wdt_clear();
    /* Your application code here */
  }
  return 0;
}
```

`device_initialize()` returns `0` on success and non-zero if a step failed — `1` means UARTF0 could not be configured. Don't let `main()` return: newlib routes it to `_exit()`, which masks interrupts and parks the core in a `wfi` loop.

## Development

### Adding Source Files

1. Add your .c files to `src/` directory
2. Update `src/CMakeLists.txt`:

```cmake
set(APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/your_file.c
    PARENT_SCOPE
)
```

### Using Drivers

Available drivers in `driver/` directory:
- WDT (Watchdog Timer)
- SSIOF0 (SPI)
- UARTF0 (UART)
- IRQ (Interrupt controller)

Include headers:
```c
#include "wdt.h"
#include "ssiof0.h"
#include "uartf0.h"
```

### Using Utilities

Board utilities in `utility/board/`:
- LED control
- Peripheral initialization helpers

## Notes

- **Watchdog Timer**: Always active, must be cleared regularly with `wdt_clear()`
- **Flash Programming**: Requires 48MHz system clock
- **Stack Size**: 1KB (configured in linker script)
- **Heap Size**: 3KB (configured in linker script)

## Troubleshooting

### Build Errors

1. **Toolchain not found**:
   ```bash
   # Check ARM GCC installation
   which arm-none-eabi-gcc
   ```

2. **CMSIS headers missing**:
   ```bash
   # Verify CMSIS installation
   ls external/CMSIS/CMSIS/Core/Include/
   ```

### Flashing Issues

- Ensure SWD connection is properly wired
- Check debug probe (J-Link, CMSIS-DAP, etc.) is recognized
- Verify OpenOCD configuration matches your debug probe

## License

You may **use and modify** everything in this repository to develop firmware
for a microcontroller manufactured by ROHM Co., Ltd. (including LAPIS-branded
products), and **ship that firmware in binary form inside your own products**.

You may **not redistribute the source** of this repository, or a derivative of
it, to third parties.

That rule applies to every file, whether it is owned by p-ban.com Corp. or by
ROHM Co., Ltd. See [`LICENSE`](LICENSE); [`LICENSE.ja`](LICENSE.ja) is the
authoritative Japanese text.

Two things sit outside it:

- The startup, system, and linker script files and the `external/CMSIS`
  submodule are Apache-2.0 and are not restricted by `LICENSE`. See
  [`LICENSE.apache-2.0`](LICENSE.apache-2.0).
- `driver/inc/`, `driver/src/`, `utility/board/`,
  `ml63q25x7/include/ML63Q25x7.h`, `src/codeoption*`, and `src/uartf_int*` are
  **copyright ROHM Co., Ltd.**, derived from the ML63Q2500 Reference Software.
  Their terms come from ROHM's notice in each file, not from `LICENSE`, and
  p-ban.com Corp. cannot sublicense them. Their copyright notices must not be
  removed.
- `driver/lib/SolistAi_Library_*.a` (the Solist-AI accelerator library) and
  `jlink/ML63Q25x7.FLM` (the flash algorithm) are **copyright ROHM Co., Ltd.**
  on those same terms. Being binaries they carry no header of their own, so
  their notice lives in [`driver/lib/README.md`](driver/lib/README.md) and in
  the comment block of `jlink/JLinkDevices.xml`. Read
  [`driver/lib/README.md`](driver/lib/README.md) before shipping or passing on
  anything that contains them — ROHM's notice grants no right of
  redistribution.

### Why this repository is public

p-ban.com Corp. distributes this repository, including the ROHM-owned files
above, under its partner agreement with ROHM Co., Ltd. That agreement permits
p-ban.com Corp. to provide ROHM-supplied software to third parties, and places
sole responsibility for doing so on p-ban.com Corp.

That basis does not extend to you. It is p-ban.com Corp.'s right to
distribute, not a licence for you to redistribute — which is why the
no-redistribution rule above still applies to you even though this repository
is published. Your rights in the ROHM-owned files come only from the ROHM
notice at the head of each file.

ROHM Co., Ltd. carries no warranty, support, or liability obligation for this
repository or for anything built from it. Direct all questions to
p-ban.com Corp., not to ROHM.

[`THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md) has the per-file
breakdown. Attribution summary: [`NOTICE`](NOTICE).

Copyright © 2026 株式会社ピーバンドットコム
