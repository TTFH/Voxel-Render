import sys
import gzip
import argparse

from nbt import nbt

if len(sys.argv) < 2:
	input(f"Usage: {sys.argv[0]} <schematic>")
	quit()

parser = argparse.ArgumentParser(description="Converts a Minecraft schematic file into a simpler format")
parser.add_argument("filename", help="The .schem file to convert")
args = parser.parse_args()

decompressed = gzip.open(args.filename)
schematic = nbt.NBTFile(buffer=decompressed)

paletteNBT = schematic["Palette"]
block_data = schematic["BlockData"]
width = schematic["Width"].value
length = schematic["Length"].value

palette_file = open(f"mc_palette.txt", "w")
indices_file = open(f"mc_indices.bin", "wb")

for item in paletteNBT:
	index = paletteNBT[item].value
	block_name = item.split("[")[0]
	block_name = block_name.removeprefix("minecraft:")

	variant = ""
	if "[" in item:
		variant = item.split("[")[1].split("]")[0]
	palette_file.write(f"{index} {block_name} {variant}\n")

	# TODO: orientation (facing)
	props = []
	if variant != "":
		for prop in variant.split(","):
			prop_name = prop.split("=")[0]
			prop_val = prop.split("=")[1]
			props.append((prop_name, prop_val))
			# TODO: remove false, implicit true

for i in range(len(block_data)):
	x = i % width
	z = (i // width) % length
	y = i // (width * length)
	if x > 255 or y > 255 or z > 255:
		print(f"Warning: Coordinates {x} {y} {z} are out of bounds")
		continue
	indices_file.write(bytes([x, y, z, block_data[i]]))

decompressed.close()
