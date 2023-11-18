#!/bin/bash

declare -A map
map["posz"]="front"
map["negz"]="back"
map["posy"]="top"
map["negy"]="bottom"
map["posx"]="right"
map["negx"]="left"

for file in *.tde; do
	./tdecrypt "$file"
done

if [ ! -e "cmft" ]; then
	echo "Error: cmft not found."
	exit 1
fi

for file in *.dds; do
	echo "Extracting $file"
	filename="${file%.dds}"
	mkdir "$filename"

	./cmft --input "$file" --output0 "$filename" --output0params tga,bgra8,facelist \
	--outputGammaDenominator 2.2 --silent \
	--negXflipH --posXflipH --negYflipH --posYflipH --negZflipH --posZflipH

	for direction in "${!map[@]}"; do
		orientation="${map[$direction]}"
		skybox_side="${filename}_${direction}_0_256x256.tga"

		if [ -e "$skybox_side" ]; then
			echo "Converting $filename - $direction"
			convert "$skybox_side" "$filename"/"${map[$direction]}".png
		else
			echo "[ERROR] $skybox_side not found."
		fi
	done
done

rm *.tga
