#!/bin/bash
set -euo pipefail

# Check for required tools
for tool in tdecrypt cmft convert; do
	if ! command -v "$tool" &> /dev/null; then
		echo "Error: '$tool' not found in PATH or current directory."
		exit 1
	fi
done

# Define face mapping
declare -A map=(
	["posz"]="front"
	["negz"]="back"
	["posy"]="top"
	["negy"]="bottom"
	["posx"]="right"
	["negx"]="left"
)

# Check for .tde files
shopt -s nullglob
tde_files=(*.tde)
shopt -u nullglob

if [ ${#tde_files[@]} -eq 0 ]; then
	echo "No .tde files found. Skipping decryption."
else
	echo "Decrypting .tde files..."
	for file in "${tde_files[@]}"; do
		echo "  Decrypting '$file'"
		./tdecrypt "$file"
	done
fi

# Check for .dds files
shopt -s nullglob
dds_files=(*.dds)
shopt -u nullglob

if [ ${#dds_files[@]} -eq 0 ]; then
	echo "No .dds files found. Nothing to process."
	exit 0
fi

# Process .dds files
for file in "${dds_files[@]}"; do
	echo "Processing $file"
	filename="${file%.dds}"
	mkdir -p "$filename"

	./cmft --input "$file" --output0 "$filename" --output0params tga,bgra8,facelist \
		--outputGammaDenominator 2.2 --silent \
		--negXflipH --posXflipH --negYflipH --posYflipH --negZflipH --posZflipH

	for direction in "${!map[@]}"; do
		orientation="${map[$direction]}"
		pattern="${filename}_${direction}_*_256x256.tga"
		matches=($pattern)

		if [ ${#matches[@]} -gt 0 ]; then
			skybox_side="${matches[0]}"
			echo "  Converting $skybox_side → $orientation.png"
			convert "$skybox_side" "$filename/$orientation.png"
		else
			echo "  [ERROR] No matching TGA file for direction '$direction' in $filename"
		fi
	done
done

# Cleanup
rm -f *.tga
echo "Done."
