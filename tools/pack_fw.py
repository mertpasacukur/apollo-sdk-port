#!/usr/bin/env python3
"""
pack_fw.py — Apollo FW flash image packer for Versal.

Takes raw .bin firmware files and prepends a 4-byte LE size header.
Output files can be programmed to QSPI flash at their respective offsets.

Usage:
    python3 pack_fw.py <input_dir> <output_dir>

Example:
    python3 pack_fw.py ../pkg/src/examples/fw_images/b0/app_signed_encrypted_B ./packed_fw

Then program each packed file to QSPI at the offset in its filename:
    flash_image_0x01030000.bin → offset 0x01030000
    flash_image_0x02000000.bin → offset 0x02000000
    flash_image_0x20000000.bin → offset 0x20000000
    flash_image_0x21000000.bin → offset 0x21000000
"""

import os
import sys
import struct
import glob


def pack_fw_file(input_path, output_path):
    """Prepend 4-byte LE size header to firmware binary."""
    with open(input_path, 'rb') as f:
        data = f.read()

    fw_size = len(data)
    header = struct.pack('<I', fw_size)  # 4 bytes, little-endian uint32

    with open(output_path, 'wb') as f:
        f.write(header)
        f.write(data)

    return fw_size


def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <input_dir> <output_dir>")
        sys.exit(1)

    input_dir = sys.argv[1]
    output_dir = sys.argv[2]
    os.makedirs(output_dir, exist_ok=True)

    files = sorted(glob.glob(os.path.join(input_dir, 'flash_image_*.bin')))
    if not files:
        print(f"ERROR: No flash_image_*.bin files found in {input_dir}")
        sys.exit(1)

    print(f"Packing {len(files)} firmware files...")
    print(f"  Input:  {input_dir}")
    print(f"  Output: {output_dir}")
    print()

    for fpath in files:
        fname = os.path.basename(fpath)
        out_path = os.path.join(output_dir, fname)
        fw_size = pack_fw_file(fpath, out_path)

        # Extract offset from filename
        offset_str = fname.replace('flash_image_', '').replace('.bin', '')

        print(f"  {fname}")
        print(f"    Original: {fw_size:,} bytes")
        print(f"    Packed:   {fw_size + 4:,} bytes (4-byte size header + data)")
        print(f"    Flash:    {offset_str}")
        print()

    print("Done! Program each packed file to QSPI at its respective offset.")
    print("In Vitis: Xilinx → Program Flash → select file → set offset")


if __name__ == '__main__':
    main()
