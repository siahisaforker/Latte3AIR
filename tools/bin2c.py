import sys

if len(sys.argv) != 3:
    print("Usage: python bin2c.py input.bin output.h")
    sys.exit(1)

input_file = sys.argv[1]
output_file = sys.argv[2]
array_name = "game_bin_data"

with open(input_file, "rb") as f:
    data = f.read()

with open(output_file, "w") as f:
    f.write(f"#pragma once\n")
    f.write(f"const unsigned char {array_name}[] = {{\n")
    for i, byte in enumerate(data):
        if i % 12 == 0:
            f.write("    ")
        f.write(f"0x{byte:02X}, ")
        if (i+1) % 12 == 0:
            f.write("\n")
    f.write("\n};\n")
    f.write(f"const unsigned int {array_name}_size = {len(data)};\n")
