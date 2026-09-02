#!/usr/bin/env bash
# Copyright (c) 2026 p-ban.com Corp.
# SPDX-License-Identifier: LicenseRef-p-ban-solist-ai-gcc-template
# See LICENSE (LICENSE.ja is the authoritative Japanese text).
#
# jlink_flash.sh -- program the ML63Q2537 through SEGGER J-Link.
#
# The OpenOCD path (scripts/hex_to_flash.py + openocd/openocd.cfg)
# drives the flash controller from the host: every 32-bit word costs an
# FLASHACP / FLASHA / FLASHD / FLASHSTA round trip over SWD.
#
# J-Link instead downloads the vendor CMSIS flash algorithm
# (jlink/ML63Q25x7.FLM, from ROHM.ML63Q25x7_DFP 0.4.0) into target RAM and runs
# it there. The erase/program loops execute on the Cortex-M0+ at 48 MHz; SWD
# only carries the image data. jlink/JLinkDevices.xml wires the algorithm to the
# ML63Q2537 flash bank at 0x10000000.
#
# Usage:
#   scripts/jlink_flash.sh [<build_dir>]        # program the .hex in build_dir
#   scripts/jlink_flash.sh --file <img> [<addr>]
#                                               # program one .bin/.hex/.elf
#                                               # (<addr> required for .bin)
#
# Environment:
#   JLINK_EXE    JLinkExe binary          (default: JLinkExe from PATH, else
#                                          /opt/SEGGER/JLink/JLinkExe)
#   JLINK_SPEED  SWD clock in kHz         (default: 4000)
#   JLINK_SN     probe serial number      (default: first probe found)
#   JLINK_VTREF  force VTref in mV, e.g. 3300, when the probe's VTref pin is
#                not wired to the target  (default: auto-detect)
#   JLINK_NO_RUN 1 = leave the core halted instead of reset-and-run

set -euo pipefail

REPO_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)

JLINK_EXE=${JLINK_EXE:-}
if [ -z "$JLINK_EXE" ]; then
    if command -v JLinkExe >/dev/null 2>&1; then
        JLINK_EXE=JLinkExe
    elif [ -x /opt/SEGGER/JLink/JLinkExe ]; then
        JLINK_EXE=/opt/SEGGER/JLink/JLinkExe
    else
        JLINK_EXE=JLinkExe
    fi
fi
JLINK_SPEED=${JLINK_SPEED:-4000}
DEVICE=ML63Q2537

usage() {
    cat <<'USAGE'
Usage:
  scripts/jlink_flash.sh [<build_dir>]         program the .hex in build_dir
                                               (default build_dir: build)
  scripts/jlink_flash.sh --file <img> [<addr>] program one .bin/.hex/.elf
                                               (<addr> required for .bin)

Environment:
  JLINK_EXE    JLinkExe binary          (default: JLinkExe from PATH, else
                                         /opt/SEGGER/JLink/JLinkExe)
  JLINK_SPEED  SWD clock in kHz         (default: 4000)
  JLINK_SN     probe serial number      (default: first probe found)
  JLINK_VTREF  force VTref in mV, e.g. 3300, when the probe's VTref pin is
               not wired to the target  (default: auto-detect)
  JLINK_NO_RUN 1 = leave the core halted instead of reset-and-run
USAGE
    exit "${1:-1}"
}

mode=hex
build_dir=
image=
image_addr=

case "${1:-}" in
    -h|--help)  usage 0 ;;
    --file)
        mode=file
        image=${2:-}
        image_addr=${3:-}
        [ -n "$image" ] || { echo "error: --file needs an image path" >&2; exit 1; }
        ;;
    -*)         echo "error: unknown option $1" >&2; usage ;;
    *)          build_dir=${1:-build} ;;
esac

if ! command -v "$JLINK_EXE" >/dev/null 2>&1; then
    echo "error: '$JLINK_EXE' not found." >&2
    echo "       Install the SEGGER J-Link software or set JLINK_EXE=/path/to/JLinkExe." >&2
    exit 1
fi

for required in "$REPO_ROOT/jlink/JLinkDevices.xml" "$REPO_ROOT/jlink/ML63Q25x7.FLM"; do
    [ -f "$required" ] || { echo "error: missing $required" >&2; exit 1; }
done

if [ "$mode" = hex ]; then
    # Glob rather than hardcode the project name: this is a template, and the
    # name changes as soon as someone renames the CMake project.
    shopt -s nullglob
    hexes=("$build_dir"/*.hex)
    shopt -u nullglob
    case ${#hexes[@]} in
        0) echo "error: no .hex found in $build_dir (build the project first)" >&2; exit 1 ;;
        1) image=${hexes[0]} ;;
        *) echo "error: multiple .hex files in $build_dir; pass one with --file" >&2
           printf '  %s\n' "${hexes[@]}" >&2; exit 1 ;;
    esac
else
    [ -f "$image" ] || { echo "error: missing $image" >&2; exit 1; }
fi

cmdfile=$(mktemp)
trap 'rm -f "$cmdfile"' EXIT

{
    # The DLL concatenates this path with "JLinkDevices.xml" verbatim, so the
    # trailing slash is required.
    echo "exec JLinkDevicesXMLPath = $REPO_ROOT/jlink/"
    if [ -n "${JLINK_SN:-}" ]; then
        echo "usb ${JLINK_SN}"
    fi
    echo "si SWD"
    echo "speed $JLINK_SPEED"
    if [ -n "${JLINK_VTREF:-}" ]; then
        echo "vtref ${JLINK_VTREF}"
    fi
    echo "device $DEVICE"
    echo "connect"
    echo "r"
    echo "halt"

    case "$image" in
        *.bin)
            [ -n "$image_addr" ] || { echo "error: a .bin image needs an address" >&2; exit 1; }
            echo "loadfile $image, $image_addr"
            echo "verifybin $image, $image_addr"
            ;;
        *)
            # .hex / .elf carry their own addresses. loadfile erases and
            # verifies the sectors it writes.
            echo "loadfile $image"
            ;;
    esac

    echo "r"
    if [ "${JLINK_NO_RUN:-0}" = 1 ]; then
        echo "halt"
    else
        echo "g"
    fi
    echo "exit"
} > "$cmdfile"

echo "--- J-Link command script ---"
cat "$cmdfile"
echo "-----------------------------"

"$JLINK_EXE" -NoGui 1 -ExitOnError 1 -CommanderScript "$cmdfile" < /dev/null
