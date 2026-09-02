# Getting Started

This template is a starting point for ML63Q2537 (ROHM/Lapis "Solist-AI") firmware. It boots, configures the clock to 48 MHz PLL, brings up a UART for logging, and drops into a watchdog-clearing loop. Everything else is up to you to add.

This guide explains **what's already provided**, **how to use it as-is**, and **how to modify each piece** for your application.

## 1. Prerequisites

- `arm-none-eabi-gcc` (10.3 or newer; tested with 14.3)
- CMake ≥ 3.16
- Python 3 (for the flash helper)
- OpenOCD with J-Link or CMSIS-DAP support

## 2. First build

```bash
git submodule update --init --recursive   # pulls external/CMSIS
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j4
```

Outputs land in `build/`:

- `solist_ai_template`     — ELF
- `solist_ai_template.hex` — Intel HEX (this is what you flash)
- `solist_ai_template.bin` — raw binary

Add `-DBUILD_TESTS=ON` if you also want the on-target test binary (`solist_ai_template_test.*`).

## 3. First flash

With a J-Link probe this is one command:

```bash
scripts/jlink_flash.sh
```

It programs the `.hex` in `build/` and resets the board. On Windows use
`powershell -ExecutionPolicy Bypass -File scripts\jlink_flash.ps1`.

The rest of this section covers the OpenOCD route.

The ML63Q2537 has a custom flash controller. You can't use stock `openocd program ... verify reset exit` — the chip needs the PLL up at 48 MHz and a specific unlock sequence first. That logic is already in `openocd/openocd.cfg` as TCL procs; you just have to drive it.

```bash
# 1. Convert the hex into a TCL script of flash_write_word calls
python3 scripts/hex_to_flash.py build/solist_ai_template.hex flash_from_hex.tcl

# 2. In terminal A, start openocd
openocd -f openocd/openocd.cfg

# 3. In terminal B, talk to openocd
telnet localhost 4444
> prepare_flash             ;# erases flash + sets clock to 48 MHz
> source flash_from_hex.tcl ;# programs the firmware
```

Connect a UART adapter to **P33 (TX) / P32 (RX)** at **115200 8N1**. Out of the box the template prints one line and then idles:

```
Hello world!
```

`uart_print_init()` itself is silent — everything on the wire comes from your own `uart_print_*` / `UART_PRINT_*` calls (see §5.2). Nothing at all means the UART never came up, which `device_initialize()` reports by returning non-zero.

## 4. What you get out of the box

```
src/
  main.c            entry point — init + WDT loop, ready for your code
  device.[ch]       device_initialize() — clock, WDT, UART, peripheral enable
  uart_print.[ch]   leveled logging + printf over UARTF0
  app_irq.c         stubs for EXI / NMI / UAF0 / SIOF0 interrupt handlers
  uartf_int.[ch]    UARTF0 interrupt dispatch (ROHM-derived)
  codeoption.[ch]   code option block at 0x1003FFC0 (ROHM-derived)
  syscalls.c        newlib stubs (lets you use stdio/etc.)
driver/             ML63Q2537 peripheral drivers (built as static lib `driver`)
driver/lib/         ROHM Solist-AI accelerator library, linked and ready (§5.5)
utility/board/      board helpers — peripheral enable, LED control
ml63q25x7/Source/   startup code + GCC linker script (1 KB stack, 3 KB heap)
tests/              on-target test framework + suites
```

The default `main.c` is intentionally bare:

```c
int main(void)
{
  if (device_initialize() != 0) {
    return -1;
  }

  uart_print_puts("Hello world!\r\n");
  uart_print_flush();

  while (1) {
    wdt_clear();
  }
  return 0;
}
```

`device_initialize()` does the heavy lifting — see §5. Do not let `main()`
return: newlib routes it to `_exit()`, which masks interrupts and parks the
core in a `wfi` loop, so the board goes quiet with no further output.

## 5. The pieces, and how to use them

### 5.1 `device_initialize()` — `src/device.c`

One call sets up everything the rest of the template assumes:

- Starts the 2-second watchdog (`wdt_init(WDT_2S); wdt_clear();`)
- Switches the system clock to the 48 MHz PLL, waits for PLL stable
- Enables the UART peripheral via `smpl_enablePeripheral(USR_PERI)`
- Calls `uart_print_init()` so logging works immediately after

**To use:** call it once at the top of `main()`, and check the return code:
`0` means every step succeeded, `1` means `uart_print_init()` could not
configure UARTF0. Note the two conventions in play — the `device_*` functions
report success as `0`, `uart_print_init()` reports it as `true`.

The PLL-stable wait is the one step with no error path: if the PLL never
stabilises, `device_initialize()` spins there clearing the watchdog rather
than returning.

**To modify:**

- Want a longer WDT timeout? Change `WDT_2S` → `WDT_8S` (defined in `driver/inc/wdt.h`).
- Need more peripherals enabled at boot? Add their `*_PERI` flag to `USR_PERI` in `src/device.h` (e.g. `(UAF0_PERI | SIOF0_PERI | TM0_PERI)`).
- Want to defer UART bring-up? Remove the `uart_print_init()` call here and call it yourself later.

### 5.2 Logging — `src/uart_print.c`

Hooks UARTF0 to **115200 8N1 on P33 (TX) / P32 (RX)**. Provides:

| Macro                              | Output prefix |
|------------------------------------|---------------|
| `UART_PRINT_ERROR(fmt, ...)`       | `[ERROR]`     |
| `UART_PRINT_WARN(fmt, ...)`        | `[WARN]`      |
| `UART_PRINT_INFO(fmt, ...)`        | `[INFO]`      |
| `UART_PRINT_DEBUG(fmt, ...)`       | `[DEBUG]`     |
| `UART_PRINT_VERBOSE(fmt, ...)`     | `[VERB]`      |
| `UART_PRINT_HEX_DUMP(data, len, prefix)` | hex dump (≥ INFO) |
| `UART_PRINT_ASSERT(cond)`          | logs and halts on false |

Plus raw helpers: `uart_print_puts`, `uart_print_putc`, `uart_print_printf`,
`uart_print_flush`, `uart_print_is_ready`, and `uart_print_init` /
`uart_print_deinit`.

`uart_print_init()` emits nothing itself, so the only bytes on the wire are the
ones you ask for. It reports success as `true` and returns `false` only when
UARTF0 does not retain the configuration it was given — in practice, when its
peripheral clock is not running. `uart_print_is_ready()` is a different
question: it answers "is the transmitter idle right now", not "did init
succeed", so do not use it to decide whether to print.

**To use:** just call the macros — `device_initialize()` already brought UART up.

```c
UART_PRINT_INFO("boot complete, sysclk=%lu Hz", device_get_sysclk());
```

**To modify:**

- **Change the compiled-in log level:** pass `-DUART_PRINT_LEVEL=UART_PRINT_LEVEL_INFO` to the compiler, or edit the default in `src/uart_print.h` (`UART_PRINT_LEVEL_NONE` through `UART_PRINT_LEVEL_VERBOSE`). Lower-than-threshold macros compile to `((void)0)` — no code, no flash, no UART traffic.
- **Change baud or pins:** edit `UART_PRINT_BAUD_RATE` / `UART_PRINT_TX_PIN` / `UART_PRINT_RX_PIN` in `src/uart_print.h`, plus `UARTF_PARAM_DLR` (baudrate divisor) and the GPIO config in `s_configure_gpio()` inside `src/uart_print.c`.

### 5.3 Interrupt stubs — `src/app_irq.c`

Already wired:

- `UAF0_IRQHandler` → `uart_procUartfInt()` — keeps the UART interrupt path alive.
- `EXI_IRQHandler`, `NMI_Handler`, `SIOF0_IRQHandler` — empty bodies you fill in.

**To modify:** put your handler body in the matching function. Add new handlers by declaring them with their `*_IRQHandler` name; the vector table in `ml63q25x7/Source/startup_ML63Q25x7.c` will pick them up via weak aliases.

### 5.4 Drivers — `driver/inc/`

The driver library exposes one header per peripheral. The commonly used ones:

| Header           | Peripheral                  |
|------------------|------------------------------|
| `wdt.h`          | watchdog                     |
| `uartf0.h`       | UART0                        |
| `ssiof0.h`       | SPI (SSIOF0)                 |
| `irq.h`          | interrupt controller         |
| `timer0_1.h`, `timer2_3.h`, `timer4_5.h` | timers       |
| `dmac0.h`, `dmac1.h` | DMA                      |
| `saAdc0.h`, `saAdc1.h` | ADC                    |
| `i2cf0.h`        | I²C                          |
| `can.h`          | CAN                          |
| `rtc0.h`         | RTC                          |
| `cmp0.h`, `cmp1.h`, `cmp2.h` | comparators        |

`driver/CMakeLists.txt` controls which `.c` files are compiled into `libdriver.a`. If you start using e.g. timers, add `src/timer0_1.c` to `DRIVER_SOURCES`.

### 5.5 Solist-AI accelerator — `driver/lib/`, `driver/inc/solistAi.h`

This is an AI project template, so the accelerator path is wired up before you
write a line of application code. `driver/CMakeLists.txt` links one of ROHM's
prebuilt archives into `libdriver.a`, and `driver/inc/solistAi.h` declares the
FFT, on-device-learning (ODL) and OSUAD entry points. Include the header and
call them — no extra build steps.

Pick the variant with the `SOLIST_AI_LIB_512` option:

| Option | Archive | Models | Inputs | Hidden units |
|---|---|---|---|---|
| `ON` (default) | `SolistAi_Library_1_512_64.a` | 1 | 512 | 64 |
| `OFF` | `SolistAi_Library_2_256_64.a` | 2 | 256 | 64 |

```bash
cmake -S . -B build -DSOLIST_AI_LIB_512=OFF     # 2 models x 256 inputs
```

The choice also sets `ODL_MAX_INST_NUM` / `ODL_MAX_INPUTS` / `ODL_MAX_UNITS`
for every target that links `driver`, so the header's array sizes match the
archive.

**One build flag matters here.** The archives are built with 32-bit enums,
while GCC defaults to `-fshort-enums` on the ARM EABI. `CPU_FLAGS` in the
top-level `CMakeLists.txt` therefore carries `-fno-short-enums`; without it
`ld` warns that "use of enum values across objects may fail" as soon as
anything calls `fft_Init(uint16_t size, FftWindow window)`. Keep the flag if
you rework the build.

These archives are **copyright ROHM Co., Ltd.** and carry no header of their
own — read [`driver/lib/README.md`](driver/lib/README.md) before shipping
or passing on anything built from them.

### 5.6 Board utilities — `utility/board/`

- `smpl_common.[ch]` — peripheral enable/disable helpers (`smpl_enablePeripheral(...)` and the `*_PERI` flags).
- `smpl_common_led.[ch]` — simple LED on/off helpers.

### 5.7 Tests — `tests/`

A tiny on-target framework. `tests/test_framework.[ch]` defines `TEST_ASSERT*` macros and the registry. Each suite (e.g. `test_device.c`) defines tests with `DEFINE_TEST(...)` and a `register_<name>_tests()` function that `test_main.c` calls during startup. Results are printed over UART.

**Run tests:** build with `-DBUILD_TESTS=ON`, flash `build/tests/solist_ai_template_test.hex`, watch the UART for pass/fail output.

**Add a suite:**

1. Create `tests/test_<area>.c` with `DEFINE_TEST(...)` blocks and a `register_<area>_tests()` function.
2. Add the file to `TEST_SOURCES` in `tests/CMakeLists.txt`.
3. Declare and call `register_<area>_tests()` in `tests/test_main.c`.

## 6. Common modification recipes

### Add a new application source file

1. Drop `foo.c` (and `foo.h` if you want one) into `src/`.
2. Append `${CMAKE_CURRENT_SOURCE_DIR}/foo.c` to `APP_SOURCES` in `src/CMakeLists.txt` (note the `PARENT_SCOPE`).
3. Rebuild.

### Use a peripheral that isn't compiled yet

Driver `.c` files live in `driver/src/` but only the ones listed in `driver/CMakeLists.txt`'s `DRIVER_SOURCES` get built. Add the file there, rebuild.

### Adjust the watchdog timeout

In `src/device.c`, change `wdt_init(WDT_2S)` to `wdt_init(WDT_8S)` (or another value from `driver/inc/wdt.h`). Remember: any code path longer than your timeout must call `wdt_clear()` periodically, or the chip resets.

### Move the application off main()'s WDT loop

The default loop just clears the watchdog. Once you add real work, keep calling `wdt_clear()` at least once per timeout interval — including inside long inner loops, ISRs that disable interrupts for a while, or busy-waits.

### Strip UART logging for a release build

Build with `-DCMAKE_C_FLAGS="-DUART_PRINT_LEVEL=UART_PRINT_LEVEL_NONE"`, or set the macro in `src/uart_print.h`. All log macros become no-ops; the `uart_print_*` functions still compile but won't be called.

## 7. Where to look next

- `README.md` — the very short version of this guide.
- `openocd/openocd.cfg` — every TCL flash command, with comments.
- `src/device.c` — exact clock setup sequence if you need to retune.
- `THIRD_PARTY_LICENSES.md` — which files are p-ban.com Corp.'s under
  Apache-2.0 and which are ROHM's under ROHM's own terms. Read this before
  copying any part of the template into another project.
