#!/bin/bash

FRAMES_DIR="output/frames"
OUTPUT_GIF="animation.gif"
FPS=24

ffmpeg -framerate "$FPS" -i "${FRAMES_DIR}/frame-%d.png" -vf "split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" "$OUTPUT_GIF"
