#!/usr/bin/env python3
"""Fills in the GBA header checksum and pads the ROM to a multiple of 4 bytes.
NOTE: it cannot add the Nintendo logo. Emulators (mGBA, EmulatorJS, ...) don't
need it; for real hardware / flash carts run `gbafix tinyfarm.gba` (devkitARM)."""
import sys
p = sys.argv[1]
d = bytearray(open(p, 'rb').read())
while len(d) % 4: d.append(0)
if len(d) < 0xC0: sys.exit("ROM too small")
d[0xB2] = 0x96
d[0xBD] = (-(sum(d[0xA0:0xBD]) + 0x19)) & 0xFF
open(p, 'wb').write(d)
print("header checksum written, %d bytes" % len(d))
