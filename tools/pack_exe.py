#!/usr/bin/env python3
"""Append ruffle.exe + game.swf to the launcher stub and write a trailer so the stub can find them."""
import struct, sys

stub_path, ruffle_path, swf_path, out_path = sys.argv[1:5]
stub = open(stub_path, "rb").read()
ruffle = open(ruffle_path, "rb").read()
swf = open(swf_path, "rb").read()

ruffle_off = len(stub)
swf_off = ruffle_off + len(ruffle)
trailer = b"BFDPAYLD" + struct.pack("<QQQQ", ruffle_off, len(ruffle), swf_off, len(swf))
with open(out_path, "wb") as f:
    f.write(stub + ruffle + swf + trailer)
print(f"wrote {out_path} ({len(stub) + len(ruffle) + len(swf) + len(trailer):,} bytes)")
