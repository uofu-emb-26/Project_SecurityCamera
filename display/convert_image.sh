#!/bin/bash
# Usage: ./convert_image.sh <input_image>
# Example: ./convert_image.sh photo.jpg

INPUT=$1

if [ -z "$INPUT" ]; then
    echo "Usage: ./convert_image.sh <input_image>"
    exit 1
fi

echo "[1/3] Resizing to 320x240..."
convert "$INPUT" -resize 320x240! -quality 60 /tmp/display_image.jpg

echo "[2/3] Generating test_image.h..."
xxd -i /tmp/display_image.jpg > Inc/test_image.h
sed -i 's/unsigned char _tmp_display_image_jpg/const uint8_t TEST_JPEG/' Inc/test_image.h
sed -i 's/unsigned int _tmp_display_image_jpg_len/const uint32_t TEST_JPEG_SIZE/' Inc/test_image.h

echo "[3/3] Building and flashing..."
cd ..
cmake --build build --target flash_display

echo "[OK] Done!"
