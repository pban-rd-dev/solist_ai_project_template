# `driver/lib/` — Solist-AI accelerator library (ROHM, binary)

## What these files are

| File | Models | Inputs | Hidden units |
|---|---|---|---|
| `SolistAi_Library_1_512_64.a` | 1 | 512 | 64 |
| `SolistAi_Library_2_256_64.a` | 2 | 256 | 64 |

Static archives containing `mlacc.o`, `mlacc_fft.o`, `mlacc_odl.o` and
`mlacc_osuad.o`. They implement the FFT, on-device-learning (ODL) and OSUAD
API declared in [`../inc/solistAi.h`](../inc/solistAi.h) (ROHM Solist-AI
driver Ver 1.3.0). `driver/CMakeLists.txt` picks one of the two with the
`SOLIST_AI_LIB_512` option and links it into the `driver` target, so the AI
path is ready to use without any further setup.

Built for Cortex-M0+ (Armv6-M, Thumb-1) with clang 1.30.0 (LLVM 18.1.8), with
32-bit enums. That is why the top-level `CMakeLists.txt` compiles the whole
project with `-fno-short-enums`: GCC defaults to variable-size enums for the
ARM EABI, and mixing the two makes `ld` warn that "use of enum values across
objects may fail" the moment anything calls an API taking an enum, such as
`fft_Init(uint16_t size, FftWindow window)`.

## License

**Copyright ROHM Co., Ltd. All rights reserved.**

These archives are ROHM's work, not p-ban.com Corp.'s. `LICENSE` grants no
rights in them and p-ban.com Corp. cannot sublicense them. Their terms are
ROHM's own, the same notice carried by the matching header
[`../inc/solistAi.h`](../inc/solistAi.h):

> This software is provided "as is" and any expressed or implied warranties,
> including, but not limited to, the implied warranties of merchantability
> and fitness for a particular purpose are disclaimed. ROHM shall not be
> liable for any direct, indirect, consequential or incidental damages
> arising from using or modifying this software. You (customer) can modify
> and use this software in whole or part on your own responsibility, only for
> the purpose of developing the software for use with microcontroller
> manufactured by ROHM.

What this means:

- Use them **only** to develop software that runs on a ROHM microcontroller.
  This grant comes from ROHM directly; `LICENSE` does not cover it.
- The notice permits use and modification. It grants **no right of
  redistribution**. To pass these archives on to a third party, obtain them
  and their license from ROHM and follow ROHM's terms directly.
- An `ar` archive cannot carry a copyright header of its own. This file is
  that notice — keep it next to the archives.
- p-ban.com Corp.'s own right to publish these archives comes from its
  partner agreement with ROHM, not from the notice above, and does not extend
  to you. See §0 of
  [`../../THIRD_PARTY_LICENSES.md`](../../THIRD_PARTY_LICENSES.md).

See [`../../THIRD_PARTY_LICENSES.md`](../../THIRD_PARTY_LICENSES.md) §2 for
the rest of the ROHM-owned files in this repository.
