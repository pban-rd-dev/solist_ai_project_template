# Copyright (c) 2026 p-ban.com Corp.
# SPDX-License-Identifier: LicenseRef-p-ban-solist-ai-gcc-template
# See LICENSE (LICENSE.ja is the authoritative Japanese text).

# Dump a flash range to a file via DAP-direct memory reads.
# Uses read_mem32 from openocd/openocd.cfg, which goes through the DAP AP
# registers directly — it does NOT require the CPU to be halted.
#
# Usage (from telnet localhost 4444, with openocd/openocd.cfg loaded):
#   source openocd/tools/dump_via_dap.tcl
#   dump_via_dap 0x10000000 4096 firmware_readback.bin    ;# start of flash
#   dump_via_dap 0x1003FFC0   64 codeoption_readback.bin  ;# code option block
#
# Then on the host, compare against what you flashed:
#   cmp -n 4096 build/solist_ai_template.bin firmware_readback.bin

proc dump_via_dap {addr size filename} {
    set words [expr {($size + 3) / 4}]
    set f [open $filename wb]

    set cur $addr
    for {set i 0} {$i < $words} {incr i} {
        set v [read_mem32 $cur]
        # binary format i = little-endian 32-bit; matches Cortex-M0+ memory order
        puts -nonewline $f [binary format i $v]
        incr cur 4

        if {[expr {($i + 1) % 256}] == 0} {
            echo "  [expr {$i + 1}] / $words words"
        }
    }
    close $f
    echo "wrote $filename ([expr {$words * 4}] bytes from 0x[format %08X $addr])"
}
