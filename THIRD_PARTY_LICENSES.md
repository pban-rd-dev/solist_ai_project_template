# License breakdown

**The short version:** you may use and modify everything here to develop
firmware for a ROHM/LAPIS microcontroller, and ship that firmware in binary
form inside your own products. You may not redistribute the source to third
parties. That rule is the same for every file, whoever owns it — see
`LICENSE` (`LICENSE.ja` is authoritative).

This document exists because ownership is *not* uniform, and that matters if
you ever want to do something outside the rule above.

| Scope | Copyright | Terms |
|---|---|---|
| p-ban.com Corp. authored files | p-ban.com Corp. (株式会社ピーバンドットコム) | `LICENSE` |
| ROHM reference software (drivers, board utilities, device header, code option, UARTF interrupt dispatch) and the ROHM binaries (Solist-AI library, flash algorithm) | ROHM Co., Ltd. | ROHM's own notice — see §2. p-ban.com Corp. cannot sublicense these |
| Startup / system / linker script | ROHM Co., Ltd., portions by Arm Limited | Apache-2.0 — **not** restricted by `LICENSE` |
| `external/CMSIS` (git submodule) | Arm Limited | Apache-2.0 — **not** restricted by `LICENSE` |

## 1. p-ban.com Corp.

```
Copyright (c) 2026 p-ban.com Corp. (株式会社ピーバンドットコム)
SPDX-License-Identifier: LicenseRef-p-ban-solist-ai-gcc-template
```

These files are licensed to you under `LICENSE`. 25 files carry that SPDX
header:

- `CMakeLists.txt`, `src/CMakeLists.txt`, `driver/CMakeLists.txt`,
  `utility/CMakeLists.txt`, `tests/CMakeLists.txt`
- `cmake/arm-none-eabi-toolchain.cmake`
- `.github/workflows/build.yml`
- `.gitattributes`
- `src/main.c`, `src/device.[ch]`, `src/uart_print.[ch]`, `src/app_irq.c`,
  `src/syscalls.c`
- `tests/test_main.c`, `tests/test_device.c`, `tests/test_uart_print.c`,
  `tests/test_framework.[ch]`
- `scripts/hex_to_flash.py`, `scripts/jlink_flash.sh`,
  `scripts/jlink_flash.ps1`
- `openocd/openocd.cfg`, `openocd/tools/dump_via_dap.tcl`

The documentation and repository metadata — `README.md`,
`GETTING_STARTED.md`, `NOTICE`, this file, `LICENSE`, `LICENSE.ja`,
`.gitignore`, `.gitmodules` — are likewise p-ban.com Corp.'s under the same
terms, but do not carry per-file SPDX headers.

The full license text is in `LICENSE` (English) and `LICENSE.ja` (Japanese,
authoritative). `LICENSE.apache-2.0` is the Apache-2.0 text, which applies
only to the components listed in §3 and §4.

## 2. ROHM Co., Ltd. — reference software (restricted)

89 source files are derived from the **ML63Q2500 Reference Software**
distributed by ROHM:

| Path | Files |
|---|---|
| `driver/inc/` | 48 |
| `driver/src/` | 31 |
| `utility/board/` | 4 |
| `ml63q25x7/include/ML63Q25x7.h` | 1 |
| `src/codeoption.c`, `src/codeoption.h`, `src/codeoption_config.h` | 3 |
| `src/uartf_int.c`, `src/uartf_int.h` | 2 |

Three further files are ROHM's but hold no source at all, so they cannot
carry a header:

| Path | What it is | Its notice |
|---|---|---|
| `driver/lib/SolistAi_Library_1_512_64.a`, `driver/lib/SolistAi_Library_2_256_64.a` | Solist-AI accelerator library (FFT / on-device learning / OSUAD), implementing the API in `driver/inc/solistAi.h` | `driver/lib/README.md` |
| `jlink/ML63Q25x7.FLM` | CMSIS flash algorithm, taken verbatim from ROHM.ML63Q25x7_DFP 0.4.0 (`Flash/ML63Q25x7.FLM`) | the comment block in `jlink/JLinkDevices.xml` |

Every one of the 89 source files carries ROHM's notice in its header, and the
three binaries are covered by the same terms through the notices named above.
The operative text is:

> This software is provided "as is" and any expressed or implied warranties,
> including, but not limited to, the implied warranties of merchantability
> and fitness for a particular purpose are disclaimed. ROHM shall not be
> liable for any direct, indirect, consequential or incidental damages
> arising from using or modifying this software. You (customer) can modify
> and use this software in whole or part on your own responsibility, only
> for the purpose of developing the software for use with microcontroller
> manufactured by ROHM.

What this means for you:

- These files may be used **only** for developing software that runs on a
  ROHM microcontroller. This grant comes from ROHM directly, through the
  notice in each file — p-ban.com Corp. cannot sublicense them to you, and
  `LICENSE` grants no rights in them.
- The ROHM notice permits use and modification. It grants **no right of
  redistribution**.
- Do **not** remove or alter the ROHM copyright notices.
- ROHM's separate sample-software license agreement also applies to the
  original package. If you intend to redistribute any of these files, or
  use them outside the scope above, obtain the official
  `ML63Q2500_ReferenceSoftware` package and its license from ROHM and
  follow ROHM's terms directly.
- Neither ROHM nor p-ban.com Corp. warrants these files. See §5 below.

### What was modified

`driver/inc/`, `driver/src/`, and `utility/board/` are the unmodified
originals from `ML63Q2500_ReferenceSoftware_v120`, with line endings
converted from CRLF to LF. Nothing else in those 83 files was changed.
`.gitattributes` (`* text=auto eol=lf`) keeps that conversion in place
regardless of a contributor's `core.autocrlf` setting.

The three binaries are byte-for-byte as received from ROHM.

The remaining 6 files were modified by p-ban.com Corp., and each records its
modifications directly below the ROHM notice:

- `src/codeoption.c`, `src/codeoption.h`, `src/codeoption_config.h` —
  `codeoption.h` includes `<stdint.h>` rather than `"mcu.h"`; comments and
  whitespace differ. The code option bit layout and the configured values
  are unchanged.
- `src/uartf_int.c`, `src/uartf_int.h` — `smpl_procUartfInt()` from
  `SourceCode/samples/UARTF/UartFCommunicate/src/main.c` renamed to
  `uart_procUartfInt()` and moved into its own translation unit. The
  function body is otherwise unchanged.

`ml63q25x7/include/ML63Q25x7.h` carries ROHM's disclaimer text but no
explicit `Copyright` line; it is ROHM's work regardless, and is covered by
the terms above.

## 3. ROHM Co., Ltd. / Arm Limited — Apache-2.0

These 4 files are Apache-2.0 with an explicit SPDX identifier, and are based
on Arm's CMSIS-Core device examples:

- `ml63q25x7/Source/startup_ML63Q25x7.c`
- `ml63q25x7/Source/system_ML63Q25x7.c`
- `ml63q25x7/Source/GCC/ML63Q25x7_gcc.ld`
- `ml63q25x7/include/system_ML63Q25x7.h`

```
Copyright (C) 2024 ROHM Co., Ltd. All rights reserved.
SPDX-License-Identifier: Apache-2.0
```

Each also names the Arm file it derives from. The Arm copyright line is
`Copyright (c) 2009-2020 Arm Limited` in `startup_ML63Q25x7.c` and
`ML63Q25x7_gcc.ld`, and `Copyright (c) 2009-2019 Arm Limited` in the other
two.

## 4. Arm Limited — CMSIS (git submodule)

`external/CMSIS` is a git submodule pointing at
<https://github.com/ARM-software/CMSIS_6.git>. Its contents are not stored
in this repository. Copyright Arm Limited, Apache-2.0; see the `LICENSE`
file inside the submodule. Some files within CMSIS carry additional
third-party notices (for example, portions copyright Keith Packard under a
BSD-style license) — refer to the submodule itself.

## 5. No warranty

The Apache-2.0 files are provided on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, as stated in `LICENSE`.

The ROHM files are provided "as is" by ROHM, as quoted in §2. p-ban.com
Corp. likewise provides this repository as a whole without warranty of any
kind and accepts no liability arising from its use.
